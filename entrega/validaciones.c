#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <ncurses.h>
#include <sys/select.h>
#include <unistd.h>

#include "procesos.h"

int Registro(char *token){
    char Registros[4][10] = {"EAX", "EBX", "ECX", "EDX"}; 
    for (int i = 0; i < 4; i++){
        if (strcmp(token, Registros[i]) == 0)
        {
            return 1; // si encontro el registro en el arreglo
        }
    }
    return 0;
}

int Operaciones(char *token, const char *linea_original, int contadorLinea)
{
    char Instrucciones[9][10] = {"MOV", "ADD", "SUB", "MUL", "DIV", "INC", "DEC", "END", "JNZ"};
    for (int i = 0; i < 9; i++)
    {
        if (strcmp(token, Instrucciones[i]) == 0)
        {
            return 1;
        }
    }
    move(y_mensajes, 0);
    clrtoeol();
    refresh();
    mvprintw(y_mensajes, 0, "ERROR: Instruccion no reconocida en linea %d:\"%s\"", contadorLinea, linea_original);
    return 0;
}

int Digito(char *token){
    int i = 0;
    if (token[0] == '\0') // esta vacia
        return 0;
    if (token[0] == '-'){ // empieza con -
        i = 1;
    } // ya se inicializo antes, no se inicializa
    for (; token[i] != '\0' && token[i] != '\n'; i++){ // mientras que no llegue al final del texto
        if (!isdigit(token[i])){ // si es digito de <ctype.h>.
            return 0;
        }
    }
    return 1;
}

int valivarLimitInt(char *token, int *resultado){
    long valor;
    if (!Digito(token)){
        return 0;
    }
    valor = strtol(token, NULL, 10);

    if (valor > INT_MAX || valor < INT_MIN){
        return 0;
    }
    *resultado = (int)valor;
    return 1;
}

int filtro(char *arg1, char *arg2, const char *linea_original, int contadorLinea)
{
    if (arg1 == NULL || arg2 == NULL){
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR: Faltan argumentos en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    if (!Registro(arg1)){
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR: El primer argumento debe ser un registro valido en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int filtroIncDecJnz(char *arg1, char *arg2, const char *linea_original, int contadorLinea)
{
    if (arg1 == NULL)
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR: No hay argumento en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (arg2 != NULL)
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR INC/DEC solo debe tener un argumento en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int Comas_2pam(const char *linea_original, int contadorLinea)
{
    int contadorComa = 0;

    for (int i = 0; linea_original[i] != '\0'; i++)
    {
        if (linea_original[i] == ',')
            contadorComa++;
    }
    const char *coma = strchr(linea_original, ',');

    if (contadorComa > 1)
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR mas de una coma en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (contadorComa == 0)
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR falta la coma en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (*(coma - 1) == ' ')
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR espacio antes de coma linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (*(coma + 1) == ' ')
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR espacio despues de coma linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int Comas_1pam(const char *linea_original, int contadorLinea)
{
    int contadorComa = 0;

    for (int i = 0; linea_original[i] != '\0'; i++)
    {
        if (linea_original[i] == ',')
            contadorComa++;
    }

    if (contadorComa >= 1)
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR: No debe tener coma en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int validarEspacios(const char *linea_original, char *instruccion, int contadorLinea)
{
    int i = 0;
    if (!Operaciones(instruccion, linea_original, contadorLinea))
    {
        return 0;
    }

    // Aqui es para mostrar error si encuentra espacios y tabulaciones al inicio de la instruccion
    if (linea_original[0] == ' ' || linea_original[0] == '\t')
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\", no debe iniciar con espacios/tabs", contadorLinea, linea_original);
        return 0;
    }
    i += 3; // por que las instrucciones tienen 3 letras
    // En caso de END
    if (strcmp(instruccion, "END") == 0)
    {
        if (linea_original[i] != '\0')
        {
            move(y_mensajes, 0);
            clrtoeol();
            refresh();
            mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d: %s, no se permiten espacios al final", contadorLinea, instruccion);
            return 0;
        }
        return 1;
    }

    if (linea_original[i] != ' ')
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d: \"%s\" debe haber 1 espacio despues de %s", contadorLinea, linea_original, instruccion);
        return 0;
    }
    if (linea_original[i + 1] == ' ')
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\" hay mas de 1 espacio despues de %s", contadorLinea, linea_original, instruccion);
        return 0;
    }
    i++; // validar espacio
    // arg1
    while (linea_original[i] && linea_original[i] != ',' && linea_original[i] != ' ')
    {
        i++;
    }
    // en caso de INC y DEC
    if (strcmp(instruccion, "INC") == 0 || strcmp(instruccion, "DEC") == 0)
    {
        if (linea_original[i] != '\0')
        {
            move(y_mensajes, 0);
            clrtoeol();
            refresh();
            mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\" %s solo lleva 1 argumento y sin espacios extra", contadorLinea, linea_original, instruccion);
            return 0;
        }
        return 1;
    }
    i++; // para saltar la coma
   
    int contadorEspacio = 0;

    for (int i = 0; linea_original[i] != '\0'; i++)
    {
        if (linea_original[i] == ' ')
            contadorEspacio++;
    }
    if (contadorEspacio > 1)
    {
        move(y_mensajes, 0);
        clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }

    for (int j = 0; linea_original[j] != '\0'; j++)
    {
        if (linea_original[j] == ' ' && linea_original[j + 1] == '\0')
        {
            move(y_mensajes, 0);
            clrtoeol();
            refresh();
            mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\", no se permiten espacios al final", contadorLinea, linea_original);
            return 0;
        }
    }

    return 1;
}

int Negativo(char *numero)
{
    if (numero[0] == '-')
    {
        return 1;
    }
    return 0;
}