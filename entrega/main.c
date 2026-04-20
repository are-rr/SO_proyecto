#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "procesos.h"

// cordenadas de fila
int y_header = 0;
int y_renglon = 1;
int y_mensajes = 3;
int y_linea_comando = 5;
int y_header2 = 7;
int y_procesoEjecucion = 8;

int ejecutando = 1;
int pid =0;

int main(){
    char comando[100];
    char archivo[100];
    int num_PID;
    int num_palabras;
    initscr();
    comando[0] = '\0';
    archivo[0] = '\0';
    struct Nodo *lista_listos = NULL;
    struct Nodo *lista_ejecucion = NULL;
    struct Nodo *lista_terminados = NULL;
    int huboError = 0;
    while (ejecutando){
        huboError = 0; //reiniciamos a cada interacion la bandera de errores

        if ((lista_ejecucion == NULL && lista_listos == NULL)){
            char entrada[200];
            char extra[100];
            comando[0] = '\0';
            archivo[0] = '\0';

            move(y_linea_comando, 0); clrtoeol();
            mvprintw(y_linea_comando, 0, "> ");
            refresh();

            getnstr(entrada, 199); //lee la entrada
            num_palabras = sscanf(entrada, "%99s %99s %99s", comando, archivo, extra); //sscanf(cadena, formato, &variable1, etc.);
            move(y_linea_comando, 0); clrtoeol();
            refresh();
            if (strcmp(comando, "salir") == 0){
                if (num_palabras > 1){
                    move(y_mensajes, 0); clrtoeol();
                    refresh();
                    mvprintw(y_mensajes, 0, "ERROR: comando invalido");
                    refresh();
                    num_palabras = 0;
                    continue;
                }
                salirPrograma();
            }
            else if (strcmp(comando, "ejecuta") == 0){
                if (num_palabras < 2) {
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: falta el nombre del archivo");
                    refresh();
                    continue;
                }
                else if (num_palabras > 2){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                    refresh();
                    num_palabras = 0;
                    continue;
                }

                FILE *file = fopen(archivo, "r");

                if (file == NULL){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "No se pudo abrir el archivo %s", archivo);
                    refresh();
                    continue;
                }

                pid++;
                insertar(&lista_listos,pid,file,archivo,'L',0); //el proceso se inserta en la lista de listos
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
                refresh();            
            } else if (strcmp(comando, "mata") == 0){
                if (num_palabras < 2) {
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: falta el PID del proceso");
                    refresh();
                    num_palabras= 0;
                    continue;
                }
                num_PID = atoi(archivo); //convertimos el PID a entero
                if(!num_PID){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: PID debe ser un entero");
                    refresh();
                    continue;
                }
                if (num_palabras > 2){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                    refresh();
                    num_palabras= 0;
                    continue;
                }
                if(lista_listos == NULL && lista_ejecucion == NULL && lista_terminados == NULL){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: no hay procesos que matar");
                    refresh();
                    num_palabras= 0;
                    continue;
                }

            }else{
                move(y_mensajes, 0); clrtoeol();
                mvprintw(y_mensajes, 0, "Comando no valido");
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                num_palabras = 0;
                continue;
            }
        }
        //Si no se tiene nada en ejecucion, pero si hay algo en listos
        if(lista_ejecucion == NULL && lista_listos != NULL){
            struct Nodo *proceso = extraerPrimero(&lista_listos); 
            if(proceso != NULL){
                proceso -> Status = 'E';
                insertarFinal(&lista_ejecucion,proceso);
            }
            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
            refresh(); 
        }
        if(lista_ejecucion == NULL){//si no hay nada en ejecucion vuelve a empexar
            continue;
        }

        struct Nodo *procesoEjecucion = lista_ejecucion;
        move(y_mensajes, 0); clrtoeol();
                   
        char linea[100];
        int contadorLinea = procesoEjecucion -> PC;
        char *token, *arg1, *arg2, *instruccion;
        int encontroEND = 0; //Variable para ver casos de la instruccion END(si hay en el documento)
        int quantum=3;
        int q=0;

        mvprintw(y_header, 0, "%-10s %-20s %10s %10s %10s %10s", "PC", "IR", "EAX", "EBX", "ECX", "EDX");//(y,x,"fotmato",variables) -(alinear a la izquierda)10(espacios para esa variable)formato de variable
        mvprintw(y_header2, 0, "%-5s %-20s %-18s %-10s %-20s %10s %10s %10s %10s", "PID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX");
        refresh();
        
        if(procesoEjecucion != NULL){
            int finArchivo = 0;
            while (q < quantum) {
                if (fgets(linea, sizeof(linea), procesoEjecucion->Archivo) == NULL) { //lee la linea del archivo
                    finArchivo = 1;
                    break;
                }
                q++;  
                contadorLinea++;
                
                
                char linea_original[100]; //gaurdamos copia de lalinea
                strcpy(linea_original, linea);
                linea_original[strcspn(linea_original, "\r\n")] = '\0';//(lineaaescanear, loquevaaencontrar)

                if(linea_original[0] == '\0'){ //linea vacia
                    move(y_mensajes,0); clrtoeol();
                    mvprintw(y_mensajes,0,"ERROR: linea vacia en linea %d", contadorLinea);
                    refresh();
                    strcpy(procesoEjecucion->IR, linea_original); //guardamos el IR
                    A_terminadosError(&lista_ejecucion,&lista_terminados);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
                    huboError = 1;
                    comando[0] = '\0';
                    archivo[0] = '\0';
                    break;
                }

                token = strtok(linea, " \n\t ,");
                if (token == NULL){
                    continue;
                }

                instruccion = token;

                arg1 = strtok(NULL, " \n\t ,");
                arg2 = strtok(NULL, " \n\t ,");

                // Sintaxis para los espacios y Verifica si la instruccion es valida
                if (!validarEspacios(linea_original, instruccion, contadorLinea)
                    || !Operaciones(instruccion, contadorLinea, linea_original)){
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion,&lista_terminados);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
                    huboError = 1;
                    reiniciarVariables(comando,archivo);
                    break;
                }

                if ((strcmp(instruccion, "MOV") == 0 && !MOV(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "ADD") == 0 && !ADD(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "SUB") == 0 && !SUB(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "MUL") == 0 && !MUL(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "DIV") == 0 && !DIV(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "INC") == 0 && !INC(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "DEC") == 0 && !DEC(arg1,arg2,contadorLinea,linea_original,procesoEjecucion))) {
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion,&lista_terminados);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
                    huboError = 1;
                    reiniciarVariables(comando,archivo);
                    break;
                }
                    
                else if ((strcmp(instruccion, "END") == 0)){
                    encontroEND = 1;
                        
                    procesoEjecucion->PC = contadorLinea; //por que hace break y no se guardaria el END
                    strcpy(procesoEjecucion->IR, linea_original);
                        
                        if(feof(procesoEjecucion -> Archivo)){//Encontro END y se acabo el archivo(correcto)
                            move(y_renglon, 0); clrtoeol();
                            mvprintw(y_renglon, 0, "%-10d %-20s %10d %10d %10d %10d", contadorLinea, linea_original, procesoEjecucion->EAX, procesoEjecucion->EBX,procesoEjecucion->ECX,procesoEjecucion->EDX);
                            refresh();
                            //napms(1000);
                            
                            struct Nodo *procesoTerminado = extraerPrimero(&lista_ejecucion); 
                            if(procesoTerminado != NULL){
                                procesoTerminado -> Status = 'T';
                                if(procesoTerminado -> Archivo != NULL){
                                    fclose(procesoTerminado->Archivo);
                                    procesoTerminado->Archivo = NULL;
                                }
                                insertarFinal(&lista_terminados,procesoTerminado);
                            }
                            
                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);                               
                            reiniciarVariables(comando,archivo);
                            break;
                        } else { //Solo encontro END
                            mvprintw(y_mensajes, 0, "ERROR: END encontrado sin que el archivo terminara linea %d:\"%s\"", contadorLinea, linea_original);
                            refresh();
                            A_terminadosError(&lista_ejecucion,&lista_terminados);
                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);                               
                            reiniciarVariables(comando,archivo);
                            huboError = 1;
                            break;
                        }
                }
                procesoEjecucion->PC = contadorLinea; //guardamos la PC y IR ejecutado
                strcpy(procesoEjecucion->IR, linea_original);
                refresh();
               // napms(1000); //Tiempo para ver las lineas de impresion para renglon
                    
                    if (kbhit()){
                        imprimirlista(procesoEjecucion, y_procesoEjecucion);

                        move(y_linea_comando, 0); clrtoeol();
                        refresh();
                        mvprintw(y_linea_comando, 0, "(D)> "); //Linea de comando que interrumpe(Dentro del kbhit)
                        char entrada[200];
                        char extra[100];
                        getnstr(entrada, 199);
                        num_palabras = sscanf(entrada, "%99s %99s %99s", comando, archivo, extra);

                        if (strcmp(comando, "salir") == 0){
                            if (num_palabras > 1){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: comando invalido");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                num_palabras = 0;
                                continue;
                            }
                            salirPrograma();
                        }

                        else if (strcmp(comando, "ejecuta") == 0){
                            if (num_palabras < 2){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: falta el nombre del archivo");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                comando[0] = '\0';
                                archivo[0] = '\0';
                                num_palabras = 0;
                                continue;
                            }
                            else if (num_palabras > 2){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                comando[0] = '\0';
                                archivo[0] = '\0';
                                num_palabras = 0;
                                continue;
                            }

                            FILE *file_interrupcion = fopen(archivo, "r");
                            if (file_interrupcion == NULL){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)No se pudo abrir el archivo %s", archivo);
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                comando[0] = '\0';
                                archivo[0] = '\0';
                                num_palabras = 0;
                                continue;
                            }
                            
                            pid++;
                            insertar(&lista_listos,pid,file_interrupcion,archivo,'L',0); 
                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
                            move(y_linea_comando, 0); clrtoeol();
                            refresh();
                            num_palabras = 0;                
                            continue;
                        }
                        else if (strcmp(comando, "mata") == 0){
                            if (num_palabras < 2) {
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: falta el PID del proceso");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                comando[0] = '\0';
                                num_PID = '\0';
                                num_palabras = 0;
                                continue;
                            }
                            num_PID = atoi(archivo);
                            if(!num_PID){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: PID debe ser un entero");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                continue;
                            }
                            if (num_palabras > 2){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                comando[0] = '\0';
                                num_PID = '\0';
                                num_palabras = 0;
                                continue;
                            }

                            refresh();
                            int lista = matar(&lista_ejecucion,&lista_terminados,&lista_listos,num_PID);
                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
                            move(y_linea_comando, 0); clrtoeol();
                            refresh();
                            num_palabras = 0;
                            if(lista == 1){ //1 -> esta en lista ejecucion, 2-> listos, 3 -> terminados, 0->no esta el PID
                                break;
                            }
                            
                            continue;
                            
                        }else{//La interrupcion con un comando que no es Salir o Ejecuta o mata
                            move(y_mensajes, 0); clrtoeol();
                            mvprintw(y_mensajes, 0, "(D)Comando no valido");
                            move(y_linea_comando, 0); clrtoeol();
                            refresh();
                            continue;
                        }
                    }
            }
            if (lista_ejecucion == NULL) {
                reiniciarVariables(comando, archivo);
                continue;
            }
            procesoEjecucion->PC = contadorLinea;
            
            if(q==quantum && encontroEND == 0 && huboError == 0){ //leyo 3 inst y no termino
                struct Nodo *p = extraerPrimero(&lista_ejecucion);
                if(p != NULL){
                    p -> Status = 'L';
                    insertarFinal(&lista_listos,p);
                }
                imprimirEstado(lista_listos,lista_ejecucion,lista_terminados);
            }
            else if (encontroEND == 0 && huboError == 0 && finArchivo == 1){ //se acbo el archivo sin END
                //else if (encontroEND == 0 && huboError == 0 && feof(procesoEjecucion->Archivo)){
                move(y_mensajes, 0); clrtoeol();
                mvprintw(y_mensajes, 0, "ERROR: Fin de archivo sin END");
                refresh();
               
                A_terminadosError(&lista_ejecucion,&lista_terminados);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);                               
                reiniciarVariables(comando,archivo);
                continue;
            }
        }   
    }
    endwin();
}