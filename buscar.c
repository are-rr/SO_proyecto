#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Nodo {
    int PID;
    char nombre[50];
    struct Nodo *sig;
};

struct Nodo* extraerNodo(struct Nodo **lista, int id) {
    struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL) {
        if (actual->PID == id) {
            if (anterior == NULL) { //si es el primer nodo en la lista, no tiene anteriro
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

void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso) {
    if (proceso == NULL) return;

    proceso->sig = NULL;

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

void moverNodo(struct Nodo **origen, struct Nodo **destino, int id) {
    struct Nodo *extraido = extraerNodo(origen, id);

    if (extraido != NULL) {
        insertarFinal(destino, extraido);
        printf("Nodo %d movido correctamente\n", id);
    } else {
        printf("Nodo %d no encontrado\n", id);
    }
}

int main() {
    struct Nodo *lista1 = NULL;
    struct Nodo *lista2 = NULL;

   //nodos para la listas
    struct Nodo *n1 = malloc(sizeof(struct Nodo));
    n1->PID = 1; strcpy(n1->nombre, "Proceso1");

    struct Nodo *n2 = malloc(sizeof(struct Nodo));
    n2->PID = 2; strcpy(n2->nombre, "Proceso2");

    struct Nodo *n3 = malloc(sizeof(struct Nodo));
    n3->PID = 3; strcpy(n3->nombre, "Proceso3");
//como enlazarlas
    n1->sig = n2;
    n2->sig = n3;
    n3->sig = NULL;

    lista1 = n1;

    // Mover nodo con ID = 2
    moverNodo(&lista1, &lista2, 2);

    return 0;
}