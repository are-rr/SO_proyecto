#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "procesos.h"
// recibe el nombre de un registro y un proceso
int *ObtenerRegistro(char *nombre, struct Nodo *p)
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

int ejecutarOperaciones(char *arg1, char *arg2, int contadorLinea, const char *linea_original, char tipoOp, struct Nodo *proceso)
{
    if (!filtro(arg1, arg2, contadorLinea, linea_original))
        return 0;
    if (!Comas_2pam(linea_original, contadorLinea))
        return 0;
    //*acceder al contenido apuntado
    // R1 = &proceso->EAX
    int *R1 = ObtenerRegistro(arg1, proceso);
    int valor = 0;

    if (Registro(arg2))
    {
        valor = *ObtenerRegistro(arg2, proceso);
    }
    else if (Digito(arg2))
    {
        valor = atoi(arg2); // convierte a tipo int
    }
    else
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "Segundo argumento invalido %s en linea %d:\"%s\"", arg2, contadorLinea, linea_original);
        refresh();
        return 0;
    }
    //'M'=MOV, 'A'=ADD, 'S'=SUB, 'U'=MUL, 'D'=DIV
    switch (tipoOp)
    {
        // proceso->EAX = valor
    case 'M':
        *R1 = valor;
        break;
    case 'A':
        *R1 += valor;
        break;
    case 'S':
        *R1 -= valor;
        break;
    case 'U':
        *R1 *= valor;
        break;
    case 'D':
        if (valor == 0)
        {
            limpiarZona(y_mensajes, 0, ancho_procesos);
            mvprintw(y_mensajes, 0, "ERROR: DIVISION POR CERO en linea %d:\"%s\"", contadorLinea, linea_original);
            refresh();
            return 0;
        }
        *R1 /= valor;
        break;
    }
    // mvprintw(y_header, 0, "%-10s %-18s %10s %10s %10s %10s %10s %10s", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "CPU","GCPU");
    limpiarZona(y_renglon, 0, ancho_procesos);
    mvprintw(y_renglon, 0, "%-10d %-18s %10d %10d %10d %10d %10d %10d", contadorLinea, linea_original, proceso->EAX, proceso->EBX, proceso->ECX, proceso->EDX, proceso->CPU, proceso->GCPU);
    refresh();
    return 1;
}

int INC_DEC(char *arg1, int contadorLinea, const char *linea_original, int incremento, struct Nodo *proceso)
{
    if (!filtroIncDecJnz(arg1, NULL, contadorLinea, linea_original))
        return 0;
    if (!Comas_1pam(linea_original, contadorLinea))
        return 0;
    if (!Registro(arg1))
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: NO es Registro %s en linea %d:\"%s\"", arg1, contadorLinea, linea_original);
        refresh();
        return 0;
    }

    int *R = ObtenerRegistro(arg1, proceso);
    *R += incremento;

    limpiarZona(y_renglon, 0, ancho_procesos);
    mvprintw(y_renglon, 0, "%-10d %-18s %10d %10d %10d %10d %10d %10d", contadorLinea, linea_original, proceso->EAX, proceso->EBX, proceso->ECX, proceso->EDX, proceso->CPU, proceso->GCPU);
    refresh();
    return 1;
}

int JNZ_(char *arg1, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    if (!filtroIncDecJnz(arg1, NULL, contadorLinea, linea_original))
        return 0;
    if (!Comas_1pam(linea_original, contadorLinea))
        return 0;

    long valor = 0; //NOTA: porque es un long el valor?
    if (!Digito(arg1))
    {
        limpiarZona(y_mensajes, 0, ancho_procesos);
        mvprintw(y_mensajes, 0, "ERROR: NO es Digito %s en linea %d:\"%s\"", arg1, contadorLinea, linea_original);
        refresh();
        return 0;
    }
    if (proceso->ECX != 0){
        valor = atoi(arg1); // valor de la pc
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
        mvprintw(y_mensajes, 0, "ERROR: ECX no es diferente de 0 para poder ejecutar linea %d:\"%s\"", contadorLinea, linea_original);
        refresh();
        return 0;
    }

    
    
}

int MOV(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return ejecutarOperaciones(arg1, arg2, contadorLinea, linea_original, 'M', proceso);
}
int ADD(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return ejecutarOperaciones(arg1, arg2, contadorLinea, linea_original, 'A', proceso);
}
int SUB(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return ejecutarOperaciones(arg1, arg2, contadorLinea, linea_original, 'S', proceso);
}
int MUL(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return ejecutarOperaciones(arg1, arg2, contadorLinea, linea_original, 'U', proceso);
}
int DIV(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return ejecutarOperaciones(arg1, arg2, contadorLinea, linea_original, 'D', proceso);
}
int INC(char *arg1, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return INC_DEC(arg1, contadorLinea, linea_original, 1, proceso);
} // positivo para que sume
int DEC(char *arg1, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return INC_DEC(arg1, contadorLinea, linea_original, -1, proceso);
} // argumento negativo para que decremente
int JNZ(char *arg1, int contadorLinea, const char *linea_original, struct Nodo *proceso)
{
    return JNZ_(arg1, contadorLinea, linea_original, proceso);
}