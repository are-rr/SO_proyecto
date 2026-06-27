#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>
#include <time.h>
#include "procesos.h"

// Funciones para las listas:

void insertar(struct Nodo **lista, int pid, int gid, int pc, int num_paginas, int num_lineas, const char *nombre)
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
    nuevo->TMP = NULL;
    nuevo->num_paginas = num_paginas;
    nuevo->num_lineas = num_lineas;
    nuevo->sig = NULL;

    if (*lista == NULL)
    {
        *lista = nuevo; // si la lista esta vacia, el nodo es la cabeza
        return;
    }

    struct Nodo *temp = *lista;
    while (temp->sig != NULL)
    { // noo esta vacia, recorre hasta el final
        temp = temp->sig;
    }
    temp->sig = nuevo; // inserta el nuevo nodo al final
}

void insertarFinal(struct Nodo **lista, struct Nodo *proceso)
{
    if (proceso == NULL)
    {
        return;
    }
    proceso->sig = NULL; // asegura eu no apunte a otro nodo

    if (*lista == NULL)
    {
        *lista = proceso;
        return;
    }

    struct Nodo *temp = *lista;
    while (temp->sig != NULL)
    {
        temp = temp->sig;
    }
    temp->sig = proceso;
}

// saca el primer proceso de la lista
struct Nodo *extraerPrimero(struct Nodo **lista)
{
    if (*lista == NULL)
    {
        return NULL;
    }
    struct Nodo *temp = *lista; // guarda el primer nodo
    *lista = (*lista)->sig;     // mueve la cabeza al siguiente //NOTA:investifar (*)
    temp->sig = NULL;           // desconecta el nodo
    return temp;
}

struct Nodo *extraerNodo(struct Nodo **lista, int pid)
{
    struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL)
    {
        if (actual->PID == pid)
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
    if (procesoError != NULL)
    {
        insertarFinal(lista_terminados, procesoError);
    }
}

struct Nodo *matar(struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos, struct Nodo **lista_nuevos, int pid)
{
    struct Nodo *proceso_mata = NULL;
    proceso_mata = extraerNodo(lista_ejecucion, pid); // busca en ejecucion
    if (proceso_mata != NULL)
    {
        insertarFinal(lista_terminados, proceso_mata);
        return proceso_mata;
    }

    if (proceso_mata == NULL)
    {
        proceso_mata = extraerNodo(lista_listos, pid); // sino busca en listos
        if (proceso_mata != NULL)
        {
            insertarFinal(lista_terminados, proceso_mata);
            return proceso_mata;
        }
    }

    if (proceso_mata == NULL)
    {
        proceso_mata = extraerNodo(lista_suspendidos, pid); // Buscamos en suspendidos
        if (proceso_mata != NULL)
        {
            insertarFinal(lista_terminados, proceso_mata);
            return proceso_mata;
        }
    }
    if (proceso_mata == NULL)
    {
        proceso_mata = extraerNodo(lista_nuevos, pid); // buscamos en nuevos
        if (proceso_mata != NULL)
        {
            insertarFinal(lista_terminados, proceso_mata);
            return proceso_mata;
        }
    }

    if (proceso_mata == NULL)
    {
        struct Nodo *aux = *lista_terminados; // por ultimo en terminados
        while (aux != NULL)
        { // pero solo busca, no lo mata, pues ya esta terminado
            if (aux->PID == pid)
            {
                limpiarZona(y_mensajes, 0, ancho_procesos);
                mvprintw(y_mensajes, 0, "El proceso con PID %d ya esta en terminados.", pid);
                refresh();
                return NULL;
            }
            aux = aux->sig;
        }
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "No se encontro el proceso con PID %d.", pid);
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
struct Nodo *buscarGID(struct Nodo *lista, int gid)
{
    struct Nodo *actual = lista;

    while (actual != NULL)
    {
        if (actual->GID == gid)
        {
            return actual;
        }
        actual = actual->sig;
    }
    return NULL;
}

struct Nodo *forkProceso(struct Nodo *proceso_original, int nuevo_pid, int nuevo_pc)
{
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo));

    if (nuevo == NULL)
    {
        return NULL;
    }

    nuevo->GID = proceso_original->GID;
    nuevo->PID = nuevo_pid;
    strcpy(nuevo->nombrePro, proceso_original->nombrePro);
    nuevo->EAX = 0;
    nuevo->EBX = 0;
    nuevo->ECX = 0;
    nuevo->EDX = 0;
    nuevo->CPU = 0;
    nuevo->GCPU = 0;
    nuevo->PRIORY = 0;
    nuevo->num_lineas = proceso_original->num_lineas;
    nuevo->num_paginas = proceso_original->num_paginas;

    strcpy(nuevo->IR, "");

    if (nuevo_pc >= 0)
    {
        nuevo->PC = nuevo_pc;
    }
    else
    {
        nuevo->PC = proceso_original->PC;
    }

    nuevo->sig = NULL;

    return nuevo;
}

struct Nodo *forkProcesoComando(struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos,
                                 int pid_comando, int nuevo_pid, int pc)
{
    struct Nodo *original = NULL;
    struct Nodo *nuevo = NULL;

    original = buscar(*lista_ejecucion, pid_comando);

    if (original == NULL)
    {
        original = buscar(*lista_listos, pid_comando);
    }
    if (original == NULL)
    {
        original = buscar(*lista_suspendidos, pid_comando);
    }

    if (original == NULL)
    {
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

    nuevo = forkProceso(original, nuevo_pid, pc);

    if (nuevo == NULL)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC invalido o no se pudo abrir el archivo");
        refresh();
        return NULL;
    }
    nuevo->num_paginas = original->num_paginas;
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

int CalculoPriodidad(struct Nodo **lista_listos, int grupos, int Base)
{
    int P, CPU, GCPU;
    struct Nodo *actual = *lista_listos;

    while (actual != NULL)
    {                                // recorre toda la lista de listos
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
int Busqueda_GID(struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int GID)
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
    //proceso->TIEMPO_SUSP = time(NULL) + 0;
    proceso->TIEMPO_SUSP = time(NULL) + tiempo;
}

void RevisarSuspendidos(FILE *swap, char RAM[][400], struct Nodo *lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int TMM[][2], int TMS[], int *porS, int *porR)
{
    struct Nodo *actual = *lista_suspendidos;
    struct Nodo *sig = NULL;
    time_t ahora = time(NULL); // sacamos el tiempo actual del sistema

    // mvprintw(4, 0, "revisando suspendidos");
    // refresh();
    while (actual != NULL)
    {
        sig = actual->sig; // avanza al siguiente nodo

        if (ahora >= actual->TIEMPO_SUSP)
        { // ver si se cumplio el tiempo en suspendidos
            int direccion_virtual = actual->PC;
            int pagina = direccion_virtual / 4; // pagina que se necesita cargar en RAM segun el PC

            if (RAMLlena(TMM))
            {
                AlgoritmoReloj(RAM, lista_ejecucion, *lista_listos, *lista_suspendidos, TMM);
                limpiarZonaTabla(y_renglon_TMM, x_TMM);
                imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
            }


            // no se usa &lista_suspendidos porque lista_suspendidos ya es un doble puntero
            struct Nodo *p = extraerNodo(lista_suspendidos, actual->PID);

            if (p != NULL)
            {
                p->TIEMPO_SUSP = 0; // limpiamos su tiempito
                if((!BusquedaBitP(lista_ejecucion,*lista_listos,*lista_suspendidos,p,pagina))){
                    EscrituraRam(swap, RAM, lista_ejecucion, *lista_listos, *lista_suspendidos, p, TMM, pagina); // cargamos la pagina que necesita el proceso
                }
                imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                limpiarZonaTabla(y_renglon_TMP, x_TMP);
                imprimir_TMP(p->TMP, y_renglon_TMP, x_TMP, p->num_paginas, p->PID);
                porcentajes(TMM, TMS, porS, porR);

                insertarFinal(lista_listos, p); // insertamos al final de listos
                // mvprintw(6, 0, "PID %d sale de suspendidos", p->PID);
                // refresh();
            }
        }

        actual = sig;
    }
}
void RevisarNuevos(FILE *swap, struct Nodo **lista_listos, struct Nodo **lista_nuevos, int TMS[])
{
    struct Nodo *procesoN = *lista_nuevos;
    struct Nodo *sig = NULL;
    int n = marcosLS(TMS);

    while (procesoN != NULL)
    {
        sig = procesoN->sig;
        if (procesoN->num_paginas <= n)
        {
            if (reescritura(swap, procesoN->nombrePro, procesoN->TMP, TMS, procesoN->PID) == 0)
            {
                extraerNodo(lista_nuevos, procesoN->PID);
                insertarFinal(lista_listos, procesoN);
            }
        }
        procesoN = sig;
    }
}

int procesarMata(FILE *swap, char RAM[][400], char archivo[], struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos, struct Nodo **lista_nuevos,
                 int TMM[][2], int TMS[], int num_palabras, int *grupos, int *porS, int *porR)
{
    if (num_palabras < 2)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: falta el PID del proceso");
        return 0;
    }
    if (!Digito(archivo))
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PID debe ser un entero");
        return 0;
    }
    if (num_palabras > 2)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
        return 0;
    }

    int num_PID = atoi(archivo);
    int lista = 0;

    struct Nodo *p_matar = buscar(*lista_ejecucion, num_PID);
    if (p_matar != NULL)
    {
        lista = 1; // esta en lista LISTOS
    }
    if (p_matar == NULL)
    {
        p_matar = buscar(*lista_listos, num_PID);
    }
    if (p_matar == NULL)
    {
        p_matar = buscar(*lista_suspendidos, num_PID);
    }
    if (p_matar == NULL)
    {
        p_matar = buscar(*lista_nuevos, num_PID);
        if (p_matar != NULL)
        {
            lista = 2; // esta en lista NUEVOS
        }
    }
    if (p_matar == NULL)
    {
        p_matar = buscar(*lista_terminados, num_PID);
    }
    if (p_matar == NULL)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: no existe el PID %d", num_PID);
        return 0;
    }

    int gid_matado = p_matar->GID;

    struct Nodo *pMata = matar(lista_ejecucion, lista_listos, lista_terminados, lista_suspendidos, lista_nuevos, num_PID);

    if (pMata == NULL)
    {
        return 0;
    }

    if (lista != 2 && Busqueda_GID(lista_ejecucion, lista_listos, lista_suspendidos, gid_matado) == 0)
    {
        liberarSWAP(swap, pMata->TMP, TMS, pMata->num_paginas);
        liberarRAMproceso(RAM, TMM, pMata->GID);
        RevisarNuevos(swap, lista_listos, lista_nuevos, TMS);
        // limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
        // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
        limpiarZonaTabla(y_renglon_TMM, x_TMM);
        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
        porcentajes(TMM, TMS, porS, porR);
        (*grupos)--;
    }
    if (lista == 2)
    {
        (*grupos)--;
    }
    imprimirEstado(*lista_listos, *lista_ejecucion, *lista_terminados, *lista_suspendidos, *lista_nuevos);
    limpiarZona(y_linea_comando, 0, ancho_procesos);
    refresh();
    return lista == 1; // 1 si mataste el que estaba en ejecución
}

int procesarFork(char archivo[], char extra[], struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos,
                 int num_palabras, int *pid)
{
    if (num_palabras < 2)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: falta pid del proceso");
        return 0;
    }
    if (num_palabras < 3)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: falta numero de instruccion");
        return 0;
    }
    if (num_palabras > 3)
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
        return 0;
    }
    if (Negativo(extra))
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC no puede ser negativo");
        return 0;
    }
    if (!Digito(archivo))
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PID debe ser un entero");
        return 0;
    }
    if (!Digito(extra))
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC debe ser un entero");
        return 0;
    }
    int num_PID = atoi(archivo);
    int num_PC = atoi(extra);
    (*pid)++;

    struct Nodo *nuevo = forkProcesoComando(lista_ejecucion, lista_listos, lista_terminados, lista_suspendidos, num_PID, *pid, num_PC);

    if (nuevo == NULL)
    {
        (*pid)--;
        return 0;
    }
    return 1;
}