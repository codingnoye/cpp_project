#include <ncurses.h>
#include <chrono>
#include <string>
using namespace std;

const int EMPTY = 0;
const int BODY = 1;
const int WALL = 2;
const int IWALL = 3;
const int POISON = 4;
const int GROWTH = 5;
const int GATE = 6;

int main() {
    WINDOW *game;
    WINDOW *info;

    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);

    border('*', '*', '*', '*', '*', '*', '*', '*');
    mvprintw(1, 1, "default");
    refresh();

    game = newwin(21, 42, 1, 1);
    wbkgd(game, COLOR_PAIR(1));
    wattron(game, COLOR_PAIR(1));
    mvwprintw(game, 1, 1, "this is game");
    wrefresh(game);

    keypad(stdscr, TRUE);
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
    while (true) {
        int i = getch();
        switch (i)
        {
        case -1:
            break;
        case 259:
            mvwprintw(game, 1, 1, "up");
            break;
        case 258:
            mvwprintw(game, 1, 1, "down");
            break;
        case 260:
            mvwprintw(game, 1, 1, "left");
            break;
        case 261:
            mvwprintw(game, 1, 1, "right");
            break;

        }
        
        wrefresh(game);
    }

    getch();
    delwin(game);
    endwin();

    return 0;
}