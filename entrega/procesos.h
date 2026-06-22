#ifndef PROCESOS_H
#define PROCESOS_H

#include <stdio.h>

// estrucutra para las listas
struct Nodo
{
    int PID;             // identificador unico
    //FILE *Archivo;       // nombre del archivo, Guardar el puntero al archivo FILE *
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
    int PRIORY; // prioridad
    int (*TMP)[3];
    int num_paginas;
    int TIEMPO_SUSP; // tiempo de salida en suspendidos
    int num_lineas;
    struct Nodo *sig; // puntero al siguiente nodo
};

// variables globales de ncurses
extern int ancho_procesos;
extern int ancho_TMP;
extern int ancho_TMM;
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
void insertar(struct Nodo **lista, int pid, int gid, int pc, int num_paginas, int num_lineas, const char *nombre);
void insertarFinal(struct Nodo **lista, struct Nodo *proceso);
struct Nodo *extraerPrimero(struct Nodo **lista);
struct Nodo *extraerNodo(struct Nodo **lista, int pid);
int contarNodos(struct Nodo *lista);
void A_terminadosError(struct Nodo **lista_ejecucion, struct Nodo **lista_terminados);
struct Nodo *matar(struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos, struct Nodo **lista_nuevos, int pid);
struct Nodo *buscar(struct Nodo *lista, int pid);
struct Nodo *buscarGID(struct Nodo *lista, int gid);
struct Nodo *forkProceso(struct Nodo *proceso_original, int nuevo_pid, int nuevo_pc);
struct Nodo *forkProcesoComando(struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos, int pid_comando, int nuevo_pid, int pc);
int CalculoPriodidad(struct Nodo **lista, int grupos, int Base);
struct Nodo *extraerNodo_Prioridad(struct Nodo **lista, int priory);
struct Nodo *Fair_Share(struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int grupos, int Base);
void GCPU_Global(struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int GID, int GCPU);
int Busqueda_GID(struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int GID);
void TiempoEnSuspendidos(struct Nodo *proceso);
void RevisarSuspendidos(FILE *swap, char RAM[][400], struct Nodo *lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, int TMM[][2], int TMS[], int *porS, int *porR);
void RevisarNuevos(FILE *swap, struct Nodo **lista_listos, struct Nodo **lista_nuevos, int TMS[]);
int procesarMata(FILE *swap, char RAM[][400], char archivo[], struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos, struct Nodo **lista_nuevos, int TMM[][2], int TMS[], int num_palabras, int *grupos, int *porS, int *porR);
int procesarFork(char archivo[], char extra[], struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_terminados, struct Nodo **lista_suspendidos, int num_palabras, int *pid);

// validaciones
int Registro(char *token);
int Operaciones(char *token, const char *linea_original, int contadorLinea);
int Digito(char *token);
int valivarLimitInt(char *token, int *resultado);
int filtro(char *arg1, char *arg2, const char *linea_original, int contadorLinea);
int filtroIncDecJnz(char *arg1, char *arg2, const char *linea_original, int contadorLinea);
int Comas_2pam(const char *linea_original, int contadorLinea);
int Comas_1pam(const char *linea_original, int contadorLinea);
int validarEspacios(const char *linea_original, char *instruccion, int contadorLinea);
int Negativo(char *numero);

// operaciones
int *ObtenerRegistro(struct Nodo * p, char * nombre);
int ejecutarOperaciones(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea, char tipoOp);
int INC_DEC(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea, int incremento);
int JNZ_(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea);
int MOV(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea);
int ADD(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea);
int SUB(struct Nodo * proceso, char * arg1, char *arg2, const char *linea_original, int contadorLinea);
int MUL(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea);
int DIV(struct Nodo *proceso, char *arg1, char *arg2, const char *linea_original, int contadorLinea);
int INC(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea);
int DEC(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea);
int JNZ(struct Nodo *proceso, char *arg1, const char *linea_original, int contadorLinea);

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
void in_TMP(int TMP[][3], int num_paginas);
void in_TMM(int TMM[][2]);
int Busqueda_TMM(int TMM[][2]);
int validarArchivo(const char *NombrePro);
int ContadorLineas(const char *archivo);
int Paginacion(FILE *archivoProceso, FILE *swap, int TMP[][3], int TMS[], int PID);
int reescritura(FILE *ArchivoBinario, const char *NombrePro, int TMP[][3], int TMS[], int pid);
int BitPresencia_TMP(int TMP[][3], int pagina);
int RAMLlena(int TMM[][2]);
void EscrituraRam(FILE *swap, char RAM[][400], struct Nodo *lista_ejecucion, struct Nodo *lista_listos, struct Nodo *lista_suspendidos, struct Nodo *proceso, int TMM[][2], int pagina);
void AlgoritmoReloj(char RAM[][400], struct Nodo *lista_ejecucion, struct Nodo *lista_listos, struct Nodo *lista_suspendidos, int TMM[][2]);
void actualizarTMPdeMarcoL(struct Nodo *lista_ejecucion, struct Nodo *lista_listos, struct Nodo *lista_suspendidos, int gidDueno, int marcoLiberado);
void LiberarRAM(char RAM[][400], int marco);
void liberarRAMproceso(char RAM[][400], int TMM[][2], int gid);
void liberarSWAP(FILE *swap, int TMP[][3], int TMS[], int num_paginas);
int marcosLS(int TMS[]);
int marcosLR(int TMM[][2]);
void porcentajes(int TMM[][2], int TMS[], int *porS, int *porR);
void actualizarTMPGrupo(struct Nodo **lista_ejecucion, struct Nodo **lista_listos, struct Nodo **lista_suspendidos, struct Nodo *proceso);
int BusquedaBitP(struct Nodo *lista_ejecucion, struct Nodo *lista_listos, struct Nodo *lista_suspendidos, struct Nodo *proceso, int pagina);
#endif