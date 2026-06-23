#include <ncurses.h>
#include <string.h>
#include "procesos.h"

void imprimirProceso(struct Nodo *p, int y_ncurse, const char *cadena)
{
    // mvprintw(y_header2, 0, "%-5s %-5s %-18s %-18s %-10s %-18s %10s %10s %10s %10s %-8s %-8s", "PID","GID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX", "CPU","GCPU");
    limpiarZona(y_ncurse, 0, ancho_procesos);
    mvprintw(y_ncurse, 0, "%-5d %-5d %-8d %-8d %-18s %-18s %-10d %-18s %10d %10d %10d %10d %8d",
             p->PID, p->GID, p->CPU, p->GCPU, p->nombrePro, cadena, p->PC, p->IR, p->EAX, p->EBX, p->ECX, p->EDX, p->PRIORY);
}
// Imprimir una lista
void imprimirlista(struct Nodo *lista, int y_ncurses, int status)
{
    char cadena[20];
    switch (status)
    {
    case 2:
        strcpy(cadena, "Listos");
        break;
    case 1:
        strcpy(cadena, "Ejecucion");
        break;
    case 3:
        strcpy(cadena, "Terminados");
        break;
    case 4:
        strcpy(cadena, "Suspendidos");
        break;
    case 5:
        strcpy(cadena,"Nuevos");
        break;
    }
    while (lista != NULL)
    {
        imprimirProceso(lista, y_ncurses, cadena);
        lista = lista->sig;
        y_ncurses++;
    }
}

void imprimirEstado(struct Nodo *listos, struct Nodo *ejecucion, struct Nodo *terminados, struct Nodo *suspendidos, struct Nodo *nuevos)
{
    int y_procesos = y_procesoEjecucion;
    imprimirlista(ejecucion, y_procesoEjecucion, 1);
    y_procesos += contarNodos(ejecucion);
    imprimirlista(listos, y_procesos, 2);
    y_procesos += contarNodos(listos);
    imprimirlista(terminados, y_procesos, 3);
    y_procesos += contarNodos(terminados);
    imprimirlista(suspendidos, y_procesos, 4);
    y_procesos += contarNodos(suspendidos);
    imprimirlista(nuevos, y_procesos, 5);
    y_procesos += contarNodos(nuevos);
    refresh();
}

// imprimir la TMP
void imprimir_TMP(int TMP[][3], int y_renglon_TMP, int x_TMP,int paginas,int pid){
    int y = y_renglon_TMP;
    int renglones = 30;
    limpiarZona(0, x_TMP, ancho_tablas);
    mvprintw(0, x_TMP, "TMP de proceso: %d", pid);
    if(paginas < renglones){
        renglones = paginas;
    }
    for (int i = 0; i < renglones; i++){
        limpiarZona(y, x_TMP, ancho_tablas);
        mvprintw(y, x_TMP, "%-5d %5d %5d %5d", i, TMP[i][0], TMP[i][1], TMP[i][2]);
        y++;
    }
    refresh();
}

void imprimir_TMS(int TMS[], int y_renglon_TMS, int x_TMS){
    int y = y_renglon_TMS;
    int renglones = 28;
    limpiarZona(32, x_TMS, ancho_tablas);
    mvprintw(32, x_TMS, "TMS");
    
    for (int i = 0; i < renglones; i++){
        limpiarZona(y, x_TMS, ancho_tablas);
        mvprintw(y, x_TMS, "%-5d %5d", i, TMS[i]);
        y++;
    }
    refresh();
}
void imprimir_TMM(int TMM[][2], int y_renglon_TMM, int x_TMM){
    int y = y_renglon_TMM;
    int renglones = 16;
    limpiarZona(0, x_TMM, ancho_tablas);
    mvprintw(0, x_TMM, "TMM");

    for (int i = 0; i < renglones; i++){
        limpiarZona(y, x_TMM, ancho_tablas);
        mvprintw(y, x_TMM, "%-5d %5d %5d", i, TMM[i][0], TMM[i][1]);
        y++;
    }
    refresh();
}
void limpiarZona(int y, int x, int ancho){
    move(y, x);
    for (int i = 0; i < ancho; i++){
        addch(' '); //imprime caracter en la posicion del cursor
    }
}

void limpiarZonaTabla(int renglon, int x){
    for (int i = 0; i < 30; i++){
        limpiarZona(renglon + i, x, ancho_tablas);
    }
}
