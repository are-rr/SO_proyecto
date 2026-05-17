#include <ncurses.h>

#include "procesos.h"

const char *statusTexto(char status){
    switch(status){
        case 'L' : return "Listos";
        case 'E' : return "Ejecucion";
        case 'T' : return "Terminados";
        case 'X' : return "Terminados-Error";
        case 'Z' : return "Terminados-Mata";
        default: return "Desconocido"; //EN caso de que no sean los anteriores
    }
}

void imprimirProceso(struct Nodo *p,int y_ncurse){
    //mvprintw(y_header2, 0, "%-5s %-5s %-18s %-18s %-10s %-18s %10s %10s %10s %10s %-8s %-8s", "PID","GID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX", "CPU","GCPU");
    if(p->Status == 'E'){
        mvprintw(y_ncurse,0,"%-5d %-5d %-8d %-8d %-18s %-18s %-10d %-18s %10d %10d %10d %10d %8d",
            p->PID,p->GID,p->CPU,p->GCPU,p->nombrePro,statusTexto(p->Status),p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX,p->PRIORY);
    }
    else if(p->Status == 'L'){
        mvprintw(y_ncurse,0,"%-5d %-5d %-8d %-8d %-18s %-18s %-10d %-18s %10d %10d %10d %10d %8d",
            p->PID,p->GID,p->CPU,p->GCPU,p->nombrePro,statusTexto(p->Status),p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX,p->PRIORY);
    }
    else if(p->Status == 'T' || p-> Status == 'X' || p-> Status == 'Z'){
        mvprintw(y_ncurse,0,"%-5d %-5d %-8d %-8d %-18s %-18s %-10d %-18s %10d %10d %10d %10d %8d",
            p->PID,p->GID,p->CPU,p->GCPU,p->nombrePro,statusTexto(p->Status),p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX,p->PRIORY);
    }
}

// Imprimir una lista
void imprimirlista(struct Nodo *lista, int y_ncurses) {
    while (lista != NULL) {  
            imprimirProceso(lista,y_ncurses);
            lista = lista->sig;
            y_ncurses++;
    }
}

void imprimirEstado(struct Nodo *listos,struct Nodo *ejecucion,struct Nodo *terminados) {
    int y_procesos = y_procesoEjecucion;
    imprimirlista(ejecucion, y_procesoEjecucion);
    y_procesos += contarNodos(ejecucion);
    imprimirlista(listos, y_procesos);
    y_procesos += contarNodos(listos);
    imprimirlista(terminados, y_procesos);
    y_procesos += contarNodos(terminados);
    refresh();
}