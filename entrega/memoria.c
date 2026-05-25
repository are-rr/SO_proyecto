#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <math.h>

#include "procesos.h"


//              tamaño_IR
 
    //FILE *archivo = fopen("archivo.txt", "r");        main.c
    //char arreglo[TAMAÑORAMCALCULADO];

   /* if(archivo != NULL){
        fgets(arreglo,TAMAÑORAMCALCULADO, archivo);
        fclose(archivo);
    
    }*/

#include <stdio.h>
#include <stdlib.h>

//memoria virtual
int Crear_ArchivoBinario(const char *nombre, int size_IR) {

    int size_ArchivoBinario  = 131072;
    size_ArchivoBinario = size_ArchivoBinario  * size_IR;
    char *numeros = malloc(size_ArchivoBinario);

    FILE *ArchivoBinario = fopen(nombre, "wb");
    if (ArchivoBinario == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    /*if (_fseeki64(file, size_binario - 1, SEEK_SET) != 0) { //fseek64(puntero al archivo, cantidad a desplazar,punto de referencia para dezplazarse)
        perror("Error al posicionar el cursor");                                                            //SEEK_SET -> desde el inicio del archivo
        fclose(file);                                                                                       //SEEK_CUR ->desde la posicion actual del cursor
        return 1;
    }*/

    //posicionarnos para cerrar el archivo
    //fwrite(&file, sizeof(char), 1, file);//NOTA: 

    memset(numeros, 0, sizeof(numeros));
    fwrite(numeros , 1 , size_ArchivoBinario,  ArchivoBinario);
    fclose(ArchivoBinario);
    free(numeros);
    return 0;
}

int reescritura(const char *NombrePro, const char *ArchivoBinario){
    FILE *archivoP = fopen(NombrePro,"r");
    if (archivoP == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }
    FILE *archivoB = fopen(ArchivoBinario,"wb");
    if (archivoB == NULL) {
        perror("Error al abrir el archivo");
        fclose(archivoB);
        return 1;
    }

    char buffer[1024];
    size_t caracteresLeidos;
    while ((caracteresLeidos = fread(buffer,1,1024,archivoP)) > 0){
        fwrite(buffer,1,caracteresLeidos,archivoB);
    }
    fclose(archivoB);
    fclose(archivoP);
}

int memoria_RAM(FILE *archivo, int size_IR){
    int size_RAM  = 0;
    size_RAM = size_IR * 64;
    char RAM[size_RAM]; // almacenar 64 instrucciones

    if(archivo != NULL){
        fgets(RAM,size_RAM, archivo);
        fclose(archivo);
    }

    return 0;
}


int main(){
    char IR[100] = "MOV EAX,EBX";
    char nombre[100] = "prueba2";

    char origen[100] = "file2";
    char destino[100] = "prueba2";

    //Crear_ArchivoBinario(nombre,strlen(IR));
    reescritura(origen,destino);
    
    return 0;
}


