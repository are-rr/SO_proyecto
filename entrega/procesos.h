#ifndef PROCESOS_H
#define PROCESOS_H

#include <stdio.h>

//estrucutra para las listas
struct Nodo {
    int PID;       // identificador unico
    FILE* Archivo;//nombre del archivo, Guardar el puntero al archivo FILE *
    char nombrePro[100]; //para el nombre del archivo
    int EAX; //Registros
    int EBX;
    int ECX;
    int EDX;
    char Status;     // L = listo, E= ejecucion,  T =terminado, X=Terminado-Error, Z=Terminado-mata
    int PC;    // contador de programa(contadorLInea)
    char IR[100];//para guardar la ultima instruccion
    int GID; //indentificador del grupo
    int CPU;
    int GCPU;
    int PRIORY; //prioridad
    struct Nodo *sig;  // puntero al siguiente nodo
};

// variables globales de ncurses
extern int y_header;        //extern quiere decir que esta variable existe en otro archivo
extern int y_renglon;
extern int y_mensajes;
extern int y_linea_comando;
extern int y_header2;
extern int y_procesoEjecucion;

extern int ejecutando;
extern int pid;

//prototipos de las funciones
//listas
void insertar(struct Nodo **cabeza, int pid,int gid,FILE *archivo,const char *nombre,char status, int pc);
void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso);
struct Nodo *extraerPrimero(struct Nodo **cabeza);
struct Nodo *extraerNodo(struct Nodo **lista, int id);
int contarNodos(struct Nodo *lista);
void A_terminadosError(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados);
int matar(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, int id_p);
struct Nodo *buscar(struct Nodo *lista,int pid);
struct Nodo* forkProceso(struct Nodo *original, int nuevo_pid, int nuevo_pc, int nuevo_gid);
struct Nodo *forkProcesoComando(struct Nodo **lista_ejecucion,struct Nodo **lista_terminados,struct Nodo **lista_listos,int pid_comando,int pc,int nuevo_pid,int nuevo_gid);
int posicionarArchivoEnPC(FILE *copiaArchivo,int pc_buscar);
int CalculoPriodidad(struct Nodo **nodolis, int grupos, int Base);
struct Nodo* extraerNodo_Prioridad(struct Nodo **lista, int priory);
struct Nodo *Fair_Share(struct Nodo **lista_listos,int grupos,int Base);
void GCPU_Global(struct Nodo **lista_listos, int GID, int GCPU);
int Busqueda_GID(struct Nodo **lista_listos,struct Nodo **lista_ejecucion, int GID);

//validaciones
int Registro(char *token);
int Operaciones(char *token, int contadorLinea, const char *linea_original);
int Digito(char *token);
int filtroIncDec(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int filtro(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int Comas_2pam(const char *linea_original, int contadorLinea);
int Comas_1pam(const char *linea_original, int contadorLinea);
int validarEspacios(const char *linea_original, char *instruccion, int contadorLinea);

//operaciones
int *ObtenerRegistro(char *nombre, struct Nodo *p);
int ejecutarOperaciones(char *arg1, char *arg2, int contadorLinea, const char *linea_original,char tipoOp, struct Nodo *proceso);
int INC_DEC(char *arg1, int contadorLinea, const char *linea_original, int incremento,struct Nodo *proceso);
int MOV(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int ADD(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int SUB(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int MUL(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int DIV(char *arg1, char *arg2, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int INC(char *arg1, int contadorLinea, const char *linea_original,struct Nodo *proceso);
int DEC(char *arg1, int contadorLinea, const char *linea_original,struct Nodo *proceso);

//ncurses
const char *statusTexto(char status);
void imprimirProceso(struct Nodo *p,int y_ncurse);
void imprimirlista(struct Nodo *lista, int y_ncurses);
void imprimirEstado(struct Nodo *listos,struct Nodo *ejecucion,struct Nodo *terminados);

//otros
void reiniciarVariables(char *comando,char *archivo);
int kbhit(void);
void salirPrograma();

#endif 
