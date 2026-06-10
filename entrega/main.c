#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "procesos.h"
#include <time.h>

// cordenadas de fila
int y_header = 0;
int y_renglon = 1;
int y_mensajes = 3;
int y_linea_comando = 5;
int y_tabla = 13;
int y_tablaR = 14;
int y_header2 = 7;
int y_procesoEjecucion = 8;

int ejecutando = 1;
int pid =0;
int gid =0;

int Base = 60;
int grupos=0; //para contar cuantos grupos tenemos

int ms = 1000;

int main(){
    char comando[100];
    char archivo[100];
    int num_PID; //pid que brindo en el comando
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
    //int TMP[32768][3];
    //in_TMP(TMP);
    char RAM[16][400];
    //in_RAM(RAM);

    char *ArchivoBinario = "archivoBinario.bin";
    if( Crear_ArchivoBinario(ArchivoBinario, 100) == 0){
            mvprintw(y_mensajes, 0, "Se creo correctamente el Archivo Binario");
            refresh();
    }//no estoy seguro si ese 100 puede ir asi, pero es el tamaño de char que tenemos para el IR

    FILE *swap = fopen(ArchivoBinario,"r+b");
    while (ejecutando){
        RevisarSuspendidos(&lista_suspendidos,&lista_listos);

        huboError = 0; //reiniciamos a cada interacion la bandera de errores
        //FILE *file;

        if ((lista_ejecucion == NULL && lista_listos == NULL)){
            char entrada[200];
            char extra[100];
            char extra2[100];
            comando[0] = '\0';
            archivo[0] = '\0';

            move(y_linea_comando, 0); clrtoeol();
            mvprintw(y_linea_comando, 0, "> ");
            refresh();

            getnstr(entrada, 199); //lee la entrada
            num_palabras = sscanf(entrada, "%99s %99s %99s %99s", comando, archivo, extra, extra2); //sscanf(cadena, formato, &variable1, etc.);
            move(y_linea_comando, 0); clrtoeol();
            refresh();
            if (strcmp(comando, "salir") == 0){
                if (num_palabras > 1){
                    move(y_mensajes, 0); clrtoeol();
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

                
               // file = fopen(archivo, "r");
               

              /*  if (file == NULL){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "No se pudo abrir el archivo %s", archivo);
                    refresh();
                    continue;
                }*/

               // FILE *swap = fopen(ArchivoBinario, "rb");
                pid++;
                gid++;
                grupos++;
                //primero pasar a nuevos

                int ContadorL =  ContadorLineas(archivo);
                if(ContadorL < 131072){
                    insertar(&lista_nuevos, pid, gid, archivo, 0);
                    struct Nodo *nuevo = buscar(lista_nuevos, pid);
                    if (nuevo == NULL) {
                        mvprintw(y_mensajes, 0, "ERROR: No se pudo crear el proceso");
                        refresh();
                        continue;
                    }
                    //ver si se peude cargar a swap
                    if (reescritura(archivo, swap, pid, TMS, nuevo->TMP) == 0) {
                        struct Nodo *p = extraerNodo(&lista_nuevos, pid); //pasamos a listos si todo bien
                        if (p != NULL) {
                            insertarFinal(&lista_listos, p);
                        }
                    } else {//no cupo
                        mvprintw(y_mensajes, 0, "Proceso %d queda en nuevos: no hay espacio en swap", pid);
                        refresh();
                    }

                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
                    refresh();    
                }else{
                mvprintw(y_mensajes,0,"ERROR: El proceso es mas grande que el swap.");
                refresh();
                continue;
                }
                
                

            } else if (strcmp(comando, "mata") == 0){
                if(lista_listos == NULL && lista_ejecucion == NULL && lista_terminados == NULL){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: no hay procesos que matar");
                    refresh();
                    num_palabras= 0;
                    continue;
                }
            }else if(strcmp(comando, "fork") == 0){
                if(lista_listos == NULL && lista_ejecucion == NULL && lista_terminados == NULL){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: no hay procesos para duplicar");
                    refresh();
                    num_palabras= 0;
                    continue;
                }
            }else if (strcmp(comando, "velocidad") == 0){
                if (num_palabras < 2) {
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: falta los milisegundos");
                    move(y_linea_comando, 0); clrtoeol();
                    refresh();
                    comando[0] = '\0';
                    num_PID = '\0';
                    num_palabras = 0;
                    continue;
                }
                if(Negativo(archivo) == 1){
                    mvprintw(y_mensajes,0, "Error: No se pueden milisegundos negativos");
                    continue;
                }
                ms = atoi(archivo);
                if(!num_PID){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: velocidad debe ser un entero positivo");
                    move(y_linea_comando, 0); clrtoeol();
                    refresh();
                    continue;
                }
                if (num_palabras > 2){
                    move(y_mensajes, 0); clrtoeol();
                    mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                    move(y_linea_comando, 0); clrtoeol();
                    refresh();
                    comando[0] = '\0';
                    num_PID = '\0';
                    num_palabras = 0;
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
            struct Nodo *proceso = Fair_Share(&lista_listos,grupos,Base);
            if(proceso != NULL){
                proceso -> Status = 'E';
                insertarFinal(&lista_ejecucion,proceso);
            }
            //mvprintw(y_variable,0,"numero de grupos:%d",grupos);
            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
            imprimir_TMP(proceso->TMP, y_tablaR, proceso->PID);
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
        int gcpu_acum=0; //varible que le pasamos para que al terminar quantum(o termine) para acrualizar el GCPU del grupo
        int pagina = 0;
        mvprintw(y_header, 0, "%-10s %-18s %10s %10s %10s %10s %10s %10s ", "PC", "IR", "EAX", "EBX", "ECX", "EDX", "CPU","GCPU");//(y,x,"fotmato",variables) -(alinear a la izquierda)10(espacios para esa variable)formato de variable
        mvprintw(y_tabla, 0, "%-5s %5s %5s %5s", "Pagi", "Bit", "M_R", "M_S");
        mvprintw(y_header2, 0, "%-5s %-5s %-8s %-8s %-18s %-18s %-10s %-18s %10s %10s %10s %10s %10s", "PID","GID", "CPU","GCPU", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX","Prioridad");
        refresh();
        int desplazamiento =0;

        if(procesoEjecucion != NULL){
            int finArchivo = 0;

            while (q < quantum) {

                int direccion_virtual = procesoEjecucion->PC;
                pagina = direccion_virtual / 4;
                desplazamiento = direccion_virtual % 4;

                if(BitPresencia_TMP(procesoEjecucion->TMP,pagina)==0){
                    if(swap == NULL){
                        mvprintw(y_mensajes,0,"Error abriendo swap");
                        refresh();
                        break;
                    }
                    if(RAMLlena(TMM) == 0){
                        EscrituraRam(swap,RAM,pagina,procesoEjecucion->TMP,TMM,procesoEjecucion->PID);
                    }else{//RAM llena
                        //mvprintw(y_mensajes,0,"ERROR: Esta llena la RAM");
                        //refresh();

                        struct Nodo *procesoSuspendido=extraerNodo(&lista_ejecucion,procesoEjecucion->PID);
                        if(procesoSuspendido != NULL){
                            TiempoEnSuspendidos(procesoSuspendido);
                            procesoSuspendido->PC = contadorLinea;
                            strcpy(procesoSuspendido->IR, linea);
                            insertarFinal(&lista_suspendidos,procesoSuspendido);
                            AlgoritmoReloj(TMM,RAM);
                            EscrituraRam(swap,RAM,pagina,procesoSuspendido->TMP,TMM,procesoSuspendido->PID);
                        } 
                        
                        imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
                        //continue;
                        //NOTA: fallo de pagina
                        break;
                    }

                    //imprimir_TMP(TMP,y_tablaR,procesoEjecucion->PID);
                }

                int MarcoRAM = procesoEjecucion->TMP[pagina][1];
                //mvprintw(4,0,"LINEA antes del mem");
                //refresh();
                memcpy(linea,&RAM[MarcoRAM][desplazamiento * 100],100);
                //mvprintw(6,0,"%s",linea);
                refresh();
                //mvprintw(4,0,"LINEA despues del mem");
                //refresh();
                
                q++;  
                contadorLinea++;
                procesoEjecucion->CPU+=20;
                procesoEjecucion->GCPU+=20;
                gcpu_acum=procesoEjecucion->GCPU;
                
                char linea_original[100]; //gaurdamos copia de lalinea
                strcpy(linea_original, linea);
                linea_original[strcspn(linea_original, "\r\n")] = '\0';//(lineaaescanear, loquevaaencontrar)
                //mvprintw(4,0,"LINEA despues del mem284");
                //refresh();
                if(linea_original[0] == '\0'){ //linea vacia
                    move(y_mensajes,0); clrtoeol();
                    mvprintw(y_mensajes,0,"ERROR: linea vacia en linea %d", contadorLinea);
                    refresh();
                    //mvprintw(4,0,"LINEA despues del mem290");
                    //refresh();
                    strcpy(procesoEjecucion->IR, linea_original); //guardamos el IR
                    A_terminadosError(&lista_ejecucion,&lista_terminados);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
                    if(Busqueda_GID(&lista_listos,&lista_ejecucion,procesoEjecucion->GID)==0){//revisar si todavia hay procesos con ese GID
                        grupos--;
                    }
                    //mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                    huboError = 1;
                    comando[0] = '\0';
                    archivo[0] = '\0';
                    //mvprintw(4,0,"LINEA 271");
                    //refresh();
                    break;
                    //mvprintw(4,0,"LINEA 274");
                    //refresh();
                }

                token = strtok(linea, " \n\t ,");
                if (token == NULL){
                    continue;
                }

                instruccion = token;

                arg1 = strtok(NULL, " \n\t ,");
                arg2 = strtok(NULL, " \n\t ,");
                //mvprintw(4,0,"LINEA despues del mem316");
                //refresh();
                imprimirEstado(lista_listos,lista_ejecucion,lista_terminados,lista_suspendidos);
                // Sintaxis para los espacios y Verifica si la instruccion es valida
                if (!validarEspacios(linea_original, instruccion, contadorLinea)
                    || !Operaciones(instruccion, contadorLinea, linea_original)){
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion,&lista_terminados);
                    if(Busqueda_GID(&lista_listos,&lista_ejecucion,procesoEjecucion->GID)==0){
                        grupos--;
                    }
                    //mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
                    huboError = 1;
                    reiniciarVariables(comando,archivo);
                    break;
                }
                //mvprintw(4,0,"LINEA despues del mem331");
                //refresh();
                if ((strcmp(instruccion, "MOV") == 0 && !MOV(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "ADD") == 0 && !ADD(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "SUB") == 0 && !SUB(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "MUL") == 0 && !MUL(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "DIV") == 0 && !DIV(arg1,arg2,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "INC") == 0 && !INC(arg1,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "DEC") == 0 && !DEC(arg1,contadorLinea,linea_original,procesoEjecucion)) ||
                    (strcmp(instruccion, "JNZ") == 0 && !JNZ(arg1,contadorLinea,linea_original,procesoEjecucion))) {
                    strcpy(procesoEjecucion->IR, linea_original);
                    A_terminadosError(&lista_ejecucion,&lista_terminados);
                    if(Busqueda_GID(&lista_listos,&lista_ejecucion,procesoEjecucion->GID)==0){
                        grupos--;
                    }
                    //mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                    imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
                    huboError = 1;
                    reiniciarVariables(comando,archivo);
                    break;
                }else if (strcmp(instruccion, "JNZ") == 0) {
                    contadorLinea = procesoEjecucion->PC;
                }
      
                    
                 else if ((strcmp(instruccion, "END") == 0)){
                    encontroEND = 1;
                        
                    procesoEjecucion->PC = contadorLinea; //por que hace break y no se guardaria el END
                    strcpy(procesoEjecucion->IR, linea_original);
                        
                            move(y_renglon, 0); clrtoeol();
                            mvprintw(y_renglon, 0, "%-10d %-18s %10d %10d %10d %10d", contadorLinea, linea_original, procesoEjecucion->EAX, procesoEjecucion->EBX,procesoEjecucion->ECX,procesoEjecucion->EDX);
                            refresh();
                            ComandoVel(ms);
                            
                            struct Nodo *procesoTerminado = extraerPrimero(&lista_ejecucion); 

                            if (procesoTerminado != NULL) {
                                int gid_terminado = procesoTerminado->GID;

                                insertarFinal(&lista_terminados, procesoTerminado);

                                if (Busqueda_GID(&lista_listos, &lista_ejecucion, gid_terminado) == 0) {
                                    grupos--;
                                }
                            }

                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);                             
                            reiniciarVariables(comando,archivo);
                            break;
                }
                procesoEjecucion->PC = contadorLinea; //guardamos la PC y IR ejecutado
                strcpy(procesoEjecucion->IR, linea_original);
                
                refresh();
                ComandoVel(ms); //Tiempo para ver las lineas de impresion para renglon
                    
                    if (kbhit()){
                        //imprimirlista(procesoEjecucion, y_procesoEjecucion);
                        
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
                            
                            pid++;
                            gid++;
                            grupos++;
                            //primero pasar a nuevos
                            insertar(&lista_nuevos, pid, gid, archivo, 0);
                            struct Nodo *nuevoP = buscar(lista_nuevos, pid);
                            if (nuevoP == NULL) {
                                mvprintw(y_mensajes, 0, "(D)ERROR: No se pudo crear el proceso");
                                refresh();
                                continue;
                            }
                            //ver si se peude cargar a swap
                            if (reescritura(archivo, swap, pid, TMS, nuevoP->TMP) == 0) {
                                struct Nodo *pn = extraerNodo(&lista_nuevos, pid); //pasamos a listos si todo bien
                                if (pn != NULL) {
                                    insertarFinal(&lista_listos, pn);
                                }
                            } else {//no cupo
                                mvprintw(y_mensajes, 0, "Proceso %d queda en nuevos: no hay espacio en swap", pid);
                                refresh();
                            }
                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
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
                            int gid_matado = -1;
                            struct Nodo *p_matar = buscar(lista_ejecucion, num_PID);

                            if (p_matar == NULL) {
                                p_matar = buscar(lista_listos, num_PID);
                            }
                            if (p_matar != NULL) {
                                gid_matado = p_matar->GID;
                            }

                            int lista = matar(&lista_ejecucion, &lista_terminados, &lista_listos, num_PID);

                            if (gid_matado != -1) {
                                if (Busqueda_GID(&lista_listos, &lista_ejecucion, gid_matado) == 0) {
                                    grupos--;
                                }
                            }
                            //mvprintw(y_variable,0,"numero de grupos:%d",grupos);
                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
                            move(y_linea_comando, 0); clrtoeol();
                            refresh();
                            num_palabras = 0;
                            if(lista == 1){ //1 -> esta en lista ejecucion, 2-> listos, 3 -> terminados, 0->no esta el PID
                                break;
                            }
                            
                            continue;
                            
                        }else if(strcmp(comando, "fork") == 0){
                            if (num_palabras < 2) {
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: falta pid del proceso");
                                refresh();
                                num_palabras= 0;
                                continue;
                            }
                            if (num_palabras < 3) {
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: falta numero de instruccion");
                                refresh();
                                num_palabras= 0;
                                continue;
                            }
                            if(Negativo(extra) == 1){
                                mvprintw(y_mensajes,0, "Error: PC no puede ser negativo");
                                refresh();
                                continue;
                            }
                            num_PID = atoi(archivo);
                            num_PC = atoi(extra);
                            if(!num_PID){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: PID debe ser un entero");
                                refresh();
                                num_PID = '\0';
                                continue;
                            }
                            if(!num_PC){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: PC debe ser un entero");
                                refresh();
                                num_PC = '\0';
                                continue;
                            }
                            if (num_palabras > 3) {
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
                                refresh();
                                num_palabras= 0;
                                num_PID = '\0';
                                num_PC = '\0';
                                continue;
                            }                                                                           
                            pid++;
                                                                                                        //num_PID num_PC son las variables que estan en el comando
                            struct Nodo *nuevo = forkProcesoComando(&lista_ejecucion,&lista_terminados,&lista_listos,num_PID,num_PC,pid);
                            
                            if (nuevo == NULL) {
                                pid--;
                            }

                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
                            move(y_linea_comando, 0); clrtoeol();
                            refresh();
                        }else if (strcmp(comando, "velocidad") == 0){
                            if(num_palabras < 2) {
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "ERROR: falta los milisegundos");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                comando[0] = '\0';
                                num_PID = '\0';
                                num_palabras = 0;
                                continue;
                            }
                            if(Negativo(archivo) == 1){
                                mvprintw(y_mensajes,0, "Error: No se pueden milisegundos negativos");
                                refresh();
                                continue;
                            }
                            ms = atoi(archivo);
                            if(!num_PID){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "ERROR: velocidad debe ser un entero positivo");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                continue;
                            }
                            if (num_palabras > 2){
                                move(y_mensajes, 0); clrtoeol();
                                mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                                move(y_linea_comando, 0); clrtoeol();
                                refresh();
                                comando[0] = '\0';
                                num_PID = '\0';
                                num_palabras = 0;
                                continue;
                            }
            
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
                    insertarFinal(&lista_listos,p);
                }
                GCPU_Global(&lista_listos,procesoEjecucion->GID,gcpu_acum);
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);
            }
            else if (encontroEND == 0 && huboError == 0 && finArchivo == 1){ //se acbo el archivo sin END
                move(y_mensajes, 0); clrtoeol();
                mvprintw(y_mensajes, 0, "ERROR: Fin de archivo sin END");
                refresh();
               
                A_terminadosError(&lista_ejecucion,&lista_terminados);
                if(Busqueda_GID(&lista_listos,&lista_ejecucion,procesoEjecucion->GID)==0){
                    grupos--;
                }
                imprimirEstado(lista_listos, lista_ejecucion, lista_terminados, lista_suspendidos);                               
                reiniciarVariables(comando,archivo);
                continue;
            }
        } 
    }
    endwin();
}