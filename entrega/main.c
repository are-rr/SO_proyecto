#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "procesos.h"
#include <time.h>

// cordenadas de fila
int ancho_procesos = 155;
int ancho_tablas = 35;
int y_header = 0;
int y_renglon = 1;
int y_mensajes = 3;
int y_variable = 4;
int y_linea_comando = 5;
int y_header2 = 7;
int y_procesoEjecucion = 8;

int y_header_TMP = 1;
int y_renglon_TMP = 2;
int x_TMP = 160;

int y_header_TMM = 1;
int y_renglon_TMM = 2;
int x_TMM = 195;

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
    int num_PID;      // pid que brindo en el comando
    int num_palabras; // palabras que se cuentan de la entrada, para los filtros
    initscr();

    comando[0] = '\0';
    archivo[0] = '\0';
    struct Nodo *lista_listos = NULL;
    struct Nodo *lista_ejecucion = NULL;
    struct Nodo *lista_terminados = NULL;
    struct Nodo *lista_nuevos = NULL;
    struct Nodo *lista_suspendidos = NULL;
    int huboError = 0;
    int TMS[32768]; // solo tiene PID dueño
    in_TMS(TMS);
    int TMM[16][2]; // tiene dueño y reloj
    in_TMM(TMM);
    char RAM[16][400];

    char *ArchivoBinario = "archivoBinario.bin";
    if (Crear_ArchivoBinario(ArchivoBinario, 100) == 0)
    {
        refresh();
    }

    FILE *swap = fopen(ArchivoBinario, "r+b");
    while (ejecutando)
    {
        huboError = 0; // reiniciamos a cada interacion la bandera de errores

        if ((lista_ejecucion == NULL && lista_listos == NULL))
        {
            char entrada[200];
            char extra[100];
            char extra2[100];
            comando[0] = '\0';
            archivo[0] = '\0';
            limpiarZona(y_linea_comando, 0, ancho_procesos);
            mvprintw(y_linea_comando, 0, "> ");
            refresh();

            if (lista_suspendidos != NULL)
            { // madamos la direccion del pntero lista_suspendidos y listos
                RevisarSuspendidos(swap, RAM, lista_ejecucion, &lista_listos, &lista_suspendidos, TMM, TMS, &porS, &porR);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                move(y_linea_comando, 2);
                refresh();

                if (!kbhit())
                {
                    continue;
                }
            }
            getnstr(entrada, 199);

            num_palabras = sscanf(entrada, "%99s %99s %99s %99s", comando, archivo, extra, extra2); // sscanf(cadena, formato, &variable1, etc.);
            if (num_palabras <= 0)
            {

                continue;
            }

            limpiarZona(y_linea_comando, 0, ancho_procesos);
            refresh();
            if (strcmp(comando, "salir") == 0)
            {
                if (num_palabras > 1)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: comando invalido");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    num_palabras = 0;
                    continue;
                }
                salirPrograma(swap);
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
                // grupos++;
                //  primero pasar a nuevos
                if (validarArchivo(archivo) == 1)
                {
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
                if (paginas <= 32768)
                {
                    insertar(&lista_nuevos, pid, gid, 0, paginas, num_lineas, archivo);

                    struct Nodo *nuevo = buscar(lista_nuevos, pid);
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
                    // if (paginas <= 32768)
                    //{
                    //  ver si se puede cargar a swap
                    if (nuevo->num_paginas <= marcos_libres)
                    {
                        int result_reescritura = reescritura(swap, archivo, nuevo->TMP, TMS, pid);
                        // limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                        // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                        porcentajes(TMM, TMS, &porS, &porR);

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
                                grupos++;
                                insertarFinal(&lista_listos, p);
                            }
                        }
                        imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    }
                }
                else
                {
                    pid--;
                    gid--;
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: El proceso es mas grande que el swap.");
                    reiniciarVariables(comando, archivo);
                    refresh();
                    continue;
                }
            }
            else if (strcmp(comando, "mata") == 0)
            {
                if (lista_listos == NULL && lista_ejecucion == NULL && lista_terminados == NULL && lista_suspendidos == NULL && lista_nuevos == NULL)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: no hay procesos que matar");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    num_palabras = 0;
                    continue;
                }
                int mato_ejecucion = procesarMata(swap, RAM, archivo, &lista_ejecucion, &lista_listos, &lista_terminados, &lista_suspendidos, &lista_nuevos,
                                                  TMM, TMS, num_palabras, &grupos, &porS, &porR);
                if (mato_ejecucion)
                {
                    break;
                }
                continue;
            }
            else if (strcmp(comando, "fork") == 0)
            {
                if (lista_listos == NULL && lista_ejecucion == NULL && lista_terminados == NULL && lista_suspendidos == NULL)
                {
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: no hay procesos para duplicar");
                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    num_palabras = 0;
                    continue;
                }
                procesarFork(archivo, extra, &lista_ejecucion, &lista_listos, &lista_terminados, &lista_suspendidos, num_palabras, &pid);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                limpiarZona(y_linea_comando, 0, ancho_procesos);
                refresh();
                continue;
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
        }
        if (lista_listos != NULL || lista_suspendidos != NULL)
        { // madamos la direccion del pntero lista_suspendidos y listos
            RevisarSuspendidos(swap, RAM, lista_ejecucion, &lista_listos, &lista_suspendidos, TMM, TMS, &porS, &porR);
            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
            move(y_linea_comando, 2);
            refresh();
            
        }
        limpiarZonaTabla(y_renglon_TMM, x_TMM);
        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
        // mvprintw(y_variable, 0, "numero de grupos:%d", grupos);
        //  Si no se tiene nada en ejecucion, pero si hay algo en listos
        if (lista_ejecucion == NULL && lista_listos != NULL)
        { // aqui proceso apunta al proceso que Fair Share eligio ejecutar
            struct Nodo *proceso = Fair_Share(&lista_listos, &lista_suspendidos, grupos, Base);
            if (proceso != NULL)
            {
                insertarFinal(&lista_ejecucion, proceso);
                //mvprintw(y_variable, 0, "numero de grupos:%d", grupos);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                limpiarZonaTabla(y_renglon_TMP, x_TMP);
                imprimir_TMP(proceso->TMP, y_renglon_TMP, x_TMP, proceso->num_paginas, proceso->PID);
                refresh();
            }
            
        }
        if (lista_ejecucion == NULL)
         { // si no hay nada en ejecucion vuelve a empezar
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
        // mvprintw(y_header_TMS, x_TMS, "%-5s %5s", "Pag", "Dueño");
        mvprintw(y_header2, 0, "%-5s %-5s %-8s %-8s %-18s %-18s %-10s %-18s %10s %10s %10s %10s %10s", "PID", "GID", "CPU", "GCPU", "Nombre", "Status", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "Prioridad");
        refresh();
        int desplazamiento = 0;

        if (procesoEjecucion != NULL)
        {
            int finArchivo = 0;

            while (q < quantum)
            {

                int direccion_virtual = procesoEjecucion->PC;
                pagina = direccion_virtual / 4;
                desplazamiento = direccion_virtual % 4;

                if (BitPresencia_TMP(procesoEjecucion->TMP, pagina) == 0){ // RAM

                    // RAM llena
                    struct Nodo *procesoSuspendido = extraerNodo(&lista_ejecucion, procesoEjecucion->PID);
                    if (procesoSuspendido != NULL)
                    {
                        TiempoEnSuspendidos(procesoSuspendido);
                        procesoSuspendido->PC = contadorLinea;
                        //actualizarTMPGrupo(&lista_ejecucion, &lista_listos, &lista_suspendidos, procesoSuspendido);
                        insertarFinal(&lista_suspendidos, procesoSuspendido);
                        limpiarZonaTabla(y_renglon_TMP, x_TMP);
                        imprimir_TMP(procesoSuspendido->TMP, y_renglon_TMP, x_TMP, procesoSuspendido->num_paginas, procesoSuspendido->PID);
                        refresh();
                    }

                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    break;

                    // imprimir_TMP(TMP,y_tablaR,procesoEjecucion->PID);
                }

                int MarcoRAM = procesoEjecucion->TMP[pagina][1];
                memcpy(linea, &RAM[MarcoRAM][desplazamiento * 100], 100);
                refresh();

                q++;
                contadorLinea++;
                procesoEjecucion->CPU += 20;
                procesoEjecucion->GCPU += 20;
                gcpu_acum = procesoEjecucion->GCPU;

                char linea_original[100]; // gaurdamos copia de lalinea
                strcpy(linea_original, linea);
                linea_original[strcspn(linea_original, "\n")] = '\0'; //(lineaaescanear, loquevaaencontrar)

                if (linea_original[0] == '\0')
                { // linea vacia
                    limpiarZona(y_mensajes, 0, ancho_procesos);
                    mvprintw(y_mensajes, 0, "ERROR: linea vacia en linea %d", contadorLinea);
                    refresh();
                    strcpy(procesoEjecucion->IR, linea_original); // guardamos el IR
                    A_terminadosError(&lista_ejecucion, &lista_terminados);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);

                    if (Busqueda_GID(&lista_ejecucion, &lista_listos, &lista_suspendidos, procesoEjecucion->GID) == 0)
                    { // revisar si todavia hay procesos con ese GID
                        liberarSWAP(swap, procesoEjecucion->TMP, TMS, procesoEjecucion->num_paginas);
                        liberarRAMproceso(RAM, TMM, procesoEjecucion->GID);
                        RevisarNuevos(swap, &lista_listos, &lista_nuevos, TMS);
                        limpiarZonaTabla(y_renglon_TMS, x_TMS);
                        porcentajes(TMM, TMS, &porS, &porR);
                        limpiarZonaTabla(y_renglon_TMM, x_TMM);
                        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                        // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                        grupos--;
                    }
                    //mvprintw(y_variable, 0, "numero de grupos:%d", grupos);
                    huboError = 1;
                    comando[0] = '\0';
                    archivo[0] = '\0';
                    break;
                }

                token = strtok(linea, " \n\t ,");
                if (token == NULL)
                {
                    continue;
                }

                instruccion = token;
                arg1 = strtok(NULL, " \n\t ,");
                arg2 = strtok(NULL, " \n\t ,");

                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                // Sintaxis para los espacios y Verifica si la instruccion es valida
                if (!validarEspacios(linea_original, instruccion, contadorLinea) || !Operaciones(instruccion, linea_original, contadorLinea))
                {
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion, &lista_terminados);
                    if (Busqueda_GID(&lista_ejecucion, &lista_listos, &lista_suspendidos, procesoEjecucion->GID) == 0)
                    {
                        liberarSWAP(swap, procesoEjecucion->TMP, TMS, procesoEjecucion->num_paginas);
                        liberarRAMproceso(RAM, TMM, procesoEjecucion->GID);
                        RevisarNuevos(swap, &lista_listos, &lista_nuevos, TMS);
                        // limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                        // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                        limpiarZonaTabla(y_renglon_TMM, x_TMM);
                        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                        porcentajes(TMM, TMS, &porS, &porR);
                        grupos--;
                    }
                    // mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    huboError = 1;
                    reiniciarVariables(comando, archivo);
                    break;
                }

                if ((strcmp(instruccion, "MOV") == 0 && !MOV(procesoEjecucion, arg1, arg2, linea_original, contadorLinea)) ||
                    (strcmp(instruccion, "ADD") == 0 && !ADD(procesoEjecucion, arg1, arg2, linea_original, contadorLinea)) ||
                    (strcmp(instruccion, "SUB") == 0 && !SUB(procesoEjecucion, arg1, arg2, linea_original, contadorLinea)) ||
                    (strcmp(instruccion, "MUL") == 0 && !MUL(procesoEjecucion, arg1, arg2, linea_original, contadorLinea)) ||
                    (strcmp(instruccion, "DIV") == 0 && !DIV(procesoEjecucion, arg1, arg2, linea_original, contadorLinea)) ||
                    (strcmp(instruccion, "INC") == 0 && !INC(procesoEjecucion, arg1, linea_original, contadorLinea)) ||
                    (strcmp(instruccion, "DEC") == 0 && !DEC(procesoEjecucion, arg1, linea_original, contadorLinea)) ||
                    (strcmp(instruccion, "JNZ") == 0 && JNZ(procesoEjecucion, arg1, linea_original, contadorLinea) == 0))
                {
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion, &lista_terminados);

                    if (Busqueda_GID(&lista_ejecucion, &lista_listos, &lista_suspendidos, procesoEjecucion->GID) == 0)
                    {
                        liberarSWAP(swap, procesoEjecucion->TMP, TMS, procesoEjecucion->num_paginas);
                        liberarRAMproceso(RAM, TMM, procesoEjecucion->GID);
                        RevisarNuevos(swap, &lista_listos, &lista_nuevos, TMS);
                        // limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                        // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                        limpiarZonaTabla(y_renglon_TMM, x_TMM);
                        imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                        porcentajes(TMM, TMS, &porS, &porR);
                        grupos--;
                    }
                    //mvprintw(y_variable, 0, "numero de grupos:%d", grupos);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);
                    huboError = 1;
                    reiniciarVariables(comando, archivo);
                    break;
                }
                else if (strcmp(instruccion, "JNZ") == 0 && JNZ(procesoEjecucion, arg1, linea_original, contadorLinea) == 1)
                {
                    contadorLinea = procesoEjecucion->PC;
                }
                else if (strcmp(instruccion, "JNZ") == 0 && JNZ(procesoEjecucion, arg1, linea_original, contadorLinea) == 2)
                {
                    q++;
                }

                else if ((strcmp(instruccion, "END") == 0))
                {
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
                        if (Busqueda_GID(&lista_ejecucion, &lista_listos, &lista_suspendidos, procesoEjecucion->GID) == 0)
                        {
                            liberarSWAP(swap, procesoTerminado->TMP, TMS, procesoTerminado->num_paginas);
                            liberarRAMproceso(RAM, TMM, procesoTerminado->GID);
                            RevisarNuevos(swap, &lista_listos, &lista_nuevos, TMS);
                            // limpiarZonaTabla(y_renglon_TMS, x_TMS, ancho_TMS);
                            // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                            limpiarZonaTabla(y_renglon_TMM, x_TMM);
                            imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                            porcentajes(TMM, TMS, &porS, &porR);
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

                if (kbhit())
                {
                    // imprimirlista(procesoEjecucion, y_procesoEjecucion);

                    limpiarZona(y_linea_comando, 0, ancho_procesos);
                    refresh();
                    mvprintw(y_linea_comando, 0, "(D)> "); // Linea de comando que interrumpe(Dentro del kbhit)
                    char entrada[200];
                    char extra[100];
                    char extra2[100];
                    getnstr(entrada, 199);
                    num_palabras = sscanf(entrada, "%99s %99s %99s %99s", comando, archivo, extra, extra2);

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
                        salirPrograma(swap);
                    }

                    else if (strcmp(comando, "ejecuta") == 0)
                    {
                        if (num_palabras < 2)
                        {
                            limpiarZona(y_mensajes, 0, ancho_procesos);
                            mvprintw(y_mensajes, 0, "(D)ERROR: falta el nombre del archivo");
                            limpiarZona(y_linea_comando, 0, ancho_procesos);
                            refresh();
                            reiniciarVariables(comando, archivo);
                            num_palabras = 0;
                            continue;
                        }
                        else if (num_palabras > 2)
                        {
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
                        // grupos++;
                        //  primero pasar a nuevos
                        if (validarArchivo(archivo) == 1)
                        {
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
                        if (paginas <= 32768)
                        {
                            insertar(&lista_nuevos, pid, gid, 0, paginas, ContadorL, archivo);

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
                            // if (paginas <= 32768)
                            //{
                            if (nuevoP->num_paginas <= marcos_libres)
                            {
                                // ver si se peude cargar a swap
                                int result_reescritura = reescritura(swap, archivo,nuevoP->TMP, TMS, pid);
                                // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                                porcentajes(TMM, TMS, &porS, &porR);
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
                                        grupos++;
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
                            pid--;
                            gid--;
                            refresh();
                            continue;
                        }
                        // mvprintw(y_variable, 0, "numero de grupos:%d", grupos);
                    }
                    else if (strcmp(comando, "mata") == 0)
                    {
                        int mato_ejecucion = procesarMata(swap, RAM, archivo, &lista_ejecucion, &lista_listos, &lista_terminados, &lista_suspendidos, &lista_nuevos,
                                                          TMM, TMS, num_palabras, &grupos, &porS, &porR);
                        if (mato_ejecucion)
                        {
                            break;
                        }
                        continue;
                    }
                    else if (strcmp(comando, "fork") == 0)
                    {
                        procesarFork(archivo, extra, &lista_ejecucion, &lista_listos, &lista_terminados, &lista_suspendidos, num_palabras, &pid);
                        imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos, lista_nuevos);

                        limpiarZona(y_linea_comando, 0, ancho_procesos);
                        refresh();

                        continue;
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
                        if (!Digito(archivo))
                        {
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
                    else
                    { // La interrupcion con un comando que no es Salir o Ejecuta o mata
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
                limpiarZonaTabla(y_renglon_TMP, x_TMP);
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
                if (Busqueda_GID(&lista_ejecucion, &lista_listos, &lista_suspendidos, procesoEjecucion->GID) == 0)
                {
                    liberarSWAP(swap, procesoEjecucion->TMP, TMS, procesoEjecucion->num_paginas);
                    liberarRAMproceso(RAM, TMM, procesoEjecucion->GID);
                    RevisarNuevos(swap, &lista_listos, &lista_nuevos, TMS);
                    limpiarZonaTabla(y_renglon_TMS, x_TMS);
                    // imprimir_TMS(TMS, y_renglon_TMS, x_TMS);
                    limpiarZonaTabla(y_renglon_TMM, x_TMM);
                    imprimir_TMM(TMM, y_renglon_TMM, x_TMM);
                    porcentajes(TMM, TMS, &porS, &porR);
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