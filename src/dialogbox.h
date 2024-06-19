#ifndef DIALOGBOX
#define DIALOGBOX

#include <ncurses.h>

struct DialogBox {
    WINDOW* win;

    bool is_editing;

    WINDOW* confirm_yes_win;
    WINDOW* confirm_no_win;
    bool is_yes;
    char op;

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
void stop_editing(struct App* app, bool save);
void handle_keypress(struct DialogBox* db, char c);
void mod_str_pop(struct DialogBox* db, bool word);
void set_dialogbox_title(struct App* app, char c);

void start_confirm(struct Panes* panes, struct DialogBox* db, char op);
void stop_confirm(struct App* app, bool cancel);

#endif
