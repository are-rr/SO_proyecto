#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "procesos.h"

//memoria virtual
int Crear_ArchivoBinario(const char *nombre, int size_IR) {

    int size_ArchivoBinario  = 131072;
    size_ArchivoBinario = size_ArchivoBinario  * size_IR;
    char *numeros = malloc(size_ArchivoBinario);
    /*if (numeros == NULL) {
        return 1;
    }*/
    //FILE *ArchivoBinario = fopen(nombre, "r+b");
    //NOTA: Aqui no usar r+b_______________________Peligroso
    //NOTA: no es necesrio poner la 'b' porque en unix todo se abre en binario

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



void in_TMS(int TMS[]){//tiene formato TMS[][1], sino especificas el numero de columnas, te marco error
    for(int i=0; i < 32768; i++){
        TMS[i] = 0;//inicializamos la tabla con todos los valores en 0
        
    }

}

int Busqueda_TMS(int TMS[]){
    for(int i=0; i < 32768; i++){ //buscamos aquel marco libre, como es no contigua, el primero que encuentre, agarra
        if(TMS[i] == 0){
            return i;//marco libre jeje
        }
    }
    return -1;
}

//paso los archivos tipo file, por que como lo vamos a utilizar en la función de reescritura, para poder escribir y leer el archivo
//necesitamos abrir los archivos FILE tal cual
int Paginacion(FILE *archivoProceso, FILE *swap, int PID, int TMS[], int TMP[][3]){
    char instruccion[100];//array que guarda instruccion
    char relleno[100]; //array que guarda lo que sobra
    int pagina = 0;

    while(1){// primero buscas un marco libre antes de poder escribirlo, pues si primero haces la lectura y no hay espacio, pss que haces xd
        int marco = Busqueda_TMS(TMS);
        if(marco == -1){
            return 1; // swap lleno
        }

        int instL = 0;//la necesitamos para leer la cantidad de lineas leidas, puede que una pagina al final solamente lea 2 instrucciones
        //además es nuestra condición de termino para el while, sino la tenemos nunca termina, pues sale cuando no lee ninguna linea
        fseek(swap, marco * 400, SEEK_SET);// se mueve al marco de página correspondiente

        for(int i = 0; i < 4; i++){
            if(fgets(instruccion, sizeof(instruccion), archivoProceso) == NULL){
                break;
            }
            int usados = strlen(instruccion);
            memset(relleno, '\0', sizeof(relleno));
            fwrite(instruccion, sizeof(char), usados, swap);
            fwrite(relleno, sizeof(char), 100 - usados, swap);
            instL++;
        }

        if(instL == 0){
            break;
        }
        TMS[marco] = PID;
        TMP[pagina][2] = marco;//Gurdar en la TMP el marco del SWAP
        pagina++;
    }
    return 0;
}
int reescritura(const char *NombrePro, FILE *ArchivoBinario, int pid, int TMS[], int TMP[][3]){
    FILE *archivoP = fopen(NombrePro, "r");

    if (archivoP == NULL) {
        //perror("Error al abrir el archivo");
        return 1;
    }
    /* if(ContadorL < 131072){
        Paginacion(archivoP,ArchivoBinario,pid,TMS,TMP,lista_nuevos,nombre);
    }else{
        mvprintw(y_mensajes,0,"ERROR: El proceso es mas grande que el swap.");
        refresh();
        return 1;
    }*/

    int resultado = Paginacion(archivoP, ArchivoBinario, pid, TMS, TMP);

    fclose(archivoP);
    return resultado;
}

void in_TMP(int TMP[][3]){

    for(int i = 0; i < 32768; i++){
        TMP[i][0] = 0;   // bit presencia: 0 = swap
        TMP[i][1] = -1;  // marco RAM
        TMP[i][2] = -1;  // marco swap
    }
}

void in_TMM(int TMM[][2]){
    for(int i=0; i < 16; i++){
        TMM[i][0] = 0;
        TMM[i][1] = 0; //BIT_REF para algoritmo de reloj
    }
}

int Busqueda_TMM(int TMM[][2]){
    for(int i=0; i < 16; i++){
        if(TMM[i][0]==0){
            return i;
        }
    }
    return -1;
}
//-------------------------------------------------------------

int BitPresencia_TMP(int TMP[][3], int pagina){
    return TMP[pagina][0];
}

int ObtenerMarcoSwap(int TMP[][3], int pagina){
    return TMP[pagina][2];
}

int ObtenerMarcoRAM(int TMP[][3], int pagina){
    return TMP[pagina][1];
}

void EscrituraRam(FILE *swap,char RAM[][400],int pagina,int TMP[][3],int TMM[][2], int PID){
   
    int marcoSwap = TMP[pagina][2];
    int marcoRAM = Busqueda_TMM(TMM);

    if(marcoRAM == -1){
        mvprintw(y_mensajes,0,"ERROR: Esta llena la RAM");
        refresh();
        return;
    }

    fseek(swap, marcoSwap * 400, SEEK_SET);

    fread(RAM[marcoRAM],sizeof(char),400,swap);

    TMP[pagina][0] = 1;
    TMP[pagina][1] = marcoRAM;

    TMM[marcoRAM][0] = PID;
    TMM[marcoRAM][1] = 1; //BIT_REF 
}

void in_RAM(char RAM[][400]){
    for(int i=0; i < 16; i++){
        for(int j=0; j < 400; j++){
            RAM[i][j] = '\0';
        }
    }
}

int RAMLlena(int TMM[][2]){
    for(int i = 0; i < 16; i++){
        if(TMM[i][0] == 0){
            return 0; // todavía hay espacio
        }
    }
    return 1; // RAM llena
}

int ContadorLineas(const char *archivo){
    int contador = 0;
    char buffer[100];

    FILE *archivoP = fopen(archivo, "r");

    if (archivoP == NULL) {
        //perror("Error al abrir el archivo");
        return 1
        ;
    }
    while (fgets(buffer, sizeof(buffer), archivoP) != NULL) {
        contador++;
    }

    fclose(archivoP);
    return contador;
}

int punteroReloj=0; //variable para ver en donde se quedo la manesilla
void AlgoritmoReloj(int TMM[][2], char RAM[][400]){
    while(1){
        if(TMM[punteroReloj][1] == 0){//expulsar marco
            int MarcoALiberar = punteroReloj;

            LiberarRAM(RAM, MarcoALiberar);

            TMM[punteroReloj][0] = -1; // marco libre
            TMM[punteroReloj][1] = 0;  // bit de uso limpio

            punteroReloj = (punteroReloj +1) %16 ; // avanza al siguiente marco 
            return;
            //return MarcoALiberar;
        }else{ //le da una segunda oportunidad
            TMM[punteroReloj][1] = 0;
             
            punteroReloj = (punteroReloj +1) %16; //para que avance en circulo //si llega al 15, reinicia a 0
        }
    }
}

void LiberarRAM(char RAM[][400], int marco){
    //un memtset para llenar de 0 ese marco
    memset(RAM[marco], 0, sizeof(int) * 400);
}