#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "procesos.h"

int *ObtenerRegistro(char *nombre, struct Nodo *p){
    if (strcmp(nombre, "EAX") == 0){
        return &p->EAX;
    }
    else if (strcmp(nombre, "EBX") == 0){
        return &p->EBX;
    }
    else if (strcmp(nombre, "ECX") == 0){
        return &p->ECX;
    }
    else if (strcmp(nombre, "EDX") == 0){
        return &p->EDX;
    }
    return NULL;
}

int ejecutarOperaciones(char *arg1, char *arg2, int contadorLinea, const char *linea_original,char tipoOp, struct Nodo *proceso) { 
    if (!filtro(arg1, arg2, contadorLinea, linea_original)) return 0;
    if (!Comas_2pam(linea_original, contadorLinea)) return 0;

    int *R1 = ObtenerRegistro(arg1,proceso);
    int valor = 0;

    if (Registro(arg2)) {
        valor = *ObtenerRegistro(arg2,proceso);
    }
    else if (Digito(arg2)) {
        valor = atoi(arg2);//convierte a tipo int
    }
    else {
        move(y_mensajes,0); clrtoeol(); refresh();
        mvprintw(y_mensajes,0,"Segundo argumento invalido %s en linea %d:\"%s\"", arg2, contadorLinea, linea_original);
        return 0;
    }
    //'M'=MOV, 'A'=ADD, 'S'=SUB, 'U'=MUL, 'D'=DIV
    switch(tipoOp){
        case 'M': *R1 = valor; break;
        case 'A': *R1 += valor; break;
        case 'S': *R1 -= valor; break;
        case 'U': *R1 *= valor; break;
        case 'D': 
            if (valor == 0){
                move(y_mensajes,0); clrtoeol(); refresh();
                mvprintw(y_mensajes,0,"ERROR: DIVISION POR CERO en linea %d:\"%s\"", contadorLinea, linea_original);
                return 0;
            }
            *R1 /= valor;
            break;
    }

    move(y_renglon,0); clrtoeol(); refresh();
    mvprintw(y_renglon,0,"%-10d %-20s %10d %10d %10d %10d", contadorLinea, linea_original, proceso->EAX, proceso->EBX, proceso->ECX, proceso->EDX);

    return 1;
}

int INC_DEC(char *arg1,char *arg2, int contadorLinea, const char *linea_original, int incremento,struct Nodo *proceso){
    if (!filtroIncDec(arg1, NULL, contadorLinea, linea_original)) return 0;
    if (!Comas_1pam(linea_original, contadorLinea)) return 0;
    if (!Registro(arg1)){
        move(y_mensajes,0); clrtoeol(); refresh();
        mvprintw(y_mensajes,0,"ERROR: NO es Registro %s en linea %d:\"%s\"", arg1, contadorLinea, linea_original);
        return 0;
    }

    int *R = ObtenerRegistro(arg1,proceso);
    *R += incremento;

    move(y_renglon,0); clrtoeol(); refresh();
    mvprintw(y_renglon,0,"%-5d %-20s %8d %8d %8d %8d", contadorLinea, linea_original,proceso->EAX, proceso->EBX,proceso-> ECX,proceso-> EDX);

    return 1;
}

int MOV(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'M',proceso); }
int ADD(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'A',proceso); }
int SUB(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'S',proceso); }
int MUL(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'U',proceso); }
int DIV(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'D',proceso); }
int INC(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso)
{ return INC_DEC(arg1,arg2,contadorLinea,linea_original,1,proceso); } //positivo para que sume
int DEC(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso)
{ return INC_DEC(arg1,arg2,contadorLinea,linea_original,-1,proceso); } //argumento negativo para que decremente
