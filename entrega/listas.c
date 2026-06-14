#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>
#include <time.h>
#include "procesos.h"

// Funciones para las listas:

void insertar(struct Nodo **cabeza, int pid, int gid, const char *nombre, int pc, int num_paginas,int num_lineas)
{
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo)); // reservar memoria para el nuevo nodo

    nuevo->PID = pid;
    nuevo->GID = gid;
    strncpy(nuevo->nombrePro, nombre, sizeof(nuevo->nombrePro) - 1); // strncpy(destino,origen,tamañp)
    nuevo->nombrePro[sizeof(nuevo->nombrePro) - 1] = '\0';           // se copia pues nombre es un dato termporal
    // nuevo-> Archivo = archivo;
    nuevo->PC = pc;
    nuevo->CPU = 0;
    nuevo->GCPU = 0;
    nuevo->PRIORY = 0;
    nuevo->EAX = 0;
    nuevo->EBX = 0;
    nuevo->ECX = 0;
    nuevo->EDX = 0;
    nuevo->IR[0] = '\0';
    nuevo -> TMP=NULL;
    nuevo->num_paginas = num_paginas; 
    nuevo->num_lineas = num_lineas;
    nuevo->sig = NULL;

    if (*cabeza == NULL)
    {
        *cabeza = nuevo; // si la lista esta vacia, el nodo es la cabeza
        return;
    }

    struct Nodo *temp = *cabeza;
    while (temp->sig != NULL)
    { // noo esta vacia, recorre hasta el final
        temp = temp->sig;
    }
    temp->sig = nuevo; // inserta el nuevo nodo al final
}

void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso)
{
    if (proceso == NULL)
    {
        return;
    }
    proceso->sig = NULL; // asegura eu no apunte a otro nodo

    if (*cabeza == NULL)
    {
        *cabeza = proceso;
        return;
    }

    struct Nodo *temp = *cabeza;
    while (temp->sig != NULL)
    {
        temp = temp->sig;
    }
    temp->sig = proceso;
}

// saca el primer proceso de la lista
struct Nodo *extraerPrimero(struct Nodo **cabeza)
{
    if (*cabeza == NULL)
    {
        return NULL;
    }
    struct Nodo *temp = *cabeza; // guarda el primer nodo
    *cabeza = (*cabeza)->sig;    // mueve la cabeza al siguiente
    temp->sig = NULL;            // desconecta el nodo
    return temp;
}

struct Nodo *extraerNodo(struct Nodo **lista, int id)
{
    struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL)
    {
        if (actual->PID == id)
        {
            if (anterior == NULL)
            { // si es el primer nodo en la lista, no tiene anterior
                *lista = actual->sig;
            }
            else
            {                                // desconectamos el nodo
                anterior->sig = actual->sig; // nodo anterior apunta al sig del actual
            }

            actual->sig = NULL; // desenlazamos el nodo de la lista
            return actual;
        }

        anterior = actual;
        actual = actual->sig;
    }

    return NULL; // No encontrado
}

int contarNodos(struct Nodo *lista)
{
    int num = 0;
    while (lista != NULL)
    {
        num++;
        lista = lista->sig;
    }
    return num;
}

void A_terminadosError(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados)
{
    struct Nodo *procesoError = extraerPrimero(lista_ejecucion);
    if (procesoError != NULL){
        insertarFinal(lista_terminados, procesoError); 
    }
}

struct Nodo *matar(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos,struct Nodo **lista_suspendidos,struct Nodo **lista_nuevos, int id_p)
{
    struct Nodo *proceso_mata = NULL;
    // NOTA: Por que no le agregamos & en este caso porque extraer nodo es **
    proceso_mata = extraerNodo(lista_ejecucion, id_p); // busca en ejecucion
    if (proceso_mata != NULL){
        insertarFinal(lista_terminados, proceso_mata);
        return proceso_mata;
    }

    if (proceso_mata == NULL){
        proceso_mata = extraerNodo(lista_listos, id_p); // sino busca en listos
        if (proceso_mata != NULL){
            insertarFinal(lista_terminados, proceso_mata);
            return proceso_mata;
        }
    }

    if(proceso_mata == NULL){
        proceso_mata = extraerNodo(lista_suspendidos, id_p); //Buscamos en suspendidos
        if(proceso_mata != NULL){
            insertarFinal(lista_terminados, proceso_mata);
            return proceso_mata;
        }
    }
    if (proceso_mata == NULL){
        proceso_mata = extraerNodo(lista_nuevos, id_p); //buscamos en nuevos
        if (proceso_mata != NULL){
            insertarFinal(lista_terminados, proceso_mata);
            return proceso_mata;
        }
    }

    if (proceso_mata == NULL){
        struct Nodo *aux = *lista_terminados; // por ultimo en terminados
        while (aux != NULL){ // pero solo busca, no lo mata, pues ya esta terminado
            if (aux->PID == id_p)
            {
                limpiarZona(y_mensajes, 0, ancho_procesos);
                mvprintw(y_mensajes, 0, "El proceso con PID %d ya esta en terminados.", id_p);
                refresh();
                return NULL;
            }
            aux = aux->sig;
        }
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "No se encontro el proceso con PID %d.", id_p);
        refresh();
        return NULL;
    }
    return NULL;
}

struct Nodo *buscar(struct Nodo *lista, int pid)
{
    struct Nodo *actual = lista;

    while (actual != NULL)
    {
        if (actual->PID == pid)
        {
            return actual;
        }
        actual = actual->sig;
    }
    return NULL;
}
// Solo valida que el PC sea correcto para el Archivo(que no sobrepase el numero de lineas que tiene el archivo)
int validarPC(FILE *copiaArchivo, int pc_buscar)
{
    char buffer[200];

    for (int i = 0; i < pc_buscar; i++)
    {
        if (fgets(buffer, sizeof(buffer), copiaArchivo) == NULL)
        {
            return 0;
        }
    }
    return 1;
}

struct Nodo *forkProceso(struct Nodo *original, int nuevo_pid, int nuevo_pc, int nuevo_gid)
{
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo));

    if (nuevo == NULL)
    {
        return NULL;
    }

    nuevo->GID = nuevo_gid;
    nuevo->PID = nuevo_pid;
    strcpy(nuevo->nombrePro, original->nombrePro);
    nuevo->Archivo = fopen(original->nombrePro, "r"); // requiere tener su propio puntero

    if (nuevo->Archivo == NULL)
    {
        return NULL;
    }

    nuevo->EAX = 0;
    nuevo->EBX = 0;
    nuevo->ECX = 0;
    nuevo->EDX = 0;
    nuevo->CPU = 0;
    nuevo->GCPU = 0;
    nuevo->PRIORY = 0;

    strcpy(nuevo->IR, "");

    if (nuevo_pc >= 0){
        nuevo->PC = nuevo_pc;
    }
    else{
        nuevo->PC = original->PC;
    }

    if (validarPC(nuevo->Archivo, nuevo->PC) == 0){
        mvprintw(y_mensajes, 0, "Error: PC invalido");
        return NULL;
    }

    nuevo->sig = NULL;

    return nuevo;
}

struct Nodo *forkProcesoComando(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int pid_comando, int pc, int nuevo_pid){
    struct Nodo *original = NULL;
    struct Nodo *nuevo = NULL;

    original = buscar(*lista_ejecucion, pid_comando);

    if (original == NULL)
    {
        original = buscar(*lista_listos, pid_comando);
    }
    if (original == NULL){
        original = buscar(*lista_suspendidos, pid_comando);
    }

    if (original == NULL){
        if (buscar(*lista_terminados, pid_comando) != NULL)
        {
            limpiarZona(y_mensajes, 0, ancho_procesos);
            mvprintw(y_mensajes, 0, "ERROR: no se puede duplicar un proceso terminado");
            refresh();
            return NULL;
        }

        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: no existe el PID %d", pid_comando);
        refresh();
        return NULL;
    }

    nuevo = forkProceso(original, nuevo_pid, pc, original->GID);

    if (nuevo == NULL)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC invalido o no se pudo abrir el archivo");
        refresh();
        return NULL;
    }
    nuevo->num_paginas = original->num_paginas;//NOTA:analisar lineas para el proceso hijo de la TMP
    nuevo->TMP = malloc(nuevo->num_paginas * sizeof(int[3]));

    if (nuevo->TMP == NULL)
    {
        free(nuevo);
        return NULL;
    }

    memcpy(nuevo->TMP, original->TMP, nuevo->num_paginas * sizeof(int[3]));

    insertarFinal(lista_listos, nuevo);

    limpiarZona(y_mensajes, 0, ancho_procesos);
    mvprintw(y_mensajes, 0, "Proceso duplicado");
    refresh();

    return nuevo;
}

int CalculoPriodidad(struct Nodo **nodolis, int grupos, int Base)
{
    int P, CPU, GCPU;
    struct Nodo *actual = *nodolis;

    while (actual != NULL){                                // recorre toda la lista de listos
        CPU = actual->CPU * 1 / 2;   // CPU/2
        GCPU = actual->GCPU * 1 / 2; // GCPU/2
        actual->CPU = CPU;
        actual->GCPU = GCPU;
        P = Base + (CPU * 1 / 2) + ((GCPU * grupos) * 1 / 4); // Base + CPU/2 + GCPU/(4*Wk)
        actual->PRIORY = P;
        actual = actual->sig;
    }

    return 0;
}

struct Nodo *extraerNodo_Prioridad(struct Nodo **lista, int priory)
{
    struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL)
    {
        if (actual->PRIORY == priory)
        {
            if (anterior == NULL)
            { // si es el primer nodo en la lista, no tiene anterior
                *lista = actual->sig;
            }
            else
            {                                // desconectamos el nodo
                anterior->sig = actual->sig; // nodo anterior apunta al sig del actual
            }

            actual->sig = NULL; // desenlazamos el nodo de la lista
            return actual;
        }

        anterior = actual;
        actual = actual->sig;
    }

    return NULL; // No encontrado
}

struct Nodo *Fair_Share(struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int grupos, int Base)
{
    struct Nodo *actual = *lista_listos;
    struct Nodo *anterior = NULL;

    CalculoPriodidad(lista_listos, grupos, Base);
    CalculoPriodidad(lista_suspendidos, grupos, Base);

    // iniciar en la cabeza de la lista
    int prioridad;
    int prioridad_A;
    while (actual != NULL)
    {
        prioridad_A = actual->PRIORY;

        if (anterior == NULL)
        { // significa que es el primer proceso de la lista y no hay con quien comparar todavia
            prioridad = prioridad_A;
        }
        else if (prioridad_A < prioridad)
        {
            prioridad = prioridad_A;
        }

        anterior = actual;
        actual = actual->sig;
    }
    return extraerNodo_Prioridad(lista_listos, prioridad);
}

// Para todo proceso de un grupo se le asigna el GCPU en caso de que se actualice
void GCPU_Global(struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int GID, int GCPU)
{
    struct Nodo *actual = *lista_listos;

    while (actual != NULL)
    {
        if (actual->GID == GID)
        {
            actual->GCPU = GCPU;
        }
        actual = actual->sig;
    }

    actual = *lista_suspendidos;

    while (actual != NULL)
    {
        if (actual->GID == GID)
        {
            actual->GCPU = GCPU;
        }
        actual = actual->sig;
    }
}

// Para saber cuantos grupos tenemos en caso de que usemos "mata" o mandemos un proceso a terminados
int Busqueda_GID(struct Nodo **lista_listos, struct Nodo **lista_ejecucion, struct Nodo **lista_suspendidos, int GID) // NOTA: para que compartan en lista_suspendidosgit
{
    struct Nodo *actual = *lista_listos;
    int grupos_restantes = 0;

    while (actual != NULL)
    {
        if (actual->GID == GID)
        { // Encontro un proceso con el mismo GID
            grupos_restantes++;
        }
        actual = actual->sig;
    }

    actual = *lista_ejecucion; // se busca por si el unico proceso del grupo esta ejecutandose
    while (actual != NULL)
    {
        if (actual->GID == GID)
        {
            grupos_restantes++;
        }
        actual = actual->sig;
    }

    actual = *lista_suspendidos; // se busca por si el unico proceso del grupo esta ejecutandose
    while (actual != NULL)
    {
        if (actual->GID == GID)
        {
            grupos_restantes++;
        }
        actual = actual->sig;
    }
    if (grupos_restantes == 0)
    { // ya no hay procesos con ese GID
        return 0;
    }
    return 1;
}

void TiempoEnSuspendidos(struct Nodo *proceso)
{
    srand(time(NULL));
    int tiempo = rand() % 9 + 2; // entre 2 a 10 segundos
    proceso->TIEMPO_SUSP = time(NULL) + tiempo;
}

void RevisarSuspendidos(struct Nodo **lista_suspendidos, struct Nodo **lista_listos,struct Nodo *lista_ejecucion,FILE *swap,char RAM[][400],int TMM[][2],int TMS[],int *porS, int *porR)
{
    struct Nodo *actual = *lista_suspendidos;
    struct Nodo *sig = NULL;
    time_t ahora = time(NULL);

    // mvprintw(4, 0, "Revisando suspendidos...");
    // refresh();

    while (actual != NULL){
        sig = actual->sig;

        if (ahora >= actual->TIEMPO_SUSP){
            int direccion_virtual = actual->PC;
            int pagina = direccion_virtual / 4;
            //NOTA: por si truena, si no, no,si PC excede las lineas del proceso puede queda en error de segmento por que se recomienda verificar antes
            /*if (pagina < 0 || pagina >= p->num_paginas) {
                mvprintw(y_mensajes, 0,"ERROR: pagina %d fuera de rango (%d)", pagina, p->num_paginas);
                refresh();
                return;
            }*/ 

            if(RAMLlena(TMM)){
                AlgoritmoReloj(TMM, RAM, *lista_listos, lista_ejecucion, *lista_suspendidos);
                limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
            }

            struct Nodo *p = extraerNodo(lista_suspendidos, actual->PID); // NOTA: Por que no usamos & aqui si es doble puntero??

            if (p != NULL){
                p->TIEMPO_SUSP = 0;
                
                //Aqui algoritmo de reloj
                
                EscrituraRam(swap, RAM, pagina, p->TMP, TMM, p->PID);
                imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                limpiarZonaTabla(y_renglon_TMP, x_TMP, ancho_TMP);
                imprimir_TMP(p->TMP, y_renglon_TMP, x_TMP, p->num_paginas, p->PID);
                porcentajes(TMS, TMM, porS, porR);

                insertarFinal(lista_listos, p);
                // mvprintw(6, 0, "PID %d sale de suspendidos", p->PID);
                // refresh();
            }
        }

        actual = sig;
    }
}
void RevisarNuevos( struct Nodo **lista_nuevos,struct Nodo **lista_listos,FILE *swap,int TMS[]){
    struct Nodo *procesoN = *lista_nuevos;
    struct Nodo *sig = NULL;
    int n = marcosLS(TMS);

    while(procesoN != NULL){
        sig = procesoN->sig;
        if(procesoN->num_paginas <= n){
            if (reescritura(procesoN->nombrePro, swap, procesoN->PID, TMS, procesoN->TMP) == 0){
                extraerNodo(lista_nuevos, procesoN->PID);
                insertarFinal(lista_listos, procesoN);
            }
        }
        procesoN=sig;
    }

}

int procesarMata(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados,struct Nodo **lista_listos, struct Nodo **lista_suspendidos,struct Nodo **lista_nuevos,int num_palabras,char archivo[],FILE *swap,char RAM[][400],int TMM[][2],int TMS[],int *grupos,int *porS,int *porR){
    if (num_palabras < 2){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: falta el PID del proceso");
        return 0;
    }
    if (!Digito(archivo)){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PID debe ser un entero");
        return 0;
    }
    if (num_palabras > 2){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
        return 0;
    }

    int num_PID = atoi(archivo);
    int lista = 0; 

    struct Nodo *p_matar = buscar(*lista_ejecucion, num_PID);
    if (p_matar != NULL){
        lista = 1; // esta en lista LISTOS
    }
    if (p_matar == NULL){
        p_matar = buscar(*lista_listos, num_PID);
    }
    if (p_matar == NULL){
        p_matar = buscar(*lista_suspendidos, num_PID);
    }
    if (p_matar == NULL){
        p_matar = buscar(*lista_nuevos, num_PID);
        if (p_matar != NULL){
            lista = 2; //esta en lista NUEVOS
        }
    }

    if (p_matar == NULL){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: no existe el PID %d", num_PID);
        return 0;
    }

    int gid_matado = p_matar->GID;

    struct Nodo *pMata = matar(lista_ejecucion,lista_terminados,lista_listos,lista_suspendidos,lista_nuevos,num_PID);

    if (pMata == NULL){
        return 0;
    }

    if (lista != 2 && Busqueda_GID(lista_listos, lista_ejecucion, lista_suspendidos, gid_matado) == 0){
        liberarSWAP(swap, pMata->TMP, pMata->num_paginas, TMS);
        liberarRAMproceso(RAM, TMM, pMata->PID);
        RevisarNuevos(lista_nuevos, lista_listos, swap, TMS);
        limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
        imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
        limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
        porcentajes(TMS, TMM, porS, porR);
        (*grupos)--;
    }
    if (lista == 2){
        (*grupos)--;
    }
    imprimirEstado(*lista_listos, *lista_ejecucion, *lista_terminados,*lista_suspendidos, *lista_nuevos);
    limpiarZona(y_linea_comando, 0, ancho_procesos);
    refresh();
    return lista == 1; // 1 si mataste el que estaba en ejecución
}

int procesarFork(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados,struct Nodo **lista_listos,struct Nodo **lista_suspendidos,int num_palabras,char archivo[],char extra[],int *pid)
{
    if (num_palabras < 2){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: falta pid del proceso");
        return 0;
    }
    if (num_palabras < 3){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: falta numero de instruccion");
        return 0;
    }
    if (num_palabras > 3){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
        return 0;
    }
    if (Negativo(extra)){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC no puede ser negativo");
        return 0;
    }
    if (!Digito(archivo)){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PID debe ser un entero");
        return 0;
    }
    if (!Digito(extra)){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC debe ser un entero");
        return 0;
    }
    int num_PID = atoi(archivo);
    int num_PC = atoi(extra);
    (*pid)++;

    struct Nodo *nuevo = forkProcesoComando(lista_ejecucion,lista_terminados, lista_listos,lista_suspendidos, num_PID,num_PC,*pid);

    if (nuevo == NULL){
        (*pid)--;
        return 0;
    }
    return 1;
}