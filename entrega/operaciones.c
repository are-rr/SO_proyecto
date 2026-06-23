#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ncurses.h>

#include "procesos.h"
// recibe el nombre de un registro y un proceso
int *ObtenerRegistro(struct Nodo *p, char *nombre)
{
    if (strcmp(nombre, "EAX") == 0)
    {
        return &p->EAX; // aqui devuelve donde esta guardado el registo
    }
    else if (strcmp(nombre, "EBX") == 0)
    {
        return &p->EBX; // la direccion de memoria donde esta guardado EBX
    } //& (operador para obtener la direccion de memoria de una variable)
    else if (strcmp(nombre, "ECX") == 0)
    {
        return &p->ECX;
    }
    else if (strcmp(nombre, "EDX") == 0)
    {
        return &p->EDX;
    }
    return NULL;
} // devuleve la direccion del registro correspondiente dentro del proceso

int ejecutarOperaciones(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea, char tipoOp)
{
    if (!filtro(arg1, arg2, linea_original, contadorLinea))
        return 0;
    if (!Comas_2pam(linea_original, contadorLinea))
        return 0;
    //*acceder al contenido apuntado
    // R1 = &proceso->EAX
    int *R1 = ObtenerRegistro(proceso, arg1);
    int valor = 0;

    if (Registro(arg2)){
        valor = *ObtenerRegistro(proceso, arg2);
    }else{
        if (!valivarLimitInt(arg2, &valor)){
            limpiarZona(y_mensajes, 0, ancho_procesos);
            mvprintw(y_mensajes, 0, "ERROR: numero invalido o fuera de rango %s en linea %d:\"%s\"", arg2, contadorLinea, linea_original);
            refresh();
            return 0;
        }
    }
    //'M'=MOV, 'A'=ADD, 'S'=SUB, 'U'=MUL, 'D'=DIV
    long long resultado = *R1;

    switch (tipoOp)
    {
    case 'M':resultado = valor; break;
    case 'A': resultado = (long long)(*R1) + valor; break;
    case 'S': resultado = (long long)(*R1) - valor; break;
    case 'U': resultado = (long long)(*R1) * valor;break;
    case 'D':
        if (valor == 0){
            limpiarZona(y_mensajes, 0, ancho_procesos);
            mvprintw(y_mensajes, 0, "ERROR: DIVISION POR CERO en linea %d:\"%s\"", contadorLinea, linea_original);
            refresh();
            return 0;
        }
        resultado = (long long)(*R1) / valor;
        break;
    }
    if (resultado > INT_MAX || resultado < INT_MIN){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: Overflow en operacion linea %d:\"%s\"", contadorLinea, linea_original);
        refresh();
        return 0;
    }

    *R1 = (int)resultado;
    // mvprintw(y_header, 0, "%-10s %-18s %10s %10s %10s %10s %10s %10s", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "CPU","GCPU");
    limpiarZona(y_renglon, 0, ancho_procesos);
    mvprintw(y_renglon, 0, "%-10d %-18s %10d %10d %10d %10d %10d %10d", contadorLinea, linea_original, proceso->EAX, proceso->EBX, proceso->ECX, proceso->EDX, proceso->CPU, proceso->GCPU);
    refresh();
    return 1;
}

int INC_DEC(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea, int incremento)
{
    if (!filtroIncDecJnz(arg1, NULL, linea_original, contadorLinea))
        return 0;
    if (!Comas_1pam(linea_original, contadorLinea))
        return 0;
    if (!Registro(arg1)){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: NO es Registro %s en linea %d:\"%s\"", arg1, contadorLinea, linea_original);
        refresh();
        return 0;
    }
    int *R = ObtenerRegistro(proceso, arg1);
    long long resultado = (long long)(*R) + incremento;

    if (resultado > INT_MAX || resultado < INT_MIN) {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: Overflow linea %d:\"%s\"", contadorLinea, linea_original);
        refresh();
        return 0;
    }
    *R = (int)resultado;

    limpiarZona(y_renglon, 0, ancho_procesos);
    mvprintw(y_renglon, 0, "%-10d %-18s %10d %10d %10d %10d %10d %10d", contadorLinea, linea_original, proceso->EAX, proceso->EBX, proceso->ECX, proceso->EDX, proceso->CPU, proceso->GCPU);
    refresh();
    return 1;
}

int JNZ_(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea)
{
    if (!filtroIncDecJnz(arg1, NULL, linea_original, contadorLinea))
    {
        return 0;
    }
    if (!Comas_1pam(linea_original, contadorLinea)){
        return 0;
    }
    int valor = 0;

    if (!valivarLimitInt(arg1, &valor)){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC invalido o fuera de rango %s en linea %d:\"%s\"", arg1, contadorLinea, linea_original);
        refresh();
        return 0;
    }
    if (valor < 0){
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: PC no puede ser negativo en linea %d:\"%s\"", contadorLinea, linea_original);
        refresh();
        return 0;
    }
    if (proceso->ECX != 0){
        // que JNZ no se pase de las lineas que tiene un proceso
        if (proceso->num_lineas > valor){
            proceso->PC = valor;
            limpiarZona(y_renglon, 0, ancho_procesos);
            mvprintw(y_renglon, 0, "%-10d %-18s %10d %10d %10d %10d %10d %10d", contadorLinea, linea_original, proceso->EAX, proceso->EBX, proceso->ECX, proceso->EDX, proceso->CPU, proceso->GCPU);
            refresh();
            return 1;
        }
        else{
            limpiarZona(y_mensajes, 0, ancho_procesos);
            mvprintw(y_mensajes, 0, "ERROR: Fuera de rango en la linea %d:\"%s\"", contadorLinea, linea_original);
            refresh();
            return 0;
        }
    }
    else{
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ECX no es diferente de 0 para poder ejecutar linea %d:\"%s\"", contadorLinea, linea_original);
        refresh();
        return 2;
    }
}

int MOV(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea){
    return ejecutarOperaciones(proceso, arg1, arg2, linea_original, contadorLinea, 'M');
}
int ADD(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea){
    return ejecutarOperaciones(proceso, arg1, arg2, linea_original, contadorLinea, 'A');
}
int SUB(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea){
    return ejecutarOperaciones(proceso, arg1, arg2, linea_original, contadorLinea, 'S');
}
int MUL(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea){
    return ejecutarOperaciones(proceso, arg1, arg2, linea_original, contadorLinea, 'U');
}
int DIV(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea){
    return ejecutarOperaciones(proceso, arg1, arg2, linea_original, contadorLinea, 'D');
}
int INC(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea){
    return INC_DEC(proceso, arg1, linea_original, contadorLinea, 1);
} // positivo para que sume
int DEC(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea){
    return INC_DEC(proceso, arg1, linea_original, contadorLinea, -1);
} // argumento negativo para que decremente
int JNZ(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea){
    return JNZ_(proceso, arg1, linea_original, contadorLinea);
}