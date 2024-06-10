#ifndef DIALOGBOX
#define DIALOGBOX

#include <ncurses.h>

struct DialogBox {
    WINDOW* win;
    WINDOW* input_box_win;
    WINDOW* input_box_border_win;

    bool risized;

    char* title;

    char** origin_str;
    char* mod_str;
};
struct App;
struct Panes;

bool render_bialogbox(struct DialogBox* db);
void start_editing(struct Panes* panes, struct DialogBox* db, char** origin);
void stop_editing(struct Panes* panes, struct DialogBox* db, WINDOW* gw, bool save);
void handle_keypress(struct DialogBox* db, char c);
void mod_str_pop(struct DialogBox* db, bool word);

#endif
