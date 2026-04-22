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

int buscar(struct Nodo **lista,int pid){
        struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL) {
        if (actual->PID == pid) {
            return actual;
        }
        anterior = actual; 
        actual = actual->sig;
    }
    return NULL; 
}

int fork(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, int pid_comando, int pc,int pid,int gid){
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo)); //reservar memoria para el nuevo nodo
    struct Nodo *original = (struct Nodo *)malloc(sizeof(struct Nodo));

    //busqueda del nodo
    if(original=buscar(&lista_ejecucion,pid_comando)){
        //operaciones para apuntar al pc que se necesita
        while(nuevo != NULL){
            if(pc >= 0){// asegura que no es negativo, si el pc no existe en el proceso que pasa??
                nuevo->PC = pc;
            }else{
                nuevo->PC = original->PC;
            }
        }
    }else if(original=buscar(&lista_listos,pid_comando)){
        while(nuevo != NULL){
            if(pc >= 0){
                nuevo->PC = pc;
            }else{
                nuevo->PC = original->PC;
            }
        }

    }else if(original=buscar(&lista_terminados,pid_comando)){
        move(y_mensajes, 0); clrtoeol();
        mvprintw(y_mensajes,0,"ERROR: no se puede duplicar un proceso que esta en terminados");
        return NULL;//ya que regresa un nodo
    }

    //una vez ubicado extraer los valores
    //Revisar como funciona los punteros para guardar el contexto
    //meter a listos
    nuevo->PID = pid;
    nuevo ->GID = gid;
    strncpy(nuevo->nombrePro, original->nombrePro, sizeof(nuevo->nombrePro) - 1); //strncpy(destino,origen,tamañp)
    nuevo->nombrePro[sizeof(nuevo->nombrePro) - 1] = '\0';//se copia pues nombre es un dato termporal
    //nuevo-> Archivo = archivo;//--------------------------------------------------
    nuevo-> Archivo = original ->Archivo;
    nuevo->Status = 'L';

    nuevo->EAX = 0;
    nuevo->EBX = 0;
    nuevo->ECX = 0;
    nuevo->EDX = 0;
    //nuevo->IR[0] = '\0';

    nuevo->sig = NULL;

}