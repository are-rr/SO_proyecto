#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "procesos.h"

//Funciones para las listas:


void insertar(struct Nodo **cabeza, int pid,int gid,FILE *archivo,const char *nombre,char status, int pc) {
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo)); //reservar memoria para el nuevo nodo
    
    nuevo->PID = pid;
    nuevo ->GID = gid;
    strncpy(nuevo->nombrePro, nombre, sizeof(nuevo->nombrePro) - 1); //strncpy(destino,origen,tamañp)
    nuevo->nombrePro[sizeof(nuevo->nombrePro) - 1] = '\0';//se copia pues nombre es un dato termporal
    nuevo-> Archivo = archivo;
    nuevo->Status = status;
    nuevo->PC = pc;

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
        procesoError->Status = 'X';
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
        proceso_mata -> Status = 'Z';
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
            proceso_mata -> Status = 'Z';
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

int posicionarArchivoEnPC(FILE *copiaArchivo,int pc_buscar){
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
    nuevo->Archivo = fopen(original->nombrePro, "r");

    if (nuevo->Archivo == NULL) {
        free(nuevo);
        return NULL;
    }

    nuevo->EAX = original->EAX;
    nuevo->EBX = original->EBX;
    nuevo->ECX = original->ECX;
    nuevo->EDX = original->EDX;

    strcpy(nuevo->IR, "");//-----------------------------------------------------------

    if (nuevo_pc >= 0) {
        nuevo->PC = nuevo_pc;
    } else {
        nuevo->PC = original->PC;
    }

    // 👇 posicionar archivo en ese PC
    if (posicionarArchivoEnPC(nuevo->Archivo, nuevo->PC) == 0) {
        printf("Error: PC invalido\n");
        return NULL;
    }

    nuevo->Status = 'L';
    nuevo->sig = NULL;

    return nuevo;
}

struct Nodo *forkProcesoComando(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados,struct Nodo **lista_listos,int pid_comando,int pc,int nuevo_pid,int nuevo_gid) {
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

    nuevo = forkProceso(original, nuevo_pid,pc,nuevo_gid);

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
    mvprintw(y_mensajes, 0, "Proceso duplicado: PID %d desde PC %d", nuevo_pid, pc);
    refresh();

    return nuevo;
}