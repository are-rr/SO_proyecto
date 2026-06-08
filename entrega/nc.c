#include <ncurses.h>
#include <string.h>
#include "procesos.h"

void imprimirProceso(struct Nodo *p,int y_ncurse,const char* cadena){
    //mvprintw(y_header2, 0, "%-5s %-5s %-18s %-18s %-10s %-18s %10s %10s %10s %10s %-8s %-8s", "PID","GID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX", "CPU","GCPU");
    
    mvprintw(y_ncurse,0,"%-5d %-5d %-8d %-8d %-18s %-18s %-10d %-18s %10d %10d %10d %10d %8d",
        p->PID,p->GID,p->CPU,p->GCPU,p->nombrePro,cadena,p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX,p->PRIORY);
}
// Imprimir una lista
void imprimirlista(struct Nodo *lista, int y_ncurses, int status) {
    char cadena[20];
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

//imprimir la TMP
void imprimir_TMP(int TMP[][3], int y_tablaR, int pid){
    int y = y_tablaR;
    move(12, 0);
    clrtoeol();
    mvprintw(12,0,"Proceso: %d",pid);

    for(int i = 0; i < 10; i++){
        //if(TMP[i][2] != -1){
            move(y, 0);
            clrtoeol();
            mvprintw(y, 0, "%-5d %5d %5d %5d", i, TMP[i][0], TMP[i][1], TMP[i][2]);
            y++;
        //}
    }
    refresh();
}