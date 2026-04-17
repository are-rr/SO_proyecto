#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <ncurses.h>
#include <curses.h>
//estrucutra para las listas
struct Nodo {
    int PID;       // identificador unico
    FILE* Archivo;//nombre del archivo, Guardar el puntero al archivo FILE *
    char nombrePro[100]; //para el nombre del archivo
    int EAX; //Registros
    int EBX;
    int ECX;
    int EDX;
    char Status;     // L = listo E= ejecucion  T =terminado X=Terminado-Error Z=Terminado-mata
    int PC;    // contador de programa(contadorLInea)
    char IR[100];//para guardar la ultima instruccion
    struct Nodo *sig;  // puntero al siguiente nodo
};

int Registro(char *token);
int Operaciones(char *token, int contadorLinea, const char *linea_original);
int Digito(char *token);
int *ObtenerRegistro(char *nombre, struct Nodo *p);
int MOV(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int ADD(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int SUB(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int MUL(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int DIV(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int INC(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int DEC(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int filtroIncDec(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int filtro(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
void salirPrograma();
int Comas_2pam(const char *linea_original, int contadorLinea);
int Comas_1pam(const char *linea_original, int contadorLinea);
int kbhit(void);
int validarEspacios(const char *linea_original, char *instruccion, int contadorLinea);
void reiniciarVariables(char *comando,char *archivo);
void insertar(struct Nodo **cabeza, int pid,FILE *archivo,const char *nombre,char status, int pc);
void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso);
struct Nodo *extraerPrimero(struct Nodo **cabeza);
void imprimirlista(struct Nodo *lista, int y_ncurses);
void imprimirEstado(struct Nodo *listo, struct Nodo *ejecucion, struct Nodo *terminados);
const char *statusTexto(char status);
void imprimirProceso(struct Nodo *p,int y_ncurse);
void A_terminadosError(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados);
struct Nodo* extraerNodo(struct Nodo **lista, int id);
int matar(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, int id_p);

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
        huboError = 0; //reiniciamos a cada interacion los errores encontrados

        if ((lista_ejecucion == NULL && lista_listos == NULL)){
            char entrada[200];
            char extra[100];
            comando[0] = '\0';
            archivo[0] = '\0';

            move(y_linea_comando, 0); clrtoeol();
            mvprintw(y_linea_comando, 0, "> ");
            refresh();

            getnstr(entrada, 199);
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
                insertar(&lista_listos,pid,file,archivo,'L',0); 
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
                num_PID = atoi(archivo);
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
        if(lista_ejecucion == NULL){
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

        mvprintw(y_header, 0, "%-10s %-20s %10s %10s %10s %10s", "PC", "IR", "EAX", "EBX", "ECX", "EDX");
        mvprintw(y_header2, 0, "%-5s %-20s %-18s %-10s %-20s %10s %10s %10s %10s", "PID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX");
        refresh();
        
        if(procesoEjecucion != NULL){
            int finArchivo = 0;
            while (q < quantum) {
                if (fgets(linea, sizeof(linea), procesoEjecucion->Archivo) == NULL) {
                    finArchivo = 1;
                    break;
                }
                q++;  
                contadorLinea++;
                
                
                char linea_original[100];
                strcpy(linea_original, linea);
                linea_original[strcspn(linea_original, "\r\n")] = '\0';//(lineaaescanear, loquevaaencontrar)

                if(linea_original[0] == '\0'){
                    move(y_mensajes,0); clrtoeol();
                    mvprintw(y_mensajes,0,"ERROR: linea vacia en linea %d", contadorLinea);
                    refresh();
                    strcpy(procesoEjecucion->IR, linea_original);
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
                            napms(1000);
                            
                            struct Nodo *procesoTerminado = extraerPrimero(&lista_ejecucion); 
                            if(procesoTerminado != NULL){
                                procesoTerminado -> Status = 'T';
                                insertarFinal(&lista_terminados,procesoTerminado);
                                fclose(procesoTerminado->Archivo);
                                procesoTerminado->Archivo = NULL;
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
                procesoEjecucion->PC = contadorLinea;
                strcpy(procesoEjecucion->IR, linea_original);
                refresh();
                napms(1000); //Tiempo para ver las lineas de impresion para renglon
                    
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
            procesoEjecucion->PC = contadorLinea;
            
            if(q==quantum && encontroEND == 0 && huboError == 0){
                struct Nodo *p = extraerPrimero(&lista_ejecucion);
                if(p != NULL){
                    p -> Status = 'L';
                    insertarFinal(&lista_listos,p);
                }
                imprimirEstado(lista_listos,lista_ejecucion,lista_terminados);
            }
            else if (encontroEND == 0 && huboError == 0 && feof(procesoEjecucion->Archivo)){
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
// 'M'=MOV, 'A'=ADD, 'S'=SUB, 'U'=MUL, 'D'=DIV
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


// ** almacena la dirección de memoria de otro puntero, debido a que la variable file es un puntero y queremos la direccion del puntero
void reiniciarVariables(char *comando,char *archivo){
    comando[0] = '\0';
    archivo[0] = '\0';
}

void salirPrograma(){
    endwin();
    exit(0);
}

int Comas_2pam(const char *linea_original, int contadorLinea){
    int contadorComa = 0;

    for (int i = 0; linea_original[i] != '\0'; i++){
        if (linea_original[i] == ',')
            contadorComa++;
    }
    const char *coma = strchr(linea_original, ',');

    if (contadorComa > 1){
        move(y_mensajes, 0); clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR mas de una coma en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (contadorComa == 0){
        move(y_mensajes, 0); clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR falta la coma en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (*(coma - 1) == ' '){
        move(y_mensajes, 0); clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR espacio antes de coma linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (*(coma + 1) == ' '){
        move(y_mensajes, 0); clrtoeol();
        refresh();
        mvprintw(y_mensajes, 0, "ERROR espacio despues de coma linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int Comas_1pam(const char *linea_original, int contadorLinea){
    int contadorComa = 0;

    for (int i = 0; linea_original[i] != '\0'; i++){
        if (linea_original[i] == ',')
            contadorComa++;
    }

    if (contadorComa >= 1){
        move(y_mensajes, 0); clrtoeol();refresh();
        mvprintw(y_mensajes, 0, "ERROR: No debe tener coma en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int filtro(char *arg1, char *arg2, int contadorLinea, const char *linea_original){
    if (arg1 == NULL || arg2 == NULL){
        move(y_mensajes, 0); clrtoeol(); refresh();
        mvprintw(y_mensajes, 0, "ERROR: Faltan argumentos en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    if (!Registro(arg1)){
        move(y_mensajes, 0); clrtoeol(); refresh();
        mvprintw(y_mensajes, 0, "ERROR: El primer argumento debe ser un registro valido en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int filtroIncDec(char *arg1, char *arg2, int contadorLinea, const char *linea_original){
    if (arg1 == NULL){
        move(y_mensajes, 0); clrtoeol();refresh();
        mvprintw(y_mensajes, 0, "ERROR: No hay argumento en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    else if (arg2 != NULL){
        move(y_mensajes, 0); clrtoeol();refresh();
        mvprintw(y_mensajes, 0, "ERROR INC/DEC solo debe tener un argumento en linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }
    return 1;
}

int Registro(char *token){
    char Registros[4][10] = {"EAX", "EBX", "ECX", "EDX"};
    for (int i = 0; i < 4; i++){
        if (strcmp(token, Registros[i]) == 0){
            return 1;
        }
    }
    return 0;
}

int Operaciones(char *token, int contadorLinea, const char *linea_original){
    char Instrucciones[8][10] = {"MOV", "ADD", "SUB", "MUL", "DIV", "INC", "DEC", "END"};
    for (int i = 0; i < 8; i++){
        if (strcmp(token, Instrucciones[i]) == 0){
            return 1;
        }
    }
    move(y_mensajes, 0); clrtoeol();refresh();
    mvprintw(y_mensajes, 0, "ERROR: Instruccion no reconocida en linea %d:\"%s\"", contadorLinea, linea_original);
    return 0;
}

int Digito(char *token){
    int i = 0;
    if (token[0] == '\0')
        return 0;
    if (token[0] == '-'){
        i = 1;
    }
    for (; token[i] != '\0' && token[i] != '\n'; i++){
        if (!isdigit(token[i])){
            return 0;
        }
    }
    return 1;
}

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

int kbhit(void){
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&read_fd);
    FD_SET(0, &read_fd); // 0 = STDIN

    if (select(1, &read_fd, NULL, NULL, &tv) == -1)
        return 0;
    if (FD_ISSET(0, &read_fd))
        return 1;

    return 0;
}

int validarEspacios(const char *linea_original, char *instruccion, int contadorLinea){
    int i = 0;
    if (!Operaciones(instruccion, contadorLinea, linea_original)){
        return 0;
    }

    // Aqui es para mostrar error si encuentra espacios y tabulaciones al inicio de la instruccion
    if (linea_original[0] == ' ' || linea_original[0] == '\t'){
        move(y_mensajes, 0);
        clrtoeol();refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\", no debe iniciar con espacios/tabs", contadorLinea, linea_original);
        return 0;
    }
    i += 3; // por que las instrucciones tienen 3 letras
    // En caso de END
    if (strcmp(instruccion, "END") == 0){
        if (linea_original[i] != '\0'){
            move(y_mensajes, 0);
            clrtoeol();refresh();
            mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d: %s, no se permiten espacios al final", contadorLinea, instruccion);
            return 0;
        }
        return 1;
    }

    if (linea_original[i] != ' '){
        move(y_mensajes, 0); clrtoeol();refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d: \"%s\" debe haber 1 espacio despues de %s", contadorLinea, linea_original, instruccion);
        return 0;
    }
    if (linea_original[i + 1] == ' '){
        move(y_mensajes, 0); clrtoeol();refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\" hay mas de 1 espacio despues de %s", contadorLinea, linea_original, instruccion);
        return 0;
    }
    i++; // validar espacio
    // arg1
    while (linea_original[i] && linea_original[i] != ',' && linea_original[i] != ' '){
        i++;
    }
    // en caso de INC y DEC
    if (strcmp(instruccion, "INC") == 0 || strcmp(instruccion, "DEC") == 0){
        if (linea_original[i] != '\0'){
            move(y_mensajes, 0); clrtoeol();refresh();
            mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\" %s solo lleva 1 argumento y sin espacios extra", contadorLinea, linea_original, instruccion);
            return 0;
        }
        return 1;
    }
    i++; // para saltar la coma
    const char *espacio = strchr(linea_original, ' ');
    int contadorEspacio = 0;

    for (int i = 0; linea_original[i] != '\0'; i++){
        if (linea_original[i] == ' ')
            contadorEspacio++;
    }
    if (contadorEspacio > 1){
        move(y_mensajes, 0); clrtoeol();refresh();
        mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\"", contadorLinea, linea_original);
        return 0;
    }

    for (int j = 0; linea_original[j] != '\0'; j++){
        if (linea_original[j] == ' ' && linea_original[j + 1] == '\0'){
            move(y_mensajes, 0); clrtoeol();refresh();
            mvprintw(y_mensajes, 0, "ERROR sintaxis linea %d:\"%s\", no se permiten espacios al final", contadorLinea, linea_original);
            return 0;
        }
    }

    return 1;
}
//Funciones para las listas:
// crea e Inserta el proceso al final de una lista
void insertar(struct Nodo **cabeza, int pid,FILE *archivo,const char *nombre,char status, int pc) {
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo)); //reservar memoria
    
    nuevo->PID = pid;
    strncpy(nuevo->nombrePro, nombre, sizeof(nuevo->nombrePro) - 1);
    nuevo->nombrePro[sizeof(nuevo->nombrePro) - 1] = '\0';
    nuevo-> Archivo = archivo;
    nuevo->Status = status;
    nuevo->PC = pc;

    nuevo->EAX = 0;
    nuevo->EBX = 0;
    nuevo->ECX = 0;
    nuevo->EDX = 0;
    nuevo->IR[0] = '\0';

    nuevo->sig = NULL;

    if (*cabeza == NULL) {
        *cabeza = nuevo; //si la lista esta vacia
        return;
    }

    struct Nodo *temp = *cabeza;
    while (temp->sig != NULL) { //noo esta vacia, recorre hasta el final
        temp = temp->sig;
    }
    temp->sig = nuevo;//inserta el nuevo nodo al final
}
// inserta el proceso al final
void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso) {
    if (proceso == NULL) return;

    proceso->sig = NULL;

    if (*cabeza == NULL) {
        *cabeza = proceso;
        return;
    }

    struct Nodo *temp = *cabeza;
    while (temp->sig != NULL) {
        temp = temp->sig;
    }
    temp->sig = proceso;
}

// saca el primer proceso de la lista
struct Nodo *extraerPrimero(struct Nodo **cabeza) {
    if (*cabeza == NULL) {
        return NULL;
    }
    struct Nodo *temp = *cabeza; //guarda el primer nodo
    *cabeza = (*cabeza)->sig; //mueve la cabeza al siguiente
    temp->sig = NULL; //desconecta el nodo
    return temp;
}
int contarNodos(struct Nodo *lista){
    int num =0;
    while(lista != NULL){
        num++;
        lista = lista->sig;
    }
    return num;
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
const char *statusTexto(char status){
    switch(status){
        case 'L' : return "Listos";
        case 'E' : return "Ejecucion";
        case 'T' : return "Terminados";
        case 'X' : return "Terminados-Error";
        case 'Z' : return "Terminados-Mata";
    }
}

void imprimirProceso(struct Nodo *p,int y_ncurse){
    //mvprintw(y_header2, 0, "%-5s %-20s %-18s %-10s %-20s %10s %10s %10s %10s", "PID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX");
    if(p->Status == 'E'){
        mvprintw(y_ncurse,0,"%-5d %-20s %-18s %-10d %-20s %10d %10d %10d %10d",
            p->PID,p->nombrePro,statusTexto(p->Status),p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX);
    }
    else if(p->Status == 'L'){
        mvprintw(y_ncurse,0,"%-5d %-20s %-18s %-10d %-20s %10d %10d %10d %10d",
            p->PID,p->nombrePro,statusTexto(p->Status),p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX);
    }
    else if(p->Status == 'T' || p-> Status == 'X' || p-> Status == 'Z'){
        mvprintw(y_ncurse,0,"%-5d %-20s %-18s %-10d %-20s %10d %10d %10d %10d",
            p->PID,p->nombrePro,statusTexto(p->Status),p->PC,p->IR,p->EAX,p->EBX,p->ECX,p->EDX);
    }
}

void A_terminadosError(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados){
    struct Nodo *procesoError = extraerPrimero(lista_ejecucion);
    if(procesoError != NULL){
        procesoError->Status = 'X';
        insertarFinal(lista_terminados, procesoError);
        fclose(procesoError -> Archivo);
        procesoError-> Archivo = NULL;
    }
}

int matar(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, int id_p) {
    struct Nodo *proceso_mata = NULL;

    proceso_mata = extraerNodo(lista_ejecucion, id_p);
    if(proceso_mata != NULL){
        proceso_mata -> Status = 'Z';
        fclose(proceso_mata->Archivo);
        proceso_mata->Archivo = NULL;
        insertarFinal(lista_terminados, proceso_mata);
        return 1;
    }

    if (proceso_mata == NULL) {
        proceso_mata = extraerNodo(lista_listos, id_p);
        if(proceso_mata != NULL){
            proceso_mata -> Status = 'Z';
            fclose(proceso_mata->Archivo);
            proceso_mata->Archivo = NULL;
            insertarFinal(lista_terminados, proceso_mata);
            return 2;
        }
    }

    if (proceso_mata == NULL) {
        struct Nodo *aux = *lista_terminados;
        while (aux != NULL) {
            if (aux->PID == id_p) {
                move(y_mensajes, 0); clrtoeol();
                mvprintw(y_mensajes,0,"El proceso con PID %d ya esta en terminados.", id_p);
                refresh();
                return 3;
            }
            aux = aux->sig;
        }
        move(y_mensajes, 0); clrtoeol();
        mvprintw(y_mensajes,0,"No se encontro el proceso con PID %d.", id_p);
        refresh();
        return 0;
    }
}

struct Nodo* extraerNodo(struct Nodo **lista, int id) {
    struct Nodo *actual = *lista;
    struct Nodo *anterior = NULL;

    while (actual != NULL) {
        if (actual->PID == id) {
            if (anterior == NULL) { //si es el primer nodo en la lista, no tiene anterior
                *lista = actual->sig;
            } else {//desconectamos el nodo
                anterior->sig = actual->sig; //nodo anterior apunta al sig del actual
            }

            actual->sig = NULL;// desenlazamos el nodo de la lista
            return actual;
        }

        anterior = actual; 
        actual = actual->sig;
    }

    return NULL; // No encontrado
}