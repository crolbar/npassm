#ifndef ENTRY
#define ENTRY

#include <ncurses.h>

struct App;

struct EntryPane {
    WINDOW* win;
    WINDOW* info_win;
    int sel_field;
};

struct Entry {
    char* title;
    char* username;
    char* email;
    char* password;
    char* notes;
};

void entry_sel_prev(int* sel_entry);
void entry_sel_next(struct Group* g);
void entry_field_sel_next(struct EntryPane* ep);
void entry_field_sel_prev(int* sel_field);

#endif
