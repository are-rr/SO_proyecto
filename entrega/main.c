#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "procesos.h"
#include <time.h>

// cordenadas de fila
int ancho_procesos = 155;
int y_header = 0;
int y_renglon = 1;
int y_mensajes = 3;
int y_linea_comando = 5;
int y_header2 = 7;
int y_procesoEjecucion = 8;

int ancho_TMP = 35;
int y_header_TMP = 1;
int y_renglon_TMP = 2;
int x_TMP = 160;

int ancho_TMM = 35;
int y_header_TMM = 1;
int y_renglon_TMM = 2;
int x_TMM = 195;

int ancho_TMS = 35;
int y_header_TMS = 33;
int y_renglon_TMS = 34;
int x_TMS = 160;

int ejecutando = 1;
int pid = 0;
int gid = 0;

int Base = 60;
int grupos = 0; // para contar cuantos grupos tenemos
int num_lineas = 0;
int paginas;
int ms = 1000;
int porS = 0;
int porR = 0;

int main()
{
    char comando[100];
    char archivo[100];
    int num_PID; // pid que brindo en el comando
    int num_PC;
    int num_palabras;
    initscr();

    comando[0] = '\0';
    archivo[0] = '\0';
    struct Nodo *lista_listos = NULL;
    struct Nodo *lista_ejecucion = NULL;
    struct Nodo *lista_terminados = NULL;
    struct Nodo *lista_nuevos = NULL;
    struct Nodo *lista_suspendidos = NULL;
    int huboError = 0;
    int TMS[32768];
    in_TMS(TMS);
    int TMM[16][2];
    in_TMM(TMM);
    char RAM[16][400];

    char *ArchivoBinario = "archivoBinario.bin";
    if (Crear_ArchivoBinario(ArchivoBinario, 100) == 0){
        //  mvprintw(y_mensajes, 0, "Se creo correctamente el Archivo Binario");
        refresh();
    } // NOTA:no estoy seguro si ese 100 puede ir asi, pero es el tamaño de char que tenemos para el IR

    FILE *swap = fopen(ArchivoBinario, "r+b");
    while (ejecutando){ /////////////////////////////////////////////////
        huboError = 0; // reiniciamos a cada interacion la bandera de errores
       
        if ((lista_ejecucion == NULL && lista_listos == NULL)){
            char entrada[200];
            char extra[100];
            char extra2[100];
            comando[0] = '\0';
            archivo[0] = '\0';
            limpiarZona(y_linea_comando, 0, ancho_procesos);
            mvprintw(y_linea_comando, 0, "> ");
            refresh();
            
            if (lista_suspendidos != NULL){
                RevisarSuspendidos(&lista_suspendidos, &lista_listos);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                move(y_linea_comando, 2);
                refresh();

                if (!kbhit()){
                    continue;
                }
            }
            getnstr(entrada, 199);                                                                  // lee la entrada
            num_palabras = sscanf(entrada, "%99s %99s %99s %99s", comando, archivo, extra, extra2); // sscanf(cadena, formato, &variable1, etc.);
            if (num_palabras <= 0){

                continue;
            }
            
            limpiarZona(y_linea_comando, 0, ancho_procesos);
            refresh();
            if (strcmp(comando, "salir") == 0){
                if (num_palabras > 1){
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: comando invalido");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    num_palabras = 0;
                    continue;
                }
                salirPrograma();
            }
            else if (strcmp(comando, "ejecuta") == 0)
            {
                if (num_palabras < 2)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: falta el nombre del archivo");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    continue;
                }
                else if (num_palabras > 2)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    num_palabras = 0;
                    continue;
                }

                pid++;
                gid++;
                grupos++;
                // primero pasar a nuevos
                // NOTA: tenemos que arreglar que cuando escribes bien un proceso pero el archivo no existe, lo mete a nuevos(CREO QUE YA ESTA, LIBERE CON FREE EL NODO)
                if(validarArchivo(archivo)==1){
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "No se pudo abrir el archivo %s", archivo);
                    refresh();
                    pid--;
                    gid--;
                    reiniciarVariables(comando, archivo);
                    num_palabras = 0;
                    continue;
                }
                num_lineas = ContadorLineas(archivo);
                paginas = (num_lineas + 3) / 4; // despues de aqui se puede obtener las paginas para la TMP

                insertar(&lista_nuevos, pid, gid, archivo, 0, paginas);

                struct Nodo *nuevo = buscar(lista_nuevos, pid);

                nuevo->num_paginas = paginas;
                nuevo->TMP = malloc(paginas * sizeof(int[3]));

                if (nuevo->TMP == NULL)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: no se pudo crear TMP");
                    refresh();
                    continue;
                }

                in_TMP(nuevo->TMP, nuevo->num_paginas);
                int marcos_libres = marcosLS(TMS);
                if (paginas < 32768)
                {
                    // ver si se peude cargar a swap
                    if(nuevo->num_paginas <= marcos_libres){
                        int result_reescritura = reescritura(archivo, swap, pid, TMS, nuevo->TMP); // NOTA: Puede     que aqui este el error
                        limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                        imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                        porcentajes(TMS, TMM, &porS, &porR);
                        if (result_reescritura == 1)
                        { // no cupo
                            mvprintw(y_mensajes, 0, "Proceso %d queda en nuevos: no hay espacio en swap", pid);
                            reiniciarVariables(comando, archivo);
                            refresh();
                            continue;
                        }
                        else if (result_reescritura == 0)
                        {                                                     // Si es exite y cabe en swap pasamos a Listos
                            struct Nodo *p = extraerNodo(&lista_nuevos, pid); // pasamos a listos si todo bien
                            if (p != NULL)
                            {
                                insertarFinal(&lista_listos, p);
                            }
                        }
                        imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    }
                }else{
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: El proceso es mas grande que el swap.");
                    reiniciarVariables(comando, archivo);
                    refresh();
                    continue;
                }
            }
            else if (strcmp(comando, "mata") == 0)
            {
                if (lista_listos == NULL && lista_ejecucion == NULL && lista_terminados == NULL)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: no hay procesos que matar");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    num_palabras = 0;
                    continue;
                }
            }
            else if (strcmp(comando, "fork") == 0)
            {
                if (lista_listos == NULL && lista_ejecucion == NULL && lista_terminados == NULL)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: no hay procesos para duplicar");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    num_palabras = 0;
                    continue;
                }
            }
            else if (strcmp(comando, "velocidad") == 0)
            {
                if (num_palabras < 2)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: falta los milisegundos");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    comando[0] = '\0';
                    num_palabras = 0;
                    continue;
                }
                if (Negativo(archivo) == 1){
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "Error: No se pueden milisegundos negativos");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    continue;
                }
                ms = atoi(archivo);
                if (!Digito(archivo))
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: velocidad debe ser un entero positivo");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    ms = '\0';
                    continue;
                }
                if (num_palabras > 2)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    comando[0] = '\0';
                    num_PID = '\0';
                    num_palabras = 0;
                    continue;
                }
            }
            else
            {
                limpiarZona(y_mensajes, 0, ancho_procesos);
                mvprintw(y_mensajes, 0, "Comando no valido");
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                num_palabras = 0;
                continue;
            }
            //}
        }

        // Si no se tiene nada en ejecucion, pero si hay algo en listos
        if (lista_ejecucion == NULL && lista_listos != NULL)
        {
            struct Nodo *proceso = Fair_Share(&lista_listos, &lista_suspendidos, grupos, Base);
            if (proceso != NULL)
            {
                insertarFinal(&lista_ejecucion, proceso);
            }
            // mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
            limpiarZonaTabla(y_renglon_TMP, x_TMP, ancho_TMP);
            imprimir_TMP(proceso->TMP, y_renglon_TMP, x_TMP, proceso->num_paginas, proceso->PID);
            refresh();
        }
        if (lista_ejecucion == NULL){ // si no hay nada en ejecucion vuelve a empezar
            continue;
        }

        struct Nodo *procesoEjecucion = lista_ejecucion;
        limpiarZona(y_mensajes, 0, ancho_procesos);

        char linea[100];
        int contadorLinea = procesoEjecucion->PC;
        char *token, *arg1, *arg2, *instruccion;
        int encontroEND = 0; // Variable para ver casos de la instruccion END(si hay en el documento)
        int quantum = 3;
        int q = 0;
        int gcpu_acum = 0; // varible que le pasamos para que al terminar quantum(o termine) para acrualizar el GCPU del grupo
        int pagina = 0;
        mvprintw(y_header, 0, "%-10s %-18s %10s %10s %10s %10s %10s %10s ", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "CPU", "GCPU"); //(y,x,"fotmato",variables) -(alinear a la izquierda)10(espacios para esa variable)formato de variable
        mvprintw(y_header_TMP, x_TMP, "%-5s %5s %5s %5s", "Pagi", "Bit", "M_R", "M_S");
        mvprintw(y_header_TMM, x_TMM, "%-5s %5s %5s", "Marco", "Dueño", "Reloj");
        mvprintw(y_header_TMS, x_TMS, "%-5s %5s", "Pag", "Dueño");
        mvprintw(y_header2, 0, "%-5s %-5s %-8s %-8s %-18s %-18s %-10s %-18s %10s %10s %10s %10s %10s", "PID", "GID", "CPU", "GCPU", "Nombre", "Status", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "Prioridad");
        refresh();
        int desplazamiento = 0;

        if (procesoEjecucion != NULL){
            int finArchivo = 0;

            while (q < quantum){

                int direccion_virtual = procesoEjecucion->PC;
                pagina = direccion_virtual / 4;
                desplazamiento = direccion_virtual % 4;

                if (BitPresencia_TMP(procesoEjecucion->TMP, pagina) == 0){ // RAM --------------------------------
                    if (swap == NULL){
                        limpiarZona(y_mensajes, 0, ancho_procesos);
                        mvprintw(y_mensajes, 0, "Error abriendo swap");
                        refresh();
                        break;
                    }
                    if (RAMLlena(TMM) == 0)
                    {
                        EscrituraRam(swap, RAM, pagina, procesoEjecucion->TMP, TMM, procesoEjecucion->PID);
                        imprimir_TMP(procesoEjecucion->TMP, y_renglon_TMP, x_TMP, procesoEjecucion->num_paginas, procesoEjecucion->PID);
                        limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                        porcentajes(TMS, TMM, &porS, &porR);
                        refresh();
                    }
                    else{ // RAM llena
                        struct Nodo *procesoSuspendido = extraerNodo(&lista_ejecucion, procesoEjecucion->PID);
                        if (procesoSuspendido != NULL){
                            TiempoEnSuspendidos(procesoSuspendido);
                            procesoSuspendido->PC = contadorLinea;
                            insertarFinal(&lista_suspendidos, procesoSuspendido);
                            AlgoritmoReloj(TMM, RAM, lista_listos, lista_ejecucion, lista_suspendidos);
                            limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                            imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                            EscrituraRam(swap, RAM, pagina, procesoSuspendido->TMP, TMM, procesoSuspendido->PID);
                            imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                            limpiarZonaTabla(y_renglon_TMP, x_TMP, ancho_TMP);
                            imprimir_TMP(procesoSuspendido->TMP, y_renglon_TMP, x_TMP, procesoSuspendido->num_paginas, procesoSuspendido->PID);
                            porcentajes(TMS, TMM, &porS, &porR);
                        }

                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                        // continue;
                        // NOTA: fallo de pagina
                        break;
                    }

                    // imprimir_TMP(TMP,y_tablaR,procesoEjecucion->PID);
                }

                int MarcoRAM = procesoEjecucion->TMP[pagina][1];
                // mvprintw(4,0,"LINEA antes del mem");
                // refresh();
                memcpy(linea, &RAM[MarcoRAM][desplazamiento * 100], 100);
                // mvprintw(6,0,"%s",linea);
                refresh();
                // mvprintw(4,0,"LINEA despues del mem");
                // refresh();

                q++;
                contadorLinea++;
                procesoEjecucion->CPU += 20;
                procesoEjecucion->GCPU += 20;
                gcpu_acum = procesoEjecucion->GCPU;

                char linea_original[100]; // gaurdamos copia de lalinea
                strcpy(linea_original, linea);
                linea_original[strcspn(linea_original, "\n")] = '\0'; //(lineaaescanear, loquevaaencontrar)
                // mvprintw(4,0,"LINEA despues del mem284");
                // refresh();
                if (linea_original[0] == '\0')
                { // linea vacia
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: linea vacia en linea %d", contadorLinea);
                    refresh();
                    strcpy(procesoEjecucion->IR, linea_original); // guardamos el IR
                    A_terminadosError(&lista_ejecucion, &lista_terminados);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    if (Busqueda_GID(&lista_listos, &lista_ejecucion, &lista_suspendidos, procesoEjecucion->GID) == 0){ // revisar si todavia hay procesos con ese GID
                        liberarSWAP(swap, procesoEjecucion->TMP, procesoEjecucion->num_paginas, TMS);
                        liberarRAMproceso(RAM, TMM, procesoEjecucion->PID);
                        RevisarNuevos(&lista_nuevos, &lista_listos, swap, TMS);
                        limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                        porcentajes(TMS, TMM, &porS, &porR);
                        limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                        imprimir_TMS(TMS, y_renglon_TMS, x_TMS);

                        grupos--;
                    }
                    // mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                    huboError = 1;
                    comando[0] = '\0';
                    archivo[0] = '\0';
                    // mvprintw(4,0,"LINEA 271");
                    // refresh();
                    break;
                    // mvprintw(4,0,"LINEA 274");
                    // refresh();
                }

                token = strtok(linea, " \n\t ,");
                if (token == NULL)
                {
                    continue;
                }

                instruccion = token;

                arg1 = strtok(NULL, " \n\t ,");
                arg2 = strtok(NULL, " \n\t ,");
                // mvprintw(4,0,"LINEA despues del mem316");
                // refresh();
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                // Sintaxis para los espacios y Verifica si la instruccion es valida
                if (!validarEspacios(linea_original, instruccion, contadorLinea) || !Operaciones(instruccion, contadorLinea, linea_original))
                {
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion, &lista_terminados);
                    if (Busqueda_GID(&lista_listos, &lista_ejecucion, &lista_suspendidos, procesoEjecucion->GID) == 0){ 
                        liberarSWAP(swap, procesoEjecucion->TMP, procesoEjecucion->num_paginas, TMS);
                        liberarRAMproceso(RAM, TMM, procesoEjecucion->PID);
                        RevisarNuevos(&lista_nuevos, &lista_listos, swap, TMS);
                        limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                        imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                        limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                        porcentajes(TMS, TMM, &porS, &porR);
                        grupos--;
                    }
                    // mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    huboError = 1;
                    reiniciarVariables(comando, archivo);
                    break;
                }
                // mvprintw(4,0,"LINEA despues del mem331");
                // refresh();
                if ((strcmp(instruccion, "MOV") == 0 && !MOV(arg1, arg2, contadorLinea, linea_original, procesoEjecucion)) ||
                    (strcmp(instruccion, "ADD") == 0 && !ADD(arg1, arg2, contadorLinea, linea_original, procesoEjecucion)) ||
                    (strcmp(instruccion, "SUB") == 0 && !SUB(arg1, arg2, contadorLinea, linea_original, procesoEjecucion)) ||
                    (strcmp(instruccion, "MUL") == 0 && !MUL(arg1, arg2, contadorLinea, linea_original, procesoEjecucion)) ||
                    (strcmp(instruccion, "DIV") == 0 && !DIV(arg1, arg2, contadorLinea, linea_original, procesoEjecucion)) ||
                    (strcmp(instruccion, "INC") == 0 && !INC(arg1, contadorLinea, linea_original, procesoEjecucion)) ||
                    (strcmp(instruccion, "DEC") == 0 && !DEC(arg1, contadorLinea, linea_original, procesoEjecucion)) ||
                    (strcmp(instruccion, "JNZ") == 0 && !JNZ(arg1, contadorLinea, linea_original, procesoEjecucion)))
                {
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion, &lista_terminados);

                    if (Busqueda_GID(&lista_listos, &lista_ejecucion, &lista_suspendidos, procesoEjecucion->GID) == 0)
                    {
                        liberarSWAP(swap, procesoEjecucion->TMP, procesoEjecucion->num_paginas, TMS);
                        liberarRAMproceso(RAM, TMM, procesoEjecucion->PID);
                        RevisarNuevos(&lista_nuevos, &lista_listos, swap, TMS);
                        limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                        imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                        limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                        porcentajes(TMS, TMM, &porS, &porR);
                        grupos--;
                    }
                    // mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    huboError = 1;
                    reiniciarVariables(comando, archivo);
                    break;
                }
                else if (strcmp(instruccion, "JNZ") == 0){
                    contadorLinea = procesoEjecucion->PC;
                }

                else if ((strcmp(instruccion, "END") == 0)){
                    encontroEND = 1;

                    procesoEjecucion->PC = contadorLinea; // por que hace break y no se guardaria el END
                    strcpy(procesoEjecucion->IR, linea_original);

                    limpiarZona(y_renglon, 0, ancho_procesos);
                    mvprintw(y_renglon, 0, "%-10d %-18s %10d %10d %10d %10d", contadorLinea, linea_original, procesoEjecucion->EAX, procesoEjecucion->EBX, procesoEjecucion->ECX, procesoEjecucion->EDX);
                    refresh();
                    ComandoVel(ms);

                    struct Nodo *procesoTerminado = extraerPrimero(&lista_ejecucion);

                    if (procesoTerminado != NULL)
                    {

                        insertarFinal(&lista_terminados, procesoTerminado);
                        if (Busqueda_GID(&lista_listos, &lista_ejecucion, &lista_suspendidos, procesoEjecucion->GID) == 0){
                            liberarSWAP(swap, procesoTerminado->TMP, procesoTerminado->num_paginas, TMS);
                            liberarRAMproceso(RAM, TMM, procesoEjecucion->PID);
                            RevisarNuevos(&lista_nuevos, &lista_listos, swap, TMS);
                            limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                            imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                            limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                            imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                            porcentajes(TMS, TMM, &porS, &porR);
                            grupos--;
                        }
                    }

                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    reiniciarVariables(comando, archivo);
                    break;
                }
                procesoEjecucion->PC = contadorLinea; // guardamos la PC y IR ejecutado
                strcpy(procesoEjecucion->IR, linea_original);

                refresh();
                ComandoVel(ms); // Tiempo para ver las lineas de impresion para renglon

                if (kbhit()){
                    // imprimirlista(procesoEjecucion, y_procesoEjecucion);

                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    mvprintw(y_linea_comando, 0, "(D)> "); // Linea de comando que interrumpe(Dentro del kbhit)
                    char entrada[200];
                    char extra[100];
                    char extra2[100];
                    getnstr(entrada, 199);
                    num_palabras = sscanf(entrada, "%99s %99s %99s %99s", comando, archivo, extra, extra2); // NOTA: ya se agrego el extra2 para el fork

                    if (strcmp(comando, "salir") == 0)
                    {
                        if (num_palabras > 1)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: comando invalido");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            num_palabras = 0;
                            continue;
                        }
                        salirPrograma();
                    }

                    else if (strcmp(comando, "ejecuta") == 0){
                        if (num_palabras < 2){
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: falta el nombre del archivo");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            reiniciarVariables(comando, archivo);
                            num_palabras = 0;
                            continue;
                        }
                        else if (num_palabras > 2){
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            reiniciarVariables(comando, archivo);
                            num_palabras = 0;
                            continue;
                        }

                        pid++;
                        gid++;
                        grupos++;
                        // primero pasar a nuevos
                        if (validarArchivo(archivo) == 1){
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "No se pudo abrir el archivo %s", archivo);
                            refresh();
                            pid--;
                            gid--;
                            reiniciarVariables(comando, archivo);
                            num_palabras = 0;
                            continue;
                        }
                        int ContadorL = ContadorLineas(archivo);
                        int paginas = (ContadorL + 3) / 4;

                        insertar(&lista_nuevos, pid, gid, archivo, 0, paginas);

                        struct Nodo *nuevoP = buscar(lista_nuevos, pid);

                        nuevoP->num_paginas = paginas;
                        nuevoP->TMP = malloc(paginas * sizeof(int[3]));

                        if (nuevoP->TMP == NULL)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "ERROR: no se pudo crear TMP");
                            refresh();
                            continue;
                        }

                        in_TMP(nuevoP->TMP, nuevoP->num_paginas);
                        int marcos_libres = marcosLS(TMS);
                        if (ContadorL < 131072)
                        {
                            if (nuevoP->num_paginas <= marcos_libres)
                            {
                                // ver si se peude cargar a swap
                                int result_reescritura = reescritura(archivo, swap, pid, TMS, nuevoP->TMP);
                                imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                                porcentajes(TMS, TMM, &porS, &porR);
                                if (result_reescritura == 1)
                                { // no cupo
                                    mvprintw(y_mensajes, 0, "Proceso %d queda en nuevos: no hay espacio en swap", pid);
                                    reiniciarVariables(comando, archivo);
                                    refresh();
                                    continue;
                                }
                                else if (result_reescritura == 0)
                                {
                                    struct Nodo *p = extraerNodo(&lista_nuevos, pid); // pasamos a listos si todo bien
                                    if (p != NULL)
                                    {
                                        insertarFinal(&lista_listos, p);
                                    }
                                }
                                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                                limpiarZona(y_linea_comando, 0, ancho_procesos);
                                refresh();
                                num_palabras = 0;
                                continue;
                            }
                        }
                        else
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "ERROR: El proceso es mas grande que el swap.");
                            reiniciarVariables(comando, archivo);
                            refresh();
                            continue;
                        }
                    }
                    else if (strcmp(comando, "mata") == 0)
                    {
                        if (num_palabras < 2)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: falta el PID del proceso");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            comando[0] = '\0';
                            num_PID = '\0';
                            num_palabras = 0;
                            continue;
                        }
                        num_PID = atoi(archivo);
                        if (!num_PID)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: PID debe ser un entero");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            continue;
                        }
                        if (num_palabras > 2)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            comando[0] = '\0';
                            num_PID = '\0';
                            num_palabras = 0;
                            continue;
                        }

                        refresh();
                        int gid_matado = -1;
                        struct Nodo *p_matar = buscar(lista_ejecucion, num_PID);

                        if (p_matar == NULL){
                            p_matar = buscar(lista_listos, num_PID);
                        }
                        if (p_matar != NULL){
                            gid_matado = p_matar->GID;
                        }

                        int lista = matar(&lista_ejecucion, &lista_terminados, &lista_listos, num_PID,swap,TMS);

                        if (gid_matado != -1)
                        {
                            if (Busqueda_GID(&lista_listos, &lista_ejecucion, &lista_suspendidos, procesoEjecucion->GID) == 0)
                            {
                                
                                liberarSWAP(swap, procesoEjecucion->TMP, procesoEjecucion->num_paginas, TMS);
                                liberarRAMproceso(RAM, TMM, procesoEjecucion->PID);
                                limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                                RevisarNuevos(&lista_nuevos,&lista_listos,swap,TMS);
                                limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                                imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                                limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                                imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                                porcentajes(TMS, TMM, &porS, &porR);
                                grupos--;
                            }
                        }
                        // mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                        imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                        limpiarZona(y_linea_comando, 0, ancho_procesos);
                        refresh();
                        num_palabras = 0;
                        if (lista == 1){ // 1 -> esta en lista ejecucion, 2-> listos, 3 -> terminados, 0->no esta el PID
                            break;
                        }

                        continue;
                    }
                    else if (strcmp(comando, "fork") == 0)
                    {
                        if (num_palabras < 2)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: falta pid del proceso");
                            refresh();
                            num_palabras = 0;
                            continue;
                        }
                        if (num_palabras < 3)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: falta numero de instruccion");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            num_palabras = 0;
                            continue;
                        }
                        if (Negativo(extra) == 1){
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "Error: PC no puede ser negativo");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            continue;
                        }
                        num_PID = atoi(archivo); //NOTA:no borrar si se usa, es lo que mandamos a la funcion
                        num_PC = atoi(extra);
                        if (!Digito(archivo))
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: PID debe ser un entero");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            num_PID = '\0';
                            num_palabras = 0;
                            num_PID = '\0';
                            continue;
                        }
                        if (!Digito(extra)){ // NOTA: atoi no filta bien el termino ya que acepta fork 1 2gakigski
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: PC debe ser un entero");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            num_palabras = 0;
                            num_PID = '\0';
                            num_PC = '\0';
                            continue;
                        }
                        if (num_palabras > 3)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            num_palabras = 0;
                            num_PID = '\0';
                            num_PC = '\0';
                            continue;
                        }
                        pid++;
                        // num_PID num_PC son las variables que estan en el comando
                        struct Nodo *nuevo = forkProcesoComando(&lista_ejecucion, &lista_terminados, &lista_listos, &lista_suspendidos, num_PID, num_PC, pid);

                        if (nuevo == NULL)
                        {
                            pid--;
                        }

                        imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                        limpiarZona(y_linea_comando, 0, ancho_procesos);
                        refresh();
                    }
                    else if (strcmp(comando, "velocidad") == 0)
                    {
                        if (num_palabras < 2)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "ERROR: falta los milisegundos");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            comando[0] = '\0';
                            num_palabras = 0;
                            continue;
                        }
                        if (Negativo(archivo) == 1)
                        {
                            mvprintw(y_mensajes, 0, "Error: No se pueden milisegundos negativos");
                            refresh();
                            continue;
                        }
                        ms = atoi(archivo);
                        if (!Digito(archivo)){//NOTA: VELOVIDAD ACEPTA velocidad 38ksgdk
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "ERROR: velocidad debe ser un entero positivo");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            ms = '\0';
                            refresh();
                            continue;
                        }
                        if (num_palabras > 2)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            comando[0] = '\0';
                            num_palabras = 0;
                            continue;
                        }
                    }
                    else{ // La interrupcion con un comando que no es Salir o Ejecuta o mata
                        limpiarZona(y_mensajes, 0, ancho_procesos);
                        mvprintw(y_mensajes, 0, "(D)Comando no valido");
                        limpiarZona(y_linea_comando, 0, ancho_procesos);
                        refresh();
                        reiniciarVariables(comando, archivo);
                        continue;
                    }
                }
            }

            if (lista_ejecucion == NULL)
            {
                limpiarZonaTabla(y_renglon_TMP, x_TMP, ancho_TMP);
                reiniciarVariables(comando, archivo);
                continue;
            }
            procesoEjecucion->PC = contadorLinea;
            if (q == quantum && encontroEND == 0 && huboError == 0)
            { // leyo 3 inst y no termino
                struct Nodo *p = extraerPrimero(&lista_ejecucion);
                if (p != NULL)
                {
                    insertarFinal(&lista_listos, p);
                }
                GCPU_Global(&lista_listos, &lista_suspendidos, procesoEjecucion->GID, gcpu_acum);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
            }
            else if (encontroEND == 0 && huboError == 0 && finArchivo == 1)
            { // se acbo el archivo sin END
                limpiarZona(y_mensajes, 0, ancho_procesos);
                mvprintw(y_mensajes, 0, "ERROR: Fin de archivo sin END");
                refresh();

                A_terminadosError(&lista_ejecucion, &lista_terminados);
                if (Busqueda_GID(&lista_listos, &lista_ejecucion, &lista_suspendidos, procesoEjecucion->GID) == 0)
                {
                    liberarSWAP(swap, procesoEjecucion->TMP, procesoEjecucion->num_paginas, TMS);
                    liberarRAMproceso(RAM, TMM, procesoEjecucion->PID);
                    RevisarNuevos(&lista_nuevos,&lista_listos,swap,TMS);
                    limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                    imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                    limpiarZonaTabla(y_renglon_TMM, x_TMM, ancho_TMM);
                    imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                    porcentajes(TMS, TMM, &porS, &porR);
                    grupos--;
                }
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                reiniciarVariables(comando, archivo);
                continue;
            }
        }
    }
    endwin();
}