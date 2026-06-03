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

    /*if (_fseeki64(file, size_binario - 1, SEEK_SET) != 0) { //fseek64(puntero al archivo, cantidad a desplazar,punto de referencia para dezplazarse)
        perror("Error al posicionar el cursor");                                                            //SEEK_SET -> desde el inicio del archivo
        fclose(file);                                                                                       //SEEK_CUR ->desde la posicion actual del cursor
        return 1;
    }*/

    //posicionarnos para cerrar el archivo
    //fwrite(&file, sizeof(char), 1, file);//NOTA: 

    memset(numeros, 0, size_ArchivoBinario);
    fwrite(numeros , 1 , size_ArchivoBinario,  ArchivoBinario);
    fclose(ArchivoBinario);
    free(numeros);
    return 0;
}

int reescritura(const char *NombrePro, const char *ArchivoBinario){
    FILE *archivoP = fopen(NombrePro,"r");
    if (archivoP == NULL) {
        //perror("Error al abrir el archivo");
        return 1;
    }
    FILE *archivoB = fopen(ArchivoBinario,"wb");
    if (archivoB == NULL) {
       // perror("mError al abrir el archivo");
        return 1;
    }

    char buffer[1024];
    size_t caracteresLeidos;
    while ((caracteresLeidos = fread(buffer,1,1024,archivoP)) > 0){
        fwrite(buffer,1,caracteresLeidos,archivoB);
    }
    fclose(archivoB);
    fclose(archivoP);
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
void in_TMS(int TMS[][2]){


    for(int i=0; i < 32768; i++){
        TMS[i][0] = 0;
        TMS[i][1] = 0;
    }

}

int Busqueda_TMS(int TMS[][2]){

    for(int i=0; i < 32768; i++){
        if(TMS[i][0] == 0){
            return i;//marco libre jeje
        }
    }
    return 1;
}


void Paginacion(FILE *archivoProceso,FILE *swap,int PID,int TMS[][2],int max_marcos){
    char instruccion[100];
    char relleno[100];
    int paginaVirtual = 0;

    while(1){
        int marco = BuscarMarcoLibre(TMS, max_marcos);

        if(marco == 0){
            printf("ERROR: Swap lleno\n");
        }

        int instL = 0;

       for(int i = 0; i < 4; i++){
            if(fgets(instruccion,sizeof(instruccion),archivoProceso) == NULL){
                break;
            }

            int usados = strlen(instruccion);
            memset(relleno, '0', sizeof(relleno));
            fwrite(instruccion,sizeof(char),usados,swap);
            fwrite(relleno,sizeof(char),100 - usados,swap);
            instL++;
        }
        if(instL == 0){
            break;
        }

        /* Registrar dueño del marco */

        TMS[marco][0] = PID;
        TMS[marco][1] = paginaVirtual;

        paginaVirtual++;

        if(instL < 4){
            break;
        }
    }
}

void inicializat_TMP(int TMP_presencia[], int TMP_marco_RAM[], int TMP_marco_swap[],int num_paginas);

void main(){//-------------------------------------------------------------------Intento de simulacion para TMP
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


void inicializar_TMP(int TMP_presencia[], int TMP_marco_RAM[], int TMP_marco_swap[],int num_paginas){
    int i = 0;
    for(i;i<num_paginas;i++){
        TMP_presencia[i] = 0;
        TMP_marco_RAM[i] = -1;
        TMP_marco_swap[i] = i;
    }

    //printf("Pagina  |   Marco swap  |   Marco RAM   |   Presencia   ");
    
    //for(int j=0;j<max_paginas;j++){
        //printf("%d  %d  %d",marco_swap[j],marco_RAM[j],bit_P[i]);
    //}
}



