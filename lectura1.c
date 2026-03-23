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
    FILE* Archivo;//nombre del archivo
    char nombrePro[100]; //para el nombre del archivo
    //Guardar el puntero al archivo FILE *, con eso ya no tendriamos que saltarnos los renglones 
    //comando???
    int EAX;
    int EBX;
    int ECX;
    int EDX;
    char Status;     // L = listo E= ejecucion  T =terminado
    int PC;    // contador de programa(contadorLInea)
    struct Nodo *sig;  // puntero al siguiente nodo
};

int Registro(char *token);
int Operaciones(char *token, int contadorLinea, const char *linea_original);
int Digito(char *token);
int *ObtenerRegistro(char *nombre);
int MOV(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int ADD(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int SUB(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int MUL(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int DIV(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int INC(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int DEC(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int filtroIncDec(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int filtro(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
void salirPrograma();
int Comas_2pam(const char *linea_original, int contadorLinea);
int Comas_1pam(const char *linea_original, int contadorLinea);
int kbhit(void);
int validarEspacios(const char *linea_original, char *instruccion, int contadorLinea);
void reiniciarVariables_cerrar(char *comando,char *archivo, int *EAX, int *EBX, int *ECX, int *EDX);
void insertar(struct Nodo **cabeza, int pid,FILE *archivo,const char *nombre,char status, int pc);
void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso);
struct Nodo *extraerPrimero(struct Nodo **cabeza);
void imprimirlista(struct Nodo *lista, int y_ncurses);
void imprimirEstado(struct Nodo *listo, struct Nodo *ejecucion, struct Nodo *terminados);

//Registros
int EAX = 0;
int EBX = 0;
int ECX = 0;
int EDX = 0;
// cordenadas de fila
int y_header = 0;
int y_renglon = 1;
int y_mensajes = 3;
int y_linea_comando = 5;
int y_header2 = 7;
int y_procesoEjecucion = 8;
int y_procesosListos = 9;
int y_procesoTerminado = 10;

int ejecutando = 1;
int pid =0;
FILE *salida;

int main(){
    char comando[100];
    char archivo[100];
    int num_palabras;
    initscr();
    comando[0] = '\0';
    archivo[0] = '\0';
    struct Nodo *lista_listos = NULL;
    struct Nodo *lista_ejecucion = NULL;
    struct Nodo *lista_terminados = NULL;

    while (ejecutando){
    comando[0] = '\0';
    archivo[0] = '\0';
      

        if ((lista_ejecucion == NULL && lista_listos == NULL)){
            char entrada[200];
            char extra[100];
            move(y_linea_comando, 0); clrtoeol();refresh();
            mvprintw(y_linea_comando, 0, "> ");
            getnstr(entrada, 199);
            num_palabras = sscanf(entrada, "%99s %99s %99s", comando, archivo, extra); //sscanf(cadena, formato, &variable1, etc.);
        }

        if (strcmp(comando, "Salir") == 0){
            if (num_palabras > 1){
                move(y_mensajes, 0); clrtoeol();
                refresh();
                mvprintw(y_mensajes, 0, "ERROR: comando invalido");
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                num_palabras = 0;
                continue;
            }
            salirPrograma();
        }

        else if (strcmp(comando, "Ejecuta") == 0){
            if (num_palabras < 2) {
                move(y_mensajes, 0); clrtoeol();
                refresh();
                mvprintw(y_mensajes, 0, "ERROR: falta el nombre del archivo");
                comando[0] = '\0';
                archivo[0] = '\0';
                refresh();
                continue;
            }
            else if (num_palabras > 2){
                move(y_mensajes, 0); clrtoeol();
                refresh();
                mvprintw(y_mensajes, 0, "ERROR: demasiados argumentos");
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                num_palabras = 0;
                continue;
            }

            FILE *file = fopen(archivo, "r");
            
            salida = fopen("salida.txt","w");


            if (file == NULL){
                move(y_mensajes, 0); clrtoeol();
                mvprintw(y_mensajes, 0, "No se pudo abrir el archivo %s", archivo);
                refresh();
                comando[0] = '\0';
                archivo[0] = '\0';
                continue;
            }
            pid++;
            
            insertar(&lista_listos,pid,file,archivo,'L',0); 
            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
            refresh();            
            fprintf(salida,"%d %s %-c %d\n", lista_listos->PID, lista_listos->nombrePro,lista_listos->Status, lista_listos->PC);

        }
        else if((strcmp(comando, "Ejecuta") != 0)&& (strcmp(comando, "Salir") != 0)){
            move(y_mensajes, 0); clrtoeol();
            refresh();
            mvprintw(y_mensajes, 0, "Comando no valido");
            continue;
        }
                   

        
            char linea[100];
            int contadorLinea = 0;
            char *token, *arg1, *arg2, *instruccion;

            mvprintw(y_header, 0, "%-5s %-20s %8s %8s %8s %8s", "PC", "IR", "EAX", "EBX", "ECX", "EDX");
            refresh();
            int encontroEND = 0; //Variable para ver casos de la instruccion END(si hay en el documento)
            int cerrado = 0; //Variable para indicar si el archivo se cerro o sigue abierto
            mvprintw(y_header2, 0, "%-5s %-20s %-12s %-5s %-20s %8s %8s %8s %8s", "PID", "Nombre", "Status","PC", "IR","EAX", "EBX", "ECX", "EDX");
            refresh();
            
            //Se pasa a lista de Ejecucion ---------------------------------------------------------------------------------------------
            if(lista_ejecucion == NULL){
                struct Nodo *proceso = extraerPrimero(&lista_listos); //+++++++++++++++++++++++ puntero a proceso????
                if(proceso != NULL){
                    proceso -> Status = 'E';
                    insertarFinal(&lista_ejecucion,proceso);
                }
            }
            

            //imprimirEstado(lista_listos,lista_ejecucion,lista_terminados);
            //fprintf(salida,"%d %s %-c %d\n", lista_ejecucion->PID, lista_ejecucion->nombrePro,lista_ejecucion->Status, lista_ejecucion->PC);
imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
refresh();            //imprimirlista(lista_listos);

struct Nodo *procesoEjecucion = lista_ejecucion;
                if(procesoEjecucion != NULL){
                while (fgets(linea, sizeof(linea), procesoEjecucion -> Archivo) != NULL){ //(loquelee, maximocaracteres,archivodedondelee)
                    contadorLinea++;

                    char linea_original[100];
                    strcpy(linea_original, linea);
                    linea_original[strcspn(linea_original, "\r\n")] = '\0';//(lineaaescanear, loquevaaencontrar)

                    if(linea_original[0] == '\0'){
                        move(y_mensajes,0); clrtoeol();
                        mvprintw(y_mensajes,0,
                        "ERROR: linea vacia en linea %d", contadorLinea);
                        refresh();

                        //fclose(file);
                        cerrado = 1;
                        comando[0] = '\0';
                        archivo[0] = '\0';
                        break;
                    }

                    token = strtok(linea, " \n\t ,");
                    if (token == NULL)
                    continue;

                    instruccion = token;

                    arg1 = strtok(NULL, " \n\t ,");
                    arg2 = strtok(NULL, " \n\t ,");

                    // Sintaxis para los espacios
                    if (!validarEspacios(linea_original, instruccion, contadorLinea)){
                        cerrado = 1;
                        if(lista_listos == NULL && lista_ejecucion == NULL){
    reiniciarVariables_cerrar(comando,archivo,&EAX,&EBX,&ECX,&EDX);
}
                        break;
                    }//Verifica si la instruccion es valida
                    if (!Operaciones(instruccion, contadorLinea, linea_original)){
                        cerrado = 1;
                        if(lista_listos == NULL && lista_ejecucion == NULL){
    reiniciarVariables_cerrar(comando,archivo,&EAX,&EBX,&ECX,&EDX);
}
                        break;
                    }

                    if ((strcmp(instruccion, "MOV") == 0 && !MOV(arg1,arg2,contadorLinea,linea_original)) ||
                        (strcmp(instruccion, "ADD") == 0 && !ADD(arg1,arg2,contadorLinea,linea_original)) ||
                        (strcmp(instruccion, "SUB") == 0 && !SUB(arg1,arg2,contadorLinea,linea_original)) ||
                        (strcmp(instruccion, "MUL") == 0 && !MUL(arg1,arg2,contadorLinea,linea_original)) ||
                        (strcmp(instruccion, "DIV") == 0 && !DIV(arg1,arg2,contadorLinea,linea_original)) ||
                        (strcmp(instruccion, "INC") == 0 && !INC(arg1,arg2,contadorLinea,linea_original)) ||
                        (strcmp(instruccion, "DEC") == 0 && !DEC(arg1,arg2,contadorLinea,linea_original))) {
if(lista_listos == NULL && lista_ejecucion == NULL){
    reiniciarVariables_cerrar(comando,archivo,&EAX,&EBX,&ECX,&EDX);
}
                            cerrado = 1;
                        break;
                    }

                    else if ((strcmp(instruccion, "END") == 0)){
                        encontroEND = 1;
                        if(feof(procesoEjecucion -> Archivo)){//Encontro END y se acabo el archivo(correcto)
                            move(y_renglon, 0); clrtoeol();
                            refresh();
                            mvprintw(y_renglon, 0, "%-5d %-20s %8d %8d %8d %8d", contadorLinea, linea_original, EAX, EBX, ECX, EDX);
                            refresh();
                            napms(1000);
                           
                            struct Nodo *procesoTerminado = extraerPrimero(&lista_ejecucion); 
                            if(procesoTerminado != NULL){
                                procesoTerminado -> Status = 'T';
                                insertarFinal(&lista_terminados,procesoTerminado);
                            }

                                 imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);   
                            //imprimirlista(lista_terminados, y_procesoTerminado);
                            
                            if(lista_listos == NULL && lista_ejecucion == NULL){
    reiniciarVariables_cerrar(comando,archivo,&EAX,&EBX,&ECX,&EDX);
}

                            break;
                        } else { //Solo encontro END
                            mvprintw(y_mensajes, 0, "ERROR: END encontrado sin que el archivo terminara linea %d:\"%s\"", contadorLinea, linea_original);
                            refresh();
                            napms(1000);
                            if(lista_listos == NULL && lista_ejecucion == NULL){
    reiniciarVariables_cerrar(comando,archivo,&EAX,&EBX,&ECX,&EDX);
}
                            break;
                        }
                    }
                    refresh();
                    napms(1000); //Tiempo para ver las lineas de impresion para renglon

                    if (kbhit()){
                        move(y_linea_comando, 0); clrtoeol();
                        refresh();
                        mvprintw(y_linea_comando, 0, "(D)> "); //Linea de comando que interrumpe(Dentro del kbhit)
                        char entrada[200];
                        char extra[100];

                        getnstr(entrada, 199);

                        num_palabras = sscanf(entrada, "%99s %99s %99s", comando, archivo, extra);

                        FILE *file_interrupcion = fopen(archivo, "r");

                        if (strcmp(comando, "Salir") == 0){
                            if (num_palabras > 1){
                                move(y_mensajes, 0); clrtoeol();
                                refresh();
                                mvprintw(y_mensajes, 0, "(D)ERROR: comando invalido");
                                refresh();
                                comando[0] = '\0';
                                archivo[0] = '\0';
                                num_palabras = 0;
                                continue;
                            }
                            salirPrograma();
                        }

                        else if (strcmp(comando, "Ejecuta") == 0){
                            if (num_palabras < 2){
                                move(y_mensajes, 0); clrtoeol();
                                refresh();
                                mvprintw(y_mensajes, 0, "(D)ERROR: falta el nombre del archivo");
                                refresh();
                                continue;
                            }
                            else if (num_palabras > 2){
                                move(y_mensajes, 0); clrtoeol();
                                refresh();
                                mvprintw(y_mensajes, 0, "(D)ERROR: demasiados argumentos");
                                refresh();

                                comando[0] = '\0';
                                archivo[0] = '\0';
                                num_palabras = 0;

                                continue;
                            }
                            if (file_interrupcion == NULL){
                                move(y_mensajes, 0); clrtoeol();
                                refresh();
                                mvprintw(y_mensajes, 0, "(D)No se pudo abrir el archivo %s", archivo);
                                refresh();
                                continue;
                            }
                            //clear();
                            //fclose(file);
                            //FILE *file = fopen(archivo, "r");
                            pid++;
                            insertar(&lista_listos,pid,file_interrupcion,archivo,'L',0); 
                            imprimirEstado(lista_listos, lista_ejecucion, lista_terminados);
                           // comando[0] = '\0';
                           // archivo[0] = '\0';

                            cerrado = 1;
                            EAX = EBX = ECX = EDX = 0; // Reiciar los valores
                            
                            refresh();   

                            break;
                        }
                        else{//La interrupcion con un comando que no es Salir o Ejecuta
                            move(y_mensajes, 0); clrtoeol();
                            refresh();
                            mvprintw(y_mensajes, 0, "(D)Comando no valido");
                            continue;
                        }
                    }
                }// Por si no hay END en el archivo y ya EOF
            }
            if (encontroEND == 0 && cerrado == 0){
                    move(y_mensajes, 0); clrtoeol();
                    refresh();
                    mvprintw(y_mensajes, 0, "ERROR: Fin de archivo sin END");
                    refresh();
                    napms(1000);
                    if(lista_listos == NULL && lista_ejecucion == NULL){
    reiniciarVariables_cerrar(comando,archivo,&EAX,&EBX,&ECX,&EDX);
}

                    continue;
            }

        //imprimirlista(lista_terminados, y_procesoTerminado);
            
    }
    endwin();
}



int ejecutarOperaciones(char *arg1, char *arg2, int contadorLinea, const char *linea_original,char tipoOp) { 
    if (!filtro(arg1, arg2, contadorLinea, linea_original)) return 0;
    if (!Comas_2pam(linea_original, contadorLinea)) return 0;

    int *R1 = ObtenerRegistro(arg1);
    int valor = 0;

    if (Registro(arg2)) {
        valor = *ObtenerRegistro(arg2);
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
    mvprintw(y_renglon,0,"%-5d %-20s %8d %8d %8d %8d", contadorLinea, linea_original, EAX, EBX, ECX, EDX);

    return 1;
}

int INC_DEC(char *arg1,char *arg2, int contadorLinea, const char *linea_original, int incremento){
    if (!filtroIncDec(arg1, NULL, contadorLinea, linea_original)) return 0;
    if (!Comas_1pam(linea_original, contadorLinea)) return 0;
    if (!Registro(arg1)){
        move(y_mensajes,0); clrtoeol(); refresh();
        mvprintw(y_mensajes,0,"ERROR: NO es Registro %s en linea %d:\"%s\"", arg1, contadorLinea, linea_original);
        return 0;
    }

    int *R = ObtenerRegistro(arg1);
    *R += incremento;

    move(y_renglon,0); clrtoeol(); refresh();
    mvprintw(y_renglon,0,"%-5d %-20s %8d %8d %8d %8d", contadorLinea, linea_original, EAX, EBX, ECX, EDX);

    return 1;
}

int MOV(char *arg1, char *arg2, int contadorLinea, const char *linea_original)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'M'); }
int ADD(char *arg1, char *arg2, int contadorLinea, const char *linea_original)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'A'); }
int SUB(char *arg1, char *arg2, int contadorLinea, const char *linea_original)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'S'); }
int MUL(char *arg1, char *arg2, int contadorLinea, const char *linea_original)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'U'); }
int DIV(char *arg1, char *arg2, int contadorLinea, const char *linea_original)
{ return ejecutarOperaciones(arg1,arg2,contadorLinea,linea_original,'D'); }
int INC(char *arg1, char *arg2, int contadorLinea, const char *linea_original)
{ return INC_DEC(arg1,arg2,contadorLinea,linea_original,1); } //positivo para que sume
int DEC(char *arg1, char *arg2, int contadorLinea, const char *linea_original)
{ return INC_DEC(arg1,arg2,contadorLinea,linea_original,-1); } //argumento negativo para que decremente


// ** almacena la dirección de memoria de otro puntero, debido a que la variable file es un puntero y queremos la direccion del puntero
void reiniciarVariables_cerrar(char *comando,char *archivo, int *EAX, int *EBX, int *ECX, int *EDX){

    comando[0] = '\0';
    archivo[0] = '\0';
    *EAX = 0;
    *EBX = 0;
    *ECX = 0;
    *EDX = 0; //Asignamos el valor apuntando hacia las variables
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

int *ObtenerRegistro(char *nombre){
    if (strcmp(nombre, "EAX") == 0){
        return &EAX;
    }
    else if (strcmp(nombre, "EBX") == 0){
        return &EBX;
    }
    else if (strcmp(nombre, "ECX") == 0){
        return &ECX;
    }
    else if (strcmp(nombre, "EDX") == 0){
        return &EDX;
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
    //mvprintw(y_procesoEjecucion,0,"%c", lista -> Status);
    while (lista != NULL) {  
            //clrtoeol();                  //p para la direccion de memoria                         //seria PC IR EAX....
            //imprimirproceso(lista,y_ncurses);
            mvprintw(y_ncurses,0,"%-5d %-20s %-12c %-5d", lista->PID, lista->nombrePro,lista->Status, lista->PC);
            lista = lista->sig;
            y_ncurses++;
    }
}

// Mostrar todas las listas
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