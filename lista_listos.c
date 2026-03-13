#include <stdio.h>
#include <stdlib.h>

// DeEstructura del nodo
struct Nodo {
    int PID;       // identificador unico
    //nombre del archivo??
    //comando???
    char Status;     // L = listo E= ejecucion  T =terminado
    int PC;    // contador de programa(contadorLInea)
    struct Nodo *sig;  // puntero al siguiente nodo
};

// IINserta el proceso al final de una lista
void insertar(struct Nodo **cabeza, int pid, char status, int pc) {
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo)); //reservar memoria
    
    nuevo->PID = pid;
    nuevo->Status = status;
    nuevo->PC = pc;
    nuevo->sig = NULL;

    if (*cabeza == NULL) {
        *cabeza = nuevo; //si la lista esta vacia
        return;
    }

    struct Nodo *temp = *cabeza;
    while (temp->sig != NULL) { //noo esta vacia, recorre hasta el final
        temp = temp->sig;
    }
    temp->sig = nuevo;//inserta el nuevo nodo al final
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

// inserta el proceso al final
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

// Imprimir una lista
void imprimir(struct Nodo *lista) {
    while (lista != NULL) {
        printf("PID=%d, Status=%c, PC=%d-> ", lista->PID, lista->Status, lista->PC);
        lista = lista->sig;
    }
    printf("NULL\n");
}

// Mostrar todas las listas
void imprimirEstado(struct Nodo *listo, struct Nodo *ejecucion, struct Nodo *terminados) {
    printf("\n---------------------\n");
    printf("Listos: ");
    imprimir(listo);

    printf("Ejecucion: ");
    imprimir(ejecucion);

    printf("Terminados: ");
    imprimir(terminados);
}

// Simular un ciclo de CPU
void simular(struct Nodo **listo, struct Nodo **ejecucion, struct Nodo **terminados) {
    // Si no hay nada ejecutandose, tomar el primero de listos
    if (*ejecucion == NULL) {
        struct Nodo *proceso = extraerPrimero(listo);
        if (proceso != NULL) {
            proceso->Status = 'E'; //cambia a E porque esta en ejecucion
            insertarFinal(ejecucion, proceso);
        }
    }

    // Tomar el proceso en ejecucion
    struct Nodo *proceso = extraerPrimero(ejecucion);

    printf("\nEjecutando proceso PID %d\n", proceso->PID);

    // 3 instrucciones
    proceso->PC += 3; //aqui seria el Quantum(variable global)

    // Mandamos a terminados a las 10 instruc
    if (proceso->PC >= 10) {
        proceso->Status = 'T';
        insertarFinal(terminados, proceso);
        printf("Proceso PID %d terminado\n", proceso->PID);
    } else {
        // Si no termina, vuelve a listos al final
        proceso->Status = 'L';
        insertarFinal(listo, proceso);
        printf("Proceso PID:%d regresa a listos\n", proceso->PID);
    }
}


int main() {
    struct Nodo *listo = NULL;
    struct Nodo *ejecucion = NULL;
    struct Nodo *terminados = NULL;

    // Crear procesos iniciales
    insertar(&listo, 1, 'L', 0);
    insertar(&listo, 2, 'L', 0);
    //insertarFinal(&listo, 3, 'L', 0);

    imprimirEstado(listo, ejecucion, terminados);

    // Simular varios ciclos
    while (listo != NULL || ejecucion != NULL) {
        simular(&listo, &ejecucion, &terminados);
        imprimirEstado(listo, ejecucion, terminados);
    }

    return 0;
}