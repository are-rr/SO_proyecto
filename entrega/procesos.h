#ifndef PROCESOS_H
#define PROCESOS_H

#include <stdio.h>

// estrucutra para las listas
struct Nodo{
    int PID;             // identificador unico
    FILE *Archivo;       // nombre del archivo, Guardar el puntero al archivo FILE *
    char nombrePro[100]; // para el nombre del archivo
    int EAX;             // Registros
    int EBX;
    int ECX;
    int EDX;
    int Status;   // 3=terminado, 4=Terminado-Error, 5=Terminado-mata
    int PC;       // contador de programa(contadorLInea)
    char IR[100]; // para guardar la ultima instruccion
    int GID;      // indentificador del grupo
    int CPU;
    int GCPU;
    int PRIORY;        // prioridad
    int(*TMP)[3];
    int num_paginas;
    int TIEMPO_SUSP;
    int num_lineas;
    struct Nodo *sig; // puntero al siguiente nodo
};

// variables globales de ncurses
extern int ancho_procesos ;
extern int ancho_TMP ;
extern int ancho_TMM ;
extern int ancho_TMS;
extern int y_header; // extern quiere decir que esta variable existe en otro archivo
extern int y_renglon;
extern int y_mensajes;
extern int y_linea_comando;
extern int y_header2;
extern int y_procesoEjecucion;
extern int y_renglon_TMS;
extern int y_renglon_TMM;
extern int y_renglon_TMP;
extern int x_TMM;
extern int x_TMS;
extern int x_TMP;
extern int ejecutando;
extern int pid;

// prototipos de las funciones
// listas
void insertar(struct Nodo **cabeza, int pid, int gid, const char *nombre, int pc, int num_paginas, int num_lineas);
void insertarFinal(struct Nodo **cabeza, struct Nodo *proceso);
struct Nodo *extraerPrimero(struct Nodo **cabeza);
struct Nodo *extraerNodo(struct Nodo **lista, int id);
int contarNodos(struct Nodo *lista);
void A_terminadosError(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados);
struct Nodo *matar(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, struct Nodo **lista_nuevos, int id_p);
struct Nodo *buscar(struct Nodo *lista, int pid);
struct Nodo *forkProceso(struct Nodo *original, int nuevo_pid, int nuevo_pc, int nuevo_gid);
struct Nodo *forkProcesoComando(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int pid_comando, int pc, int nuevo_pid);
int validarPC(FILE *copiaArchivo, int pc_buscar);
int CalculoPriodidad(struct Nodo **nodolis, int grupos, int Base);
struct Nodo *extraerNodo_Prioridad(struct Nodo **lista, int priory);
struct Nodo *Fair_Share(struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int grupos, int Base);
void GCPU_Global(struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int GID, int GCPU);
int Busqueda_GID(struct Nodo **lista_listos, struct Nodo **lista_ejecucion, struct Nodo **lista_suspendidos, int GID);
void RevisarNuevos( struct Nodo **lista_nuevos,struct Nodo **lista_listos,FILE *swap,int TMS[]);

void TiempoEnSuspendidos(struct Nodo *proceso);
void RevisarSuspendidos(struct Nodo **lista_suspendidos, struct Nodo **lista_listos, struct Nodo *lista_ejecucion, FILE *swap, char RAM[][400], int TMM[][2], int TMS[], int *porS, int *porR);
// validaciones
int Registro(char *token);
int Operaciones(char *token, int contadorLinea, const char *linea_original);
int Digito(char *token);
int filtroIncDecJnz(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int filtro(char *arg1, char *arg2, int contadorLinea, const char *linea_original);
int Comas_2pam(const char *linea_original, int contadorLinea);
int Comas_1pam(const char *linea_original, int contadorLinea);
int validarEspacios(const char *linea_original, char *instruccion, int contadorLinea);
int Negativo(char *numero);

// operaciones
int *ObtenerRegistro(char *nombre, struct Nodo *p);
int ejecutarOperaciones(char *arg1, char *arg2, int contadorLinea, const char *linea_original, char tipoOp, struct Nodo *proceso);
int INC_DEC(char *arg1, int contadorLinea, const char *linea_original, int incremento, struct Nodo *proceso);
int MOV(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso);
int ADD(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso);
int SUB(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso);
int MUL(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso);
int DIV(char *arg1, char *arg2, int contadorLinea, const char *linea_original, struct Nodo *proceso);
int INC(char *arg1, int contadorLinea, const char *linea_original, struct Nodo *proceso);
int DEC(char *arg1, int contadorLinea, const char *linea_original, struct Nodo *proceso);
int JNZ(char *arg1, int contadorLinea, const char *linea_original, struct Nodo *proceso);

// ncurses
void imprimirProceso(struct Nodo *p, int y_ncurse, const char *cadena);
void imprimirlista(struct Nodo *lista, int y_ncurses, int status);
void imprimirEstado(struct Nodo *listos, struct Nodo *ejecucion, struct Nodo *terminados, struct Nodo *suspendidos, struct Nodo *nuevos);
void imprimir_TMP(int TMP[][3], int y_renglon_TMP, int x_TMP, int ContadorL, int pid);
void limpiarZona(int y, int x, int ancho);
void limpiarZonaTabla(int renglon, int x, int ancho);
void imprimir_TMM(int TMM[][2], int y_renglon_TMM, int x_TMM);
void imprimir_TMS(int TMS[], int y_renglon_TMS, int x_TMS);
// otros
void reiniciarVariables(char *comando, char *archivo);
int kbhit(void);
void salirPrograma(FILE *swap);
void ComandoVel(int ms);

// Memoria
int Crear_ArchivoBinario(const char *nombre, int size_IR);
void in_TMS(int TMS[]);
int Busqueda_TMS(int TMS[]);
int Paginacion(FILE *archivoProceso, FILE *swap, int PID, int TMS[], int TMP[][3]);
int reescritura(const char *NombrePro, FILE *ArchivoBinario, int pid, int TMS[], int TMP[][3]);
void in_TMP(int TMP[][3], int num_paginas);
void in_TMM(int TMM[][2]);
int Busqueda_TMM(int TMM[][2]);
int BitPresencia_TMP(int TMP[][3], int pagina);
int ObtenerMarcoSwap(int TMP[][3], int pagina);
int ObtenerMarcoRAM(int TMP[][3], int pagina);
void EscrituraRam(FILE *swap, char RAM[][400], int pagina, int TMP[][3], int TMM[][2], int PID);
void in_RAM(char RAM[][400]);
int RAMLlena(int TMM[][2]);
int ContadorLineas(const char *archivo);
void AlgoritmoReloj(int TMM[][2], char RAM[][400], struct Nodo *lista_listos, struct Nodo *lista_ejecucion, struct Nodo *lista_suspendidos);
void actualizarTMPdeMarcoL(struct Nodo *lista_listos, struct Nodo *lista_ejecucion, struct Nodo *lista_suspendidos, int pidDueno, int marcoLiberado);
void LiberarRAM(char RAM[][400], int marco);
int validarArchivo(const char *NombrePro);
void liberarSWAP(FILE *swap, int TMP[][3], int num_paginas, int TMS[]);
int marcosLS(int TMS[]);
int marcosLR(int TMM[][2]);
void porcentajes(int TMS[], int TMM[][2], int *porS, int *porR);
void liberarRAMproceso(char RAM[][400], int TMM[][2], int pid);
#endif