#include <ncurses.h>
#include <chrono>
#include <string>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <deque>
#include <cstring>
using namespace std;

const int MISSION = 11;
const int INFO = 10;
const int BKGRD = -1;
const int BODY = 1;
const int WALL = 2;
const int IWALL = 3;
const int POISON = 5;
const int GROWTH = 6;
const int GATE1 = 7;
const int GATE2 = 8;
const int EMPTY = 9;

const int NONE = -1;
const int DOWN = 0;
const int UP = 1;
const int LEFT = 2;
const int RIGHT = 3;

const int CLOCKWISE[4] = {LEFT, RIGHT, UP, DOWN};

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
    int item_pos[3][2];
    pos gate_pos[2];
    // int itemcooltime;
    int sc_growth = 0; int sc_poison = 0; int sc_gate = 0;
    int item_tick = 10;
    int mission[4]; //B,Grwoth,Poison,Gate
    bool game_over = false;
    int gate_cooltime = 0;
    int current_length = 0; int max_length = 0;

    deque<pos> body;
    WINDOW* win;
    WINDOW* info;
    WINDOW* miss;

    Game(WINDOW* win, WINDOW* info, WINDOW* miss) {
        this->win = win;
        this->info = info;
        this->miss = miss;
        init_pair(EMPTY, COLOR_WHITE, COLOR_WHITE);
        init_pair(BODY, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(WALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(IWALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(GROWTH, COLOR_GREEN, COLOR_GREEN);
        init_pair(POISON, COLOR_RED, COLOR_RED);
        init_pair(GATE1, COLOR_MAGENTA, COLOR_MAGENTA);
        init_pair(GATE2, COLOR_MAGENTA, COLOR_MAGENTA);
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
                    if (map[y][x]==0) map[y][x]=EMPTY;
                    else if (map[y][x]==BODY) body.push_back(pos{y, x});
                }
            }
        } else {
            return 1;
        }
        return 0;
    }
    char* to_char(string a, char* b){
        strcpy(b,a.c_str());
        return b;
    }
    void add_mission(){
        srand((unsigned int)time(0));
        mission[0] = rand()%16 + 3;
        mission[1] = rand()%5 + 1;
        mission[2] = rand()%5 + 1;
        mission[3] = rand()%3 + 1;
        mvwprintw(info, 1, 2, "[ Score Board ]");
        mvwprintw(info, 3, 2, "B :  3/3");
        mvwprintw(info, 4, 2, "+ :  0");
        mvwprintw(info, 5, 2, "- :  0");
        mvwprintw(info, 6, 2, "G :  0");
        mvwprintw(miss, 1, 2, "[ MISSION ]");
        string tmp = "B : "+ to_string(mission[0]) + "  (X)";
        char b[20];
        mvwprintw(miss, 3, 2, to_char(tmp, b));
        tmp = "+ : "+ to_string(mission[1]) + "  (X)";
        mvwprintw(miss, 4, 2, to_char(tmp, b));
        tmp = "- : "+ to_string(mission[2]) + "  (X)";
        mvwprintw(miss, 5, 2, to_char(tmp, b));
        tmp = "G : "+ to_string(mission[3]) + "  (X)";
        mvwprintw(miss, 6, 2, to_char(tmp, b));
        wrefresh(info);
        wrefresh(miss);
    }
    bool mission_check(int item){
        string tmp;
        char b[20];
        tmp = "B :  " + to_string(current_length) + "/" + to_string(max_length);
        mvwprintw(info, 3, 2, to_char(tmp, b));
        if(max_length == mission[0]) mvwprintw(miss, 3, 9, "(V)");
        switch(item){
            case GROWTH:
                tmp = "+ :  "+ to_string(sc_growth);
                mvwprintw(info, 4, 2, to_char(tmp, b));
                if(sc_growth == mission[1]) mvwprintw(miss, 4, 9, "(V)");
                break;
            case POISON:
                tmp = "- :  "+ to_string(sc_poison);
                mvwprintw(info, 5, 2, to_char(tmp, b));
                if(sc_poison == mission[2]) mvwprintw(miss, 5, 9, "(V)");
                break;
            case GATE1:
            case GATE2:
                tmp = "G :  "+ to_string(sc_gate);
                mvwprintw(info, 6, 2, to_char(tmp, b));
                if(sc_gate == mission[3]) mvwprintw(miss, 6, 9, "(V)");
                break;
        }
        if(mission[0] <= max_length && mission[1] <= sc_growth && mission[2] <= sc_poison && mission[3] <= sc_gate) return true;
        wrefresh(info);
        wrefresh(miss);
        return false;
    }
    void clear_item(){
        for(int i = 0; i<3; i++){
            if(map[item_pos[i][0]][item_pos[i][1]]!=BODY) map[item_pos[i][0]][item_pos[i][1]] = EMPTY;
        }
        wrefresh(win);
    }
    void clear_gate(){
        map[gate_pos[0].y][gate_pos[0].x] = WALL;
        map[gate_pos[1].y][gate_pos[1].x] = WALL;
        wrefresh(win);
    }
    void random_item(){
        srand((unsigned int)time(0));
        int percent = rand()%4;
        for(int i = 0; i<percent; i++){
            while(true){
                int item_y = rand()%17 + 2; //1~19 random
                int item_x = rand()%17 + 2;
                if(map[item_y][item_x] == EMPTY){
                    map[item_y][item_x] = GROWTH;
                    item_pos[i][0] = item_y;
                    item_pos[i][1] = item_x;
                    break;
                }
            }
        } //growth
        for(int i = percent; i<3; i++){
            while(true){
                int item_y = rand()%17 + 2; //1~19 random
                int item_x = rand()%17 + 2;
                if(map[item_y][item_x] == EMPTY){
                    map[item_y][item_x] = POISON;
                    item_pos[i][0] = item_y;
                    item_pos[i][1] = item_x;
                    break;
                }
            }
        } //poision
    }  
    void random_gate() {
        srand((unsigned int)time(0));
        int gate_y, gate_x;
        while(true){
            gate_y = rand()%21;
            gate_x = rand()%21;
            if(map[gate_y][gate_x] == WALL){
                gate_pos[0] = pos {gate_y, gate_x};
                break;
            }
        }
        map[gate_y][gate_x] = GATE1;
        while(true){
            gate_y = rand()%21;
            gate_x = rand()%21;
            if(map[gate_y][gate_x] == WALL){
                gate_pos[1] = pos {gate_y, gate_x};
                break;
            }
        }
        map[gate_y][gate_x] = GATE2;
    }    
    bool init(string map_uri) {
        load_map(map_uri);
        going = LEFT;
        current_length = 3;
        max_length = 3;
        random_item();
        random_gate();
        add_mission();
        return true;
    }
    bool tick(int lastinput) {
        if (item_tick-- == 0) {
            clear_item();
            random_item();
            item_tick = 10;
        }
        if (gate_cooltime-- == 0) {
            clear_gate();
            random_gate();
        }
        return move(lastinput);
    }
    bool is_center(pos gate) {
        return !(gate.x==0 || gate.y==0 || gate.x==20 || gate.y==20);
    }
    bool using_gate(pos now_gate, pos other_gate) {
        bool ic = is_center(other_gate);
        if (ic) {
            while (true) {
                pos gate_go = pos {other_gate.y + (going==UP?-1:0) + (going==DOWN?1:0), other_gate.x + (going==LEFT?-1:0) + (going==RIGHT?1:0)};
                int go_tile = map[gate_go.y][gate_go.x];
                if (!((go_tile == GATE1) || (go_tile == GATE2) || (go_tile == WALL) || (go_tile == IWALL))) break;
                going = CLOCKWISE[going];
            }
        } else {
            if (other_gate.y==20) going = UP;
            else if (other_gate.y==0) going = DOWN;
            else if (other_gate.x==20) going = LEFT;
            else if (other_gate.x==0) going = RIGHT;
        }
        gate_cooltime = current_length-1;
        pos go = pos {other_gate.y + (going==UP?-1:0) + (going==DOWN?1:0), other_gate.x + (going==LEFT?-1:0) + (going==RIGHT?1:0)};
        if (can_go(go))
            return move_and_get_item(go);
        game_over = true;
        return false;
    }
    bool move(int direction) {
        switch (direction) {
            case UP:
                if (going != DOWN){
                    going = UP;
                    break;
                }
                else{
                    game_over = true;
                    return false;
                } 
            case DOWN:
                if (going != UP){
                    going = DOWN; 
                    break;
                } 
                else{
                    game_over = true;
                    return false;
                } 
            case LEFT:
                if (going != RIGHT){
                    going = LEFT;
                    break;
                }
                else{
                    game_over = true;
                    return false;
                } 
            case RIGHT:
                if (going != LEFT){
                    going = RIGHT;
                    break;
                } 
                else{
                    game_over = true;
                    return false;
                } 
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
                return move_and_get_item(go);
            } else {
                game_over = true;
                return false;
            }
        }
        return true;
    }
    bool move_and_get_item(pos go) {
        int go_tile = map[go.y][go.x];
        if (go_tile == EMPTY) {
            // Basic move
            pos tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;

            body.push_front(go);
            map[go.y][go.x] = BODY;
        } else if (go_tile == GROWTH) {
            sc_growth += 1;
            current_length += 1;
            max_length = current_length>max_length?current_length:max_length;
            if(mission_check(go_tile)) return false;
            body.push_front(go);
            map[go.y][go.x] = BODY;
            if (gate_cooltime>=0) gate_cooltime++;
        } else if (go_tile == POISON) {
            sc_poison += 1;
            current_length -= 1;
            if(mission_check(go_tile)) return false;
            pos tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;

            tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;
            
            body.push_front(go);
            map[go.y][go.x] = BODY;
            if (current_length<3){
                game_over = true;
                return false;
            }
            if (gate_cooltime>0) gate_cooltime--;
        } else if (go_tile == GATE1 || go_tile == GATE2) {
            sc_gate += 1;
            if(mission_check(go_tile)) return false;
            pos now_gate = gate_pos[go_tile-GATE1];
            pos other_gate = gate_pos[GATE2-go_tile];
            return using_gate(now_gate, other_gate);
        }
        return true;
    }
    bool can_go(pos go) {
        if ((map[go.y][go.x] == WALL) || (map[go.y][go.x] == IWALL) || (map[go.y][go.x] == BODY)){
            game_over = true;
            return false;
        }
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
    WINDOW* score_win;
    WINDOW* mission_win;

    initscr();
    start_color();
    
    init_pair(BKGRD, COLOR_BLACK, 
    COLOR_BLACK);
    border('*', '*', '*', '*', '*', '*', '*', '*');
    refresh();

    game_win = newwin(22, 42, 1, 1);
    wbkgd(game_win, COLOR_PAIR(1));
    mvwprintw(game_win, 1, 1, "A new window");
    wrefresh(game_win);

    init_pair(INFO, COLOR_WHITE, COLOR_BLACK);
    score_win = newwin(8, 30, 3, 47);
    wbkgd(score_win, COLOR_PAIR(INFO));
    wattron(score_win, COLOR_PAIR(INFO));
    wborder(score_win, '@','@','@','@','@','@','@','@');
    wrefresh(score_win);

    init_pair(MISSION, COLOR_WHITE, COLOR_CYAN);
    mission_win = newwin(8, 30, 12, 47);
    wbkgd(mission_win, COLOR_PAIR(MISSION));
    wattron(mission_win, COLOR_PAIR(MISSION));
    wborder(mission_win, '@','@','@','@','@','@','@','@');
    wrefresh(mission_win);

    keypad(stdscr, TRUE);
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);

    for(int i = 1; i<=5; i++){
        // Game setting
        Game game(game_win, score_win, mission_win);
        int lastinput = NONE;
        int lasttime = getms();
        game.init("maps/" + to_string(i));

        // Main loop
        while (true) {
            int i = getch();
            if (258<=i && i<=261) lastinput = i - 258;
            int now = getms();
            int dt = now - lasttime;
            if (dt>=300) {
                if (!game.tick(lastinput)) {
                    // Game over
                    break;
                }
                game.draw();

                wrefresh(game_win);
                lasttime = now;
                lastinput = NONE;
            }
        }

        if(game.game_over) break;

        getch();
    }
    delwin(game_win);
    endwin();
    return 0;
}