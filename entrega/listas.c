#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>
#include "procesos.h"

//Funciones para las listas:

void insertar(struct Nodo **cabeza, int pid,int gid,const char *nombre, int pc) {
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo)); //reservar memoria para el nuevo nodo
    
    nuevo->PID = pid;
    nuevo ->GID = gid;
    strncpy(nuevo->nombrePro, nombre, sizeof(nuevo->nombrePro) - 1); //strncpy(destino,origen,tamañp)
    nuevo->nombrePro[sizeof(nuevo->nombrePro) - 1] = '\0';//se copia pues nombre es un dato termporal
    //nuevo-> Archivo = archivo;
    nuevo->PC = pc;
    nuevo -> CPU = 0;
    nuevo -> GCPU = 0;
    nuevo -> PRIORY = 0;
    nuevo->EAX = 0;
    nuevo->EBX = 0;
    nuevo->ECX = 0;
    nuevo->EDX = 0;
    nuevo->IR[0] = '\0';

    nuevo->sig = NULL;

    if (*cabeza == NULL) {
        *cabeza = nuevo; //si la lista esta vacia, el nodo es la cabeza
        return;
    }

    struct Nodo *temp = *cabeza;
    while (temp->sig != NULL) { //noo esta vacia, recorre hasta el final
        temp = temp->sig;
    }
    temp->sig = nuevo;//inserta el nuevo nodo al final
}


void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso) {
    if (proceso == NULL) {
        return;
    }
    proceso->sig = NULL;//asegura eu no apunte a otro nodo

    if (*cabeza == NULL) {
        *cabeza = proceso;
        return;
    }

    struct Nodo *temp = *cabeza;
    while (temp->sig != NULL) {
        temp = temp->sig;
    }
    temp->sig = proceso;
}

// saca el primer proceso de la lista
struct Nodo *extraerPrimero(struct Nodo **cabeza) {
    if (*cabeza == NULL) {
        return NULL;
    }
    struct Nodo *temp = *cabeza; //guarda el primer nodo
    *cabeza = (*cabeza)->sig; //mueve la cabeza al siguiente
    temp->sig = NULL; //desconecta el nodo
    return temp;
}

struct Nodo* extraerNodo(struct Nodo **lista, int id) {
    struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL) {
        if (actual->PID == id) {
            if (anterior == NULL) { //si es el primer nodo en la lista, no tiene anterior
                *lista = actual->sig;
            } else {//desconectamos el nodo
                anterior->sig = actual->sig; //nodo anterior apunta al sig del actual
            }

            actual->sig = NULL;// desenlazamos el nodo de la lista
            return actual;
        }

        anterior = actual; 
        actual = actual->sig;
    }

    return NULL; // No encontrado
}

int contarNodos(struct Nodo *lista){
    int num =0;
    while(lista != NULL){
        num++;
        lista = lista->sig;
    }
    return num;
}

void A_terminadosError(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados){
    struct Nodo *procesoError = extraerPrimero(lista_ejecucion);
    if(procesoError != NULL){
        procesoError->Status = 4;
        if(procesoError->Archivo != NULL){
            fclose(procesoError -> Archivo);
            procesoError -> Archivo = NULL;
        }
        insertarFinal(lista_terminados, procesoError);
    }
}

int matar(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, int id_p) {
    struct Nodo *proceso_mata = NULL;

    proceso_mata = extraerNodo(lista_ejecucion, id_p); //busca en ejecucion
    if(proceso_mata != NULL){
        //proceso_mata -> Status = 'Z';
        if(proceso_mata->Archivo != NULL){
            fclose(proceso_mata -> Archivo);
            proceso_mata -> Archivo = NULL;
        }
        insertarFinal(lista_terminados, proceso_mata);
        return 1;
    }

    if (proceso_mata == NULL) {
        proceso_mata = extraerNodo(lista_listos, id_p); //sino busca en listos
        if(proceso_mata != NULL){
            //proceso_mata -> Status = 'Z';
            if(proceso_mata->Archivo != NULL){
                fclose(proceso_mata -> Archivo);
                proceso_mata -> Archivo = NULL;
            }
            insertarFinal(lista_terminados, proceso_mata);
            return 2;
        }
    }

    if (proceso_mata == NULL) {
        struct Nodo *aux = *lista_terminados; //por ultimo en terminados
        while (aux != NULL) {                //pero solo busca, no lo mata, pues ya esta terminado
            if (aux->PID == id_p) {
                move(y_mensajes, 0); clrtoeol();
                mvprintw(y_mensajes,0,"El proceso con PID %d ya esta en terminados.", id_p);
                refresh();
                return 3;
            }
            aux = aux->sig;
        }
        move(y_mensajes, 0); clrtoeol();
        mvprintw(y_mensajes,0,"No se encontro el proceso con PID %d.", id_p);
        refresh();
        return 0;
    }
    return 0;
}

struct Nodo *buscar(struct Nodo *lista,int pid){
    struct Nodo *actual = lista;

    while (actual != NULL) {
        if (actual->PID == pid) {
            return actual;
        } 
        actual = actual->sig;
    }
    return NULL; 
}
//Solo valida que el PC sea correcto para el Archivo(que no sobrepase el numero de lineas que tiene el archivo)
int validarPC(FILE *copiaArchivo,int pc_buscar){
    char buffer[200];

    for(int i=0;i<pc_buscar;i++){
        if(fgets(buffer,sizeof(buffer), copiaArchivo)==NULL){
            return 0;
        }  

    }
    return 1;
}

struct Nodo* forkProceso(struct Nodo *original, int nuevo_pid, int nuevo_pc, int nuevo_gid) {
    struct Nodo *nuevo = (struct Nodo*) malloc(sizeof(struct Nodo));

    if (nuevo == NULL) {
        return NULL;
    }

    nuevo->GID = nuevo_gid;
    nuevo->PID = nuevo_pid;
    strcpy(nuevo->nombrePro, original->nombrePro);
    nuevo->Archivo = fopen(original->nombrePro, "r"); //requiere tener su propio puntero

    if (nuevo->Archivo == NULL) {
        return NULL;
    }

    nuevo->EAX = 0;
    nuevo->EBX = 0;
    nuevo->ECX = 0;
    nuevo->EDX = 0;

    strcpy(nuevo->IR, "");

    if (nuevo_pc >= 0) {
        nuevo->PC = nuevo_pc;
    } else {
        nuevo->PC = original->PC;
    }

    if (validarPC(nuevo->Archivo, nuevo->PC) == 0) {
        mvprintw(y_mensajes,0,"Error: PC invalido");
        return NULL;
    }

    nuevo->sig = NULL;

    return nuevo;
}

struct Nodo *forkProcesoComando(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados,struct Nodo **lista_listos,int pid_comando,int pc,int nuevo_pid) {
    struct Nodo *original = NULL;
    struct Nodo *nuevo = NULL;

    original = buscar(*lista_ejecucion, pid_comando);

    if (original == NULL) {
        original = buscar(*lista_listos, pid_comando);
    }

    if (original == NULL) {
        if (buscar(*lista_terminados, pid_comando) != NULL) {
            move(y_mensajes, 0); 
            clrtoeol();
            mvprintw(y_mensajes, 0, "ERROR: no se puede duplicar un proceso terminado");
            refresh();
            return NULL;
        }

        move(y_mensajes, 0); 
        clrtoeol();
        mvprintw(y_mensajes, 0, "ERROR: no existe el PID %d", pid_comando);
        refresh();
        return NULL;
    }

    nuevo = forkProceso(original, nuevo_pid,pc,original->GID);

    if (nuevo == NULL) {
        move(y_mensajes, 0); 
        clrtoeol();
        mvprintw(y_mensajes, 0, "ERROR: PC invalido o no se pudo abrir el archivo");
        refresh();
        return NULL;
    }

    insertarFinal(lista_listos, nuevo);

    move(y_mensajes, 0);
    clrtoeol();
    mvprintw(y_mensajes, 0, "Proceso duplicado");
    refresh();

    return nuevo;
}

int CalculoPriodidad(struct Nodo **nodolis, int grupos, int Base){
    int P,CPU,GCPU;
    struct Nodo *actual = *nodolis;

    while (actual != NULL) { //recorre toda la lista de listos
        CPU=actual->CPU*1/2;    //CPU/2
        GCPU= actual->GCPU*1/2; //GCPU/2
        actual-> CPU = CPU;
        actual -> GCPU = GCPU;
        P = Base+(CPU*1/2)+((GCPU*grupos)*1/4); //Base + CPU/2 + GCPU/(4*Wk)
        actual ->PRIORY =P;
        actual = actual->sig;
    }

    return 0;
}

struct Nodo* extraerNodo_Prioridad(struct Nodo **lista, int priory) {
    struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL) {
        if (actual->PRIORY == priory) {
            if (anterior == NULL) { //si es el primer nodo en la lista, no tiene anterior
                *lista = actual->sig;
            } else {//desconectamos el nodo
                anterior->sig = actual->sig; //nodo anterior apunta al sig del actual
            }

            actual->sig = NULL;// desenlazamos el nodo de la lista
            return actual;
        }

        anterior = actual; 
        actual = actual->sig;
    }

    return NULL; // No encontrado
}

struct Nodo *Fair_Share(struct Nodo **lista_listos,int grupos,int Base){
    struct Nodo *actual = *lista_listos;
    struct Nodo *anterior = NULL;

    CalculoPriodidad(lista_listos,grupos,Base);
      
    //iniciar en la cabeza de la lista
    int prioridad;
    int prioridad_A;
    while (actual != NULL){
        prioridad_A = actual->PRIORY;

        if(anterior == NULL){//significa que es el primer proceso de la lista y no hay con quien comparar todavia
            prioridad = prioridad_A;

        }else if(prioridad_A < prioridad){
            prioridad = prioridad_A;
        }

        anterior = actual; 
        actual = actual->sig;
    }
    return extraerNodo_Prioridad(lista_listos,prioridad);
}

//Para todo proceso de un grupo se le asigna el GCPU en caso de que se actualice
void GCPU_Global(struct Nodo **lista_listos, int GID, int GCPU){
    struct Nodo *actual = *lista_listos;

    while (actual != NULL) {
        if (actual->GID == GID) {
            actual->GCPU = GCPU;
        }
        actual = actual->sig;
    }
}

//Para saber cuantos grupos tenemos en caso de que usemos "mata" o mandemos un proceso a terminados 
int Busqueda_GID(struct Nodo **lista_listos,struct Nodo **lista_ejecucion, int GID){
    struct Nodo *actual = *lista_listos;
    int grupos_restantes = 0;

    while (actual != NULL) {
        if (actual->GID == GID) { //Encontro un proceso con el mismo GID
            grupos_restantes++;
        }
        actual = actual->sig;
    }

    actual = *lista_ejecucion; //se busca por si el unico proceso del grupo esta ejecutandose
    while (actual != NULL) {
        if (actual->GID == GID) {
            grupos_restantes++;
        }
        actual = actual->sig;
    }
    if(grupos_restantes == 0){//ya no hay procesos con ese GID
        return 0;
    }
    return 1;
}
