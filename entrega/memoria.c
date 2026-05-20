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

    long int size_binario = 131072;
    size_binario = size_binario * (long int)size_IR;

    FILE *file = fopen(nombre, "wb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    if (_fseeki64(file, size_binario - 1, SEEK_SET) != 0) { //fseek64(puntero al archivo, cantidad a desplazar,punto de referencia para dezplazarse)
        perror("Error al posicionar el cursor");                                                            //SEEK_SET -> desde el inicio del archivo
        fclose(file);                                                                                       //SEEK_CUR ->desde la posicion actual del cursor
        return 1;
    }

    //posicionarnos para cerrar el archivo
    fwrite(&file, sizeof(char), 1, file);//NOTA: 
    fclose(file);
    return 0;
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



