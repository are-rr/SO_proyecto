#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "procesos.h"


//              tamaño_IR
 
    //FILE *archivo = fopen("archivo.txt", "r");        main.c
    //char arreglo[TAMAÑORAMCALCULADO];

   /* if(archivo != NULL){
        fgets(arreglo,TAMAÑORAMCALCULADO, archivo);
        fclose(archivo);
    
    }*/

#include <stdio.h>
#include <stdlib.h>

//memoria virtual
int Crear_ArchivoBinario(const char *nombre, int size_IR) {

    int size_ArchivoBinario  = 131072;
    size_ArchivoBinario = size_ArchivoBinario  * size_IR;
    char *numeros = malloc(size_ArchivoBinario);

    FILE *ArchivoBinario = fopen(nombre, "wb");
    if (ArchivoBinario == NULL) {
        //perror("Error al abrir el archivo");
        return 1;
    }

    memset(numeros, 0, size_ArchivoBinario);
    fwrite(numeros , 1 , size_ArchivoBinario,  ArchivoBinario);
    fclose(ArchivoBinario);
    free(numeros);
    return 0;
}



int memoria_RAM(FILE *archivo, int size_IR){
    int size_RAM  = 0;
    size_RAM = size_IR * 64;
    char RAM[size_RAM]; // almacenar 64 instrucciones

    if(archivo != NULL){
        fgets(RAM,size_RAM, archivo);
        fclose(archivo);
    }

    return 0;
}


/*int main(){
    char IR[100] = "MOV EAX,EBX";
    char nombre[100] = "prueba2";

    char origen[100] = "file2";
    char destino[100] = "prueba2";

    //Crear_ArchivoBinario(nombre,strlen(IR));
    reescritura(origen,destino);
    
    return 0;
}
*/


void in_TMS(int TMS[][1]){//tiene formato TMS[][1], sino especificas el numero de columnas, te marco error
    for(int i=0; i < 32768; i++){
        TMS[i][0] = 0;//inicializamos la tabla con todos los valores en 0
        
    }

}

int Busqueda_TMS(int TMS[][1]){
    for(int i=0; i < 32768; i++){ //buscamos aquel marco libre, como es no contigua, el primero que encuentre, agarra
        if(TMS[i][0] == 0){
            return i;//marco libre jeje
        }
    }
    return -1;
}


//mvprintw(y_tabla, 0, "%-5s %-5s %5s %5s %5s","TMP", "Pagi", "Bit", "M_R", "M_S");

//paso los archivos tipo file, por que como lo vamos a utilizar en la función de reescritura, para poder escribir y leer el archivo
//necesitamos abrir los archivos FILE tal cual
void Paginacion(FILE *archivoProceso,FILE *swap,int PID,int TMS[][1],int TMP[][3]){
    char instruccion[100];//array que guarda instruccion
    char relleno[100]; //array que guarda lo que sobra


    while(1){// primero buscas un marco libre antes de poder escribirlo, pues si primero haces la lectura y no hay espacio, pss que haces xd
        int marco = Busqueda_TMS(TMS);

        if(marco == -1){
            printf("ERROR: Swap lleno\n");
        }

        int instL = 0;//la necesitamos para leer la cantidad de lineas leidas, puede que una pagina al final solamente lea 2 instrucciones
        //además es nuestra condición de termino para el while, sino la tenemos nunca termina, pues sale cuando no lee ninguna linea
        
        int pagina = 0;  

        fseek(swap, marco * 400, SEEK_SET);// se mueve al marco de página correspondiente

       for(int i = 0; i < 4; i++){
            if(fgets(instruccion,sizeof(instruccion),archivoProceso) == NULL){
                break;
            }

            int usados = strlen(instruccion);
            memset(relleno, '0', sizeof(relleno));
            fwrite(instruccion,sizeof(char),usados,swap);
            fwrite(relleno,sizeof(char),100 - usados,swap);
            
            instL++;
            //rewind(swap);
        }

        
        if(instL == 0){
            break;
        }
        
        TMS[marco][0] = PID;
        TMP[pagina][2] = marco; //Gurdar en la TMP el marco del SWAP
        pagina++;
    }
}

int reescritura(const char *NombrePro, const char *ArchivoBinario,int pid, int TMS[][1], int TMP[][3]){
    FILE *archivoP = fopen(NombrePro,"r");
    if (archivoP == NULL) {
        //perror("Error al abrir el archivo");
        return 1;
    }
    FILE *archivoB = fopen(ArchivoBinario,"r+b");//NOTA: investigar por que no funciona wb
    if (archivoB == NULL) {
       // perror("mError al abrir el archivo");
        return 1;
    }

   Paginacion(archivoP,archivoB,pid,TMS,TMP);


   //fclose(archivoB);
   return 0;
}


//void inicializar_TMP(int TMP_presencia[], int TMP_marco_RAM[], int TMP_marco_swap[],int num_paginas);

/*void main(){//-------------------------------------------------------------------Intento de simulacion para TMP
    int max_paginas = 32768; //variable global
    int tam_pag = 4;

    //Tabla TMP
    int TMP_presencia[max_paginas]; //1->RAM y 0->swap
    int TMP_marco_RAM[max_paginas];
    int TMP_marco_swap[max_paginas];

    int paginas = 3; //paginas que tiene el proceso
    //inicializar la tabla para TMP
    inicializar_TMP(TMP_presencia,TMP_marco_swap,TMP_marco_RAM,paginas);



    int bit_presencia = 0;

    TMP(1,3,5);//------------------------------------------
}

*/
void inicializar_TMP(int TMP[][3]){

    for(int i = 0; i < 32768; i++){
        TMP[i][0] = 0;   // bit presencia: 0 = swap
        TMP[i][1] = 0;  // marco RAM
        TMP[i][2] = -1;  // marco swap
    }
}



