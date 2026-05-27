#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

void ComandoVel (int ms){
    napms(ms);
}

int main(){
    initscr();
    echo(); 

    char buffer[20]; 
    int ms;          
    int contador = 0; 

    printw("Introduce los milisegundos: ");
    refresh();
    
    scanw("%19s", buffer); 
    ms = atoi(buffer); 

    noecho(); 
    nodelay(stdscr, TRUE); 

    while(getch() != 'q'){
        clear(); 
        mvprintw(1, 1, "hola %d", contador); 
        refresh();
        contador++; 
        
        ComandoVel(ms); 
    }
}

#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

void ComandoVel (int ms){
    napms(ms);
}

int Negativo(char *ASCCI){
    int i=0;
    if(ASCCI[0]== '-'){
        return 1;
    }
    return 0;
}

int main(){

    while (1){
    initscr();
    echo(); 

    char buffer[20]; 
    int ms;          
    int contador = 0; 

    printw("Introduce los milisegundos: ");
    refresh();
    
    scanw("%19s", buffer); 
    if(Negativo(buffer) == 1){
        mvprintw(3,3, "Es negativo pa");
        break;
    }
    ms = atoi(buffer); 

    noecho(); 
    nodelay(stdscr, TRUE); 

    while(getch() != 'q'){
        clear(); 
        mvprintw(1, 1, "hola %d", contador); 
        refresh();
        contador++; 
        
        ComandoVel(ms); 
    }
    }

    endwin(); 
    return 0;
}