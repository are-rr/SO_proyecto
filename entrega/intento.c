#include <ncurses.h>
#include <string.h>

/*void barra_uso(WINDOW *win, int y, int x, int ancho, int porcentaje, char *titulo) {
    mvwprintw(win, y, x, "%s %3d%% [", titulo, porcentaje);

    int llenos = (porcentaje * ancho) / 100;

    for (int i = 0; i < ancho; i++) {
        if (i < llenos)
            waddch(win, '#');
        else
            waddch(win, '-');
    }

    waddch(win, ']');
}

void dibujar_interfaz() {
    int alto, ancho;
    getmaxyx(stdscr, alto, ancho);

    int mitad = ancho / 2;
    int alto_barras = 4;

    WINDOW *win_izq = newwin(alto - alto_barras, mitad, 0, 0);
    WINDOW *win_der = newwin(alto - alto_barras, ancho - mitad, 0, mitad);
    WINDOW *win_barras = newwin(alto_barras, ancho, alto - alto_barras, 0);

    box(win_izq, 0, 0);
    box(win_der, 0, 0);
    box(win_barras, 0, 0);

    // Lado izquierdo
    mvwprintw(win_izq, 1, 2, "CPU / Lectura de archivo");
    mvwprintw(win_izq, 3, 2, "PC   IR        EAX   EBX   ECX   EDX");
    mvwprintw(win_izq, 4, 2, "7    END       0     0     100   1");

    mvwprintw(win_izq, 7, 2, "PID  Nombre       Status      PC   IR");
    mvwprintw(win_izq, 8, 2, "1    vacio.txt    Terminado   7    END");
    mvwprintw(win_izq, 9, 2, "2    archivo.txt  Ejecucion   3    INC EBX");

    // Lado derecho
    mvwprintw(win_der, 1, 2, "TMP");
    mvwprintw(win_der, 2, 2, "Pag  Pres  RAM  SWAP");
    mvwprintw(win_der, 3, 2, "0    1     5    -");
    mvwprintw(win_der, 4, 2, "1    0     -    2");

    mvwprintw(win_der, 1, 25, "TMM");
    mvwprintw(win_der, 2, 25, "Marco  Dueno  Reloj");
    mvwprintw(win_der, 3, 25, "0      2      0");
    mvwprintw(win_der, 4, 25, "1      3      1");

    // Barras inferiores
    barra_uso(win_barras, 1, 2, 25, 60, "RAM ");
    barra_uso(win_barras, 2, 2, 25, 35, "SWAP");

    wrefresh(win_izq);
    wrefresh(win_der);
    wrefresh(win_barras);

    delwin(win_izq);
    delwin(win_der);
    delwin(win_barras);
}*/

//tamaño de terminal
int alto; 
int ancho; 

int main() {
    initscr();
    //noecho();
    //curs_set(0);
    
    getmaxyx(stdscr, alto, ancho);//obtine las filas y columnas de la terminal

    int mitad_terminal = ancho* 1/2; //para dividir la pantalla a la mitad
    int alto_abajo =4;
                    //newwin(alto,ancho,coordenada y, coordenada x);
    WINDOW *lado_izq = newwin(alto-alto_abajo,ancho,0,0);
    WINDOW *lado_der = newwin(alto-alto_abajo,ancho-mitad_terminal,0,mitad_terminal); 
    WINDOW *abajo= newwin(alto_abajo,ancho,alto-alto_abajo,0); 

    // Lado izquierdo
    mvwprintw(lado_izq, 1, 2, "CPU");
    mvwprintw(lado_izq, 3, 2, "PC   IR        EAX   EBX   ECX   EDX");
    mvwprintw(lado_izq, 4, 2, "7    END       0     0     100   1");

    mvwprintw(lado_izq, 7, 2, "PID  Nombre       Status      PC   IR");
    mvwprintw(lado_izq, 8, 2, "1    vacio.txt    Terminado   7    END");

    // Lado derecho
    mvwprintw(lado_der, 1, 2, "TMP");
    mvwprintw(lado_der, 2, 2, "Pag  Pres  RAM  SWAP");
    mvwprintw(lado_der, 3, 2, "0    1     5    -");

    mvwprintw(lado_der, 1, 25, "TMM");
    mvwprintw(lado_der, 2, 25, "Marco  Dueno  Reloj");
    mvwprintw(lado_der, 3, 25, "0      2      0");

    mvwprintw(abajo, 1, 2, "RAM");
    mvwprintw(abajo, 2, 2, "SWAP");

    wrefresh(lado_izq);
    wrefresh(lado_der);
    wrefresh(abajo);
    //mvwprintw(lado_der,)
    while (1) {
        //clear();
        dibujar_interfaz();
        refresh();

        int tecla = getch();
        if (tecla == 'q')
            break;
    }

    endwin();
    return 0;
}