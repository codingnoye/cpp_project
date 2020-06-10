#include <ncurses.h>
#include <chrono>
#include <string>
#include <fstream>
#include <deque>
using namespace std;

const int BKGRD = -1;
const int BODY = 1;
const int WALL = 2;
const int IWALL = 3;
const int POISON = 4;
const int GROWTH = 5;
const int GATE = 6;
const int EMPTY = 7;

const int NONE = -1;
const int DOWN = 0;
const int UP = 1;
const int LEFT = 2;
const int RIGHT = 3;

int getms() {
    return chrono::duration_cast<chrono::milliseconds> (
            chrono::system_clock::now().time_since_epoch()
        ).count();
}

struct pos{
    int y;
    int x;
};

class Game{
public:
    int map[21][21];
    int going = NONE;
    deque<pos> body;
    WINDOW* win;
    Game(WINDOW* win) {
        this->win = win;
        init_pair(EMPTY, COLOR_WHITE, COLOR_WHITE);
        init_pair(BODY, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(WALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(IWALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(GROWTH, COLOR_GREEN, COLOR_GREEN);
        init_pair(POISON, COLOR_RED, COLOR_RED);
        init_pair(GATE, COLOR_MAGENTA, COLOR_MAGENTA);
    }
    int load_map(string uri) {
        ifstream readfile;
        readfile.open(uri);
        if (readfile.is_open()) {
            char tmp[64];
            for (int y=0; y<21; y++) {
                readfile.getline(tmp, 64);
                for (int x=0; x<21; x++) {
                    map[y][x] = tmp[x]-48;
                    if (map[y][x]==0) map[y][x]=7;
                    else if (map[y][x]==1) body.push_back(pos{y, x});
                }
            }
        } else {
            return 1;
        }
        return 0;
    }
    bool init(string map_uri) {
        load_map(map_uri);
        going = LEFT;
        return true;
    }
    bool tick(int lastinput) {
        return move(lastinput);
    }
    bool move(int direction) {
        switch (direction) {
            case UP:
                if (going != DOWN) going = UP;
                break;
            case DOWN:
                if (going != UP) going = DOWN;
                break;
            case LEFT:
                if (going != RIGHT) going = LEFT;
                break;
            case RIGHT:
                if (going != LEFT) going = RIGHT;
                break;
        }
        if (going != NONE) {
            pos go = body.front();
            switch (going) {
                case UP:
                    go.y -= 1;
                    break;
                case DOWN:
                    go.y += 1;
                    break;
                case LEFT:
                    go.x -= 1;
                    break;
                case RIGHT:
                    go.x += 1;
                    break;
            }
            if (can_go(go)) {
                int go_tile = map[go.y][go.x];
                if (go_tile == EMPTY) {
                    // Basic move
                    pos tail = body.back();
                    body.pop_back();
                    map[tail.y][tail.x] = EMPTY;

                    body.push_front(go);
                    map[go.y][go.x] = BODY;
                } else if (go_tile == GROWTH) {
                    body.push_front(go);
                    map[go.y][go.x] = BODY;
                } else if (go_tile == POISON) {
                    pos tail = body.back();
                    body.pop_back();
                    map[tail.y][tail.x] = EMPTY;

                    tail = body.back();
                    body.pop_back();
                    map[tail.y][tail.x] = EMPTY;
                    
                    body.push_front(go);
                    map[go.y][go.x] = BODY;
                    if (body.size()<3) return false;
                }
                
            } else {
                // Game over
                return false;
            }
        }
        return true;
    }
    bool can_go(pos go) {
        if ((map[go.y][go.x] == WALL) || (map[go.y][go.x] == IWALL)) return false;
        return true;
    }
    void draw() {
        for (int y=0; y<21; y++) {
            for (int x=0; x<21; x++) {
                int now = map[y][x];
                wattron(win, COLOR_PAIR(now));
                mvwprintw(win, y, x*2, "aa");
                wattroff(win, COLOR_PAIR(now));
            }
        }
        wrefresh(win);
    }
};


int main() {
    // Ncurses setting
    WINDOW* game_win;
    WINDOW* info;

    initscr();
    start_color();
    
    init_pair(BKGRD, COLOR_BLACK, 
    COLOR_BLACK);
    border('*', '*', '*', '*', '*', '*', '*', '*');
    refresh();

    game_win = newwin(22, 42, 1, 1);
    wbkgd(game_win, COLOR_PAIR(1));
    wrefresh(game_win);

    keypad(stdscr, TRUE);
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);

    // Game setting
    Game game(game_win);
    int lastinput = NONE;
    int lasttime = getms();
    game.init("maps/1");

    // Main loop
    while (true) {
        int i = getch();
        if (258<=i && i<=261) lastinput = i - 258;
        int now = getms();
        int dt = now - lasttime;
        if (dt>=300) {
            if (!game.tick(lastinput)) {
                // Game over
            }
            game.draw();

            wrefresh(game_win);
            lasttime = now;
            lastinput = NONE;
        }
    }

    getch();
    delwin(game_win);
    endwin();

    return 0;
}