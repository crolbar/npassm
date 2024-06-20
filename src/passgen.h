#ifndef PASSGEN
#define PASSGEN

#include <ncurses.h>

struct Passgen {
    WINDOW* win;
    WINDOW* pass_border_win;
    WINDOW* pass_win;


    char* title;

    int sel;

    int slider;

    bool numbers;
    bool symbols;

    char* genpassword;
};

void start_passgen(struct App* app);
void stop_passgen(struct App* app);
void handle_enter_passgen(struct App* app);
void set_password(struct App* app);

#endif
