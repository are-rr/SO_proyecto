#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <ncurses.h>
#include <curses.h>
// DeEstructura del nodo
struct Nodo {
    int PID;       // identificador unico
    char Archivo[100];//nombre del archivo
    //Guardar el puntero al archivo FILE *, con eso ya no tendriamos que saltarnos los renglones 
    //comando???
    //valor de EAX,EBX,ECX,EDX
    int EAX;
    int EBX;
    int ECX;
    int EDX;
    char Status;     // L = listo E= ejecucion  T =terminado
    int PC;    // contador de programa(contadorLInea)
    struct Nodo *sig;  // puntero al siguiente nodo
};

// crea e Inserta el proceso al final de una lista
void insertar(struct Nodo **cabeza, int pid,const char *archivo,char status, int pc) {
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo)); //reservar memoria
    
    nuevo->PID = pid;
    strncpy(nuevo->Archivo, archivo, sizeof(nuevo->Archivo) - 1);
    nuevo->Archivo[sizeof(nuevo->Archivo) - 1] = '\0';
    //nuevo-> Archivo = archivo;
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

int buscar(struct Nodo lista, int PID){
    
}


// Imprimir una lista
void imprimir(struct Nodo *lista) {
    mvprintw(0, 0, "%-5s %-20s %8s %8s %8s %8s", "PC", "IR", "EAX", "EBX", "ECX", "EDX");
    refresh();
    int y_header2 = 6;
    int lineaProceso = 7;
    mvprintw(y_header2, 0, "%-5s %-20s %-15s %8s", "PID", "Nombre", "Status", "PC");
    refresh();
    while (lista != NULL) {
        mvprintw(lineaProceso,0,"%-5d %-20s %-15c %8d", lista->PID, lista->Archivo,lista->Status, lista->PC);
        refresh();
        lineaProceso++;
        lista = lista->sig;
    }
}

// Mostrar todas las listas
void imprimirEstado(struct Nodo *listo, struct Nodo *ejecucion, struct Nodo *terminados) {
    imprimir(listo);
    imprimir(ejecucion);
    imprimir(terminados);
}

void salirPrograma(){
    endwin();
    exit(0);
}
// cordenadas de fila
int y_header = 0;
int y_renglon = 1;
int y_mensajes = 3;
int y_linea_comando = 5;

int pid =0;

int main() {
    int ejecutando = 1;
    char comando[100];
    char archivo[100];
    int num_palabras;
    
    initscr();
    //comando[0] = '\0';
    //archivo[0] = '\0';

    struct Nodo *listo = NULL;
    struct Nodo *ejecucion = NULL;
    struct Nodo *terminados = NULL;

    while (ejecutando){

    comando[0] = '\0';
    archivo[0] = '\0';
        move(y_linea_comando, 0);
        clrtoeol();
        mvprintw(y_linea_comando, 0, "> ");

        if(listo != NULL){

        }

        if (comando[0] == '\0' || archivo[0] == '\0'){
            char entrada[200];
            char extra[100];
            move(y_linea_comando, 0); clrtoeol();refresh();
            mvprintw(y_linea_comando, 0, "> ");
            getnstr(entrada, 199);
            num_palabras = sscanf(entrada, "%99s %99s %99s", comando, archivo, extra); //sscanf(cadena, formato, &variable1, etc.);
        }

        if (strcmp(comando, "Salir") == 0){
            if (num_palabras > 1){
                move(y_mensajes, 0); clrtoeol();
                refresh();
                mvprintw(y_mensajes, 0, "ERROR: comando invalido");
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                num_palabras = 0;
                continue;
            }
            salirPrograma();
        }

        else if (strcmp(comando, "Ejecuta") == 0){
            if (num_palabras < 2) {
                move(y_mensajes, 0); clrtoeol();
                refresh();
                mvprintw(y_mensajes, 0, "ERROR: falta el nombre del archivo");
                comando[0] = '\0';
                archivo[0] = '\0';
                refresh();
                continue;
            }
            else if (num_palabras > 2){
                move(y_mensajes, 0); clrtoeol();
                refresh();
                mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                num_palabras = 0;
                continue;
            }

            FILE *file = fopen(archivo, "r");
            pid++;  //
            
            if (file == NULL){
                move(y_mensajes, 0); clrtoeol();
                mvprintw(y_mensajes, 0, "No se pudo abrir el archivo %s", archivo);
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                continue;
            }
                        insertar(&listo,pid,archivo,'L',0); //  Creamos proceso y encolamos a Listos

            imprimirEstado(listo, ejecucion, terminados);
        }

    }

    return 0;
}