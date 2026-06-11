#include <ncurses.h>
#include <string.h>
#include "procesos.h"

WINDOW *win_izq;
WINDOW *win_der;
WINDOW *win_abajo;

void Interfaz() {
    int alto, ancho;
    getmaxyx(stdscr, alto, ancho); //obtinee medidas de ancho y alto de la terminal

    int alto_abajo = 4; 
    int alto_principal = alto - alto_abajo; //para ventana derecha quitando la ventana de abajo
    int ancho_div = ancho /2;

            //newwin(alto,ancho,coordenada y, coordenada x);
    win_izq = newwin(alto_principal, ancho_div, 0, 0); 
    win_der = newwin(alto_principal, ancho_div, 0, ancho_div);
    win_abajo = newwin(alto_abajo, ancho, alto_principal, 0);

    //box(win_izq, 0, 0);
    //box(win_der, 0, 0);
    //box(win_abajo, 0, 0);

    wrefresh(win_izq);
    wrefresh(win_der);
    wrefresh(win_abajo);
}

void refrescarVentanas() {
    wrefresh(win_izq);
    wrefresh(win_der);
    wrefresh(win_abajo);
}

void cerrarInterfaz() {
    delwin(win_izq);
    delwin(win_der);
    delwin(win_abajo);
}

void imprimirProceso(struct Nodo *p,int y_ncurse,const char* cadena){
    //mvprintw(y_header2, 0, "%-5s %-5s %-18s %-18s %-10s %-18s %10s %10s %10s %10s %-8s %-8s", "PID","GID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX", "CPU","GCPU");
    
    mvprintw(y_ncurse,0,"%-5d %-5d %-8d %-8d %-18s %-18s %-10d %-18s %10d %10d %10d %10d %8d",
        p->PID,p->GID,p->CPU,p->GCPU,p->nombrePro,cadena,p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX,p->PRIORY);
}
// Imprimir una lista
void imprimirlista(struct Nodo *lista, int y_ncurses, int status) {
    char cadena[20];
    /*if(status == 2){
        strcpy(cadena,"Listos");   
    }else if(status == 1){
        strcpy(cadena,"Ejecucion");
    }else if(lista->Status == 4){
    //}else if( status == 3 ){
        strcpy(cadena,"Terminados");
    }else if( status == 3 ){
    //}else if(lista->Status == 4){
        strcpy(cadena,"Terminados-Error");
    }*/
    switch(status){
        case 2 : strcpy(cadena,"Listos"); break;
        case 1 : strcpy(cadena,"Ejecucion"); break;
        case 3 : strcpy(cadena,"Terminados"); break;
        //NOTA: Agregar los if para los otros casos de terminados (mata y error)
    }
    while (lista != NULL) {  
            imprimirProceso(lista,y_ncurses,cadena);
            lista = lista->sig;
            y_ncurses++;
    }
}

void imprimirEstado(struct Nodo *listos,struct Nodo *ejecucion,struct Nodo *terminados) {
    int y_procesos = y_procesoEjecucion;
    imprimirlista(ejecucion, y_procesoEjecucion,1);
    y_procesos += contarNodos(ejecucion);
    imprimirlista(listos, y_procesos,2);
    y_procesos += contarNodos(listos);
    imprimirlista(terminados, y_procesos,3);
    y_procesos += contarNodos(terminados);
    refresh();
}