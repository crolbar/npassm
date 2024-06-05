#ifndef GROUP
#define GROUP

#include <ncurses.h>

struct GroupPane {
    int num_groups;
    struct Group* groups;
    int sel;

    WINDOW* win;
};

struct Group {
    char* name;

    int num_entries;
    struct Entry* entries;
    int sel_entry;
};

void group_select_prev(int* group_sel);
void group_select_next(struct GroupPane* gp);

#endif
