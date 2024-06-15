#ifndef ENTRY
#define ENTRY

#include <ncurses.h>

struct App;

struct EntryPane {
    WINDOW* win;
    WINDOW* info_win;
    WINDOW* field_windows[4];
    bool pass_hiden;
};

struct Entry {
    int sel_field;
    char* name;
    char* username;
    char* email;
    char* password;
    char* notes;
};

void entry_sel_prev(int* sel_entry);
void entry_sel_next(struct Group* g);
void entry_field_sel_next(int* sel_field);
void entry_field_sel_prev(int* sel_field);
void entry_add(struct EntryPane* ep, struct Group* g);
void entry_remove(struct EntryPane* ep, struct Group* g);

#endif
