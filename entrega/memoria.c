#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "procesos.h"

// memoria virtual
int Crear_ArchivoBinario(const char *nombre, int size_IR)
{
    int size_ArchivoBinario = 131072;
    size_ArchivoBinario = size_ArchivoBinario * size_IR;
    char *numeros = malloc(size_ArchivoBinario);
    // NOTA: no es necesrio poner la 'b' porque en unix todo se abre en binario

    FILE *ArchivoBinario = fopen(nombre, "wb");
    if (ArchivoBinario == NULL){
        return 1;
    }

    memset(numeros, 0, size_ArchivoBinario); 
    fwrite(numeros, 1, size_ArchivoBinario, ArchivoBinario); 
    fclose(ArchivoBinario);
    free(numeros);
    return 0;
}

void in_TMS(int TMS[]){
    for (int i = 0; i < 32768; i++){
        TMS[i] = 0; // inicializamos la tabla con todos los valores en 0
    }
}

int Busqueda_TMS(int TMS[]){
    for (int i = 0; i < 32768; i++){ // buscamos aquel marco libre, como es no contigua, el primero que encuentre, agarra
        if (TMS[i] == 0)
        {
            return i; // marco libre jeje
        }
    }
    return -1;
}

// paso los archivos tipo file, por que como lo vamos a utilizar en la función de reescritura, para poder escribir y leer el archivo
// necesitamos abrir los archivos FILE tal cual
int Paginacion(FILE *archivoProceso, FILE *swap, int PID, int TMS[], int TMP[][3]){
    char instruccion[100];
    char relleno[100];
    int pagina = 0;

    while (1)
    {
        int instL = 0;

        // leer primer instruccion para ver si acabo el archivo
        if (fgets(instruccion, sizeof(instruccion), archivoProceso) == NULL){
            break; // ya no hay nada que guardar
        }

        // se busca el marco
        int marco = Busqueda_TMS(TMS);
        if (marco == -1){
            return 1; //swap lleno
        }

        fseek(swap, marco * 400, SEEK_SET);

        //guardamos la instruccion
        int usados = strlen(instruccion);
        memset(relleno, '\0', sizeof(relleno));
        fwrite(instruccion, sizeof(char), usados, swap);
        fwrite(relleno, sizeof(char), 100 - usados, swap);
        instL++;

        // ahora guardamos las otras 3 si hay
        for (int i = 1; i < 4; i++){
            if (fgets(instruccion, sizeof(instruccion), archivoProceso) == NULL){
                break;
            }

            usados = strlen(instruccion);
            memset(relleno, '\0', sizeof(relleno));
            fwrite(instruccion, sizeof(char), usados, swap);
            fwrite(relleno, sizeof(char), 100 - usados, swap);
            instL++;
        }
        //actualizamos TMP Y TMS
        TMS[marco] = PID;
        TMP[pagina][2] = marco;
        pagina++;
    }

    return 0;
}
int reescritura(const char *NombrePro, FILE *ArchivoBinario, int pid, int TMS[], int TMP[][3]){
    FILE *archivoP = fopen(NombrePro, "r");

    int resultado = Paginacion(archivoP, ArchivoBinario, pid, TMS, TMP);

    fclose(archivoP);
    return resultado;
}

void in_TMP(int TMP[][3], int num_paginas)
{
    for (int i = 0; i < num_paginas; i++){
        TMP[i][0] = 0;  // bit presencia
        TMP[i][1] = -1; // marco RAM
        TMP[i][2] = -1; // marco swap
    }
}

void in_TMM(int TMM[][2])
{
    for (int i = 0; i < 16; i++)
    {
        TMM[i][0] = 0;
        TMM[i][1] = 0; // BIT_REF para algoritmo de reloj
    }
}

int Busqueda_TMM(int TMM[][2])
{
    for (int i = 0; i < 16; i++){
        if (TMM[i][0] == 0){
            return i;
        }
    }
    return -1;
}

int BitPresencia_TMP(int TMP[][3], int pagina){
    return TMP[pagina][0];
}

void EscrituraRam(FILE *swap, char RAM[][400], int pagina, int TMP[][3], int TMM[][2], int PID)
{

    int marcoSwap = TMP[pagina][2];
    int marcoRAM = Busqueda_TMM(TMM);

    if (marcoRAM == -1){
        mvprintw(y_mensajes, 0, "ERROR: Esta llena la RAM");
        refresh();
        return;
    }

    fseek(swap, marcoSwap * 400, SEEK_SET);

    fread(RAM[marcoRAM], sizeof(char), 400, swap);

    TMP[pagina][0] = 1;
    TMP[pagina][1] = marcoRAM;

    TMM[marcoRAM][0] = PID;
    TMM[marcoRAM][1] = 1; // BIT_REF
}

int RAMLlena(int TMM[][2])
{
    for (int i = 0; i < 16; i++){
        if (TMM[i][0] == 0){
            return 0; // todavía hay espacio
        }
    }
    return 1; // RAM llena
}

int ContadorLineas(const char *archivo)
{
    int contador = 0;
    char buffer[100];

    FILE *archivoP = fopen(archivo, "r");

    if (archivoP == NULL){
        return 1;
    }
    while (fgets(buffer, sizeof(buffer), archivoP) != NULL){
        contador++;
    }

    fclose(archivoP);
    return contador;
}

int punteroReloj = 0; // variable para ver en donde se quedo la manesilla
void AlgoritmoReloj(int TMM[][2], char RAM[][400] ,struct Nodo *lista_listos, struct Nodo *lista_ejecucion, struct Nodo *lista_suspendidos)
{
    while (1)
    {
        if (TMM[punteroReloj][1] == 0){ // expulsar marco
            int MarcoALiberar = punteroReloj;
            int pidDueno = TMM[punteroReloj][0];

            //actualizamos la TMP del dueño del marco
            actualizarTMPdeMarcoL(lista_listos, lista_ejecucion, lista_suspendidos, pidDueno, MarcoALiberar);

            LiberarRAM(RAM, MarcoALiberar);

            TMM[punteroReloj][0] = 0; // marco libre
            TMM[punteroReloj][1] = 0; // bit de uso limpio
            
            punteroReloj = (punteroReloj + 1) % 16; // avanza al siguiente marco
            return;
            // return MarcoALiberar;
        }
        else
        { // le da una segunda oportunidad
            TMM[punteroReloj][1] = 0;
            punteroReloj = (punteroReloj + 1) % 16; // para que avance en circulo //si llega al 15, reinicia a 0
        }
    }
}

void actualizarTMPdeMarcoL(struct Nodo *lista_listos,struct Nodo *lista_ejecucion, struct Nodo *lista_suspendidos, int pidDueno,int marcoLiberado){
    struct Nodo *p = buscar(lista_ejecucion, pidDueno);
    if(p== NULL){
        p = buscar(lista_listos, pidDueno);
    }
    if(p == NULL){
        p = buscar(lista_suspendidos, pidDueno);
    }
    if (p == NULL){
        return;
    }
    for (int i = 0; i < p->num_paginas;i++){
        if(p->TMP[i][1]==marcoLiberado){
            p->TMP[i][0] = 0;
            p->TMP[i][1] = -1;
            return;
        }
    }
}

void LiberarRAM(char RAM[][400], int marco)
{
    memset(RAM[marco], '\0', sizeof(RAM[marco]));
}

int validarArchivo(const char *NombrePro){
    FILE *archivoP = fopen(NombrePro, "r");

    if (archivoP == NULL){
        return 1;
    }

    fclose(archivoP);
    return 0;
}

void liberarSWAP(FILE *swap,int TMP[][3],int num_paginas, int TMS[]){
    char relleno[400];
    memset(relleno, 0, sizeof(relleno));

    for (int i = 0; i < num_paginas; i++){
        int ms = TMP[i][2];

        if(ms<0){
            continue; //-1
        }
        fseek(swap, ms * 400, SEEK_SET);
        fwrite(relleno, sizeof(char), 400, swap);

        TMS[ms] = 0;
        //actualizamos la TMP 
        TMP[i][0] = 0;
        TMP[i][1] = -1;
        TMP[i][2] = -1;
    }
}

void liberarRAMproceso(char RAM[][400], int TMM[][2], int pid)
{
    for (int i = 0; i < 16; i++)
    {
        if (TMM[i][0] == pid)
        {
            memset(RAM[i], '\0', 400);

            TMM[i][0] = 0; // marco libre
            TMM[i][1] = 0; // bit reloj limpio
        }
    }
}

int marcosLS(int TMS[]){
    int marcos = 0;
    for (int i = 0; i < 32768;i++){
        if(TMS[i] == 0){
            marcos++;
        }
    }
    return marcos;
}

int marcosLR(int TMM[][2]){
    int marcos = 0;
    for (int i = 0; i < 16; i++)
    {
        if (TMM[i][0] == 0)
        {
            marcos++;
        }
    }
    return marcos;
}
void porcentajes(int TMS[], int TMM[][2], int *porS, int *porR){
    int marcosR = marcosLR(TMM);
    int marcosS = marcosLS(TMS);
   // mvprintw(35, 195, "marcosS: %d", marcosS);

    *porS = 32768 - marcosS;
    *porR = 16 - marcosR;

    limpiarZona(33, 195, 30);
    limpiarZona(34, 195, 30);
    mvprintw(34, 195, "RAM en uso: %d/16 marcos", *porR);
    mvprintw(33, 195, "SWAP en uso: %d/32768 paginas", *porS);
    refresh();
}