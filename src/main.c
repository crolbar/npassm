#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>

#include "app.h"

#include "group/group.c"
#include "group/entry/entry.c"
#include "update.c"
#include "ui.c"

void init_windows(struct App* app) {
    erase();

    app->group_pane.win = newwin(LINES, COLS * 0.15, 0, 0);
    app->entry_pane  = (struct EntryPane) {
        .win = newwin(LINES, COLS * 0.35, 0, COLS * 0.15),
        .info_win = newwin(LINES, COLS * 0.50, 0, COLS * 0.50), 
    };

    refresh();
}

void init_entries(struct Group* group) {
    group->entries[0] = (struct Entry) {
        .title = "gmail",
        .username = "g username",
        .email = "g@gmail.com",
        .password = "12345",
        .notes = "gmail notes aosntehu aosentu aosneuq",
        
    };
    group->entries[1] = (struct Entry) {
        .title = "proton",
        .username = "p username",
        .email = "p@proton.me",
        .password = "12345",
        .notes = "proton notes aosntehu aosentu aosneuq",
    };
    group->entries[2] = (struct Entry) {
        .title = "discord",
        .username = "d username",
        .email = "d@gmail.com",
        .password = "12345",
        .notes = "discord notes aosntehu aosentu aosneuq",
    };
}

void init_groups(struct GroupPane* gp) {
    gp->num_groups = 4;
    gp->groups = (struct Group*)malloc(4 * sizeof(struct Group));

    gp->groups[0] = (struct Group) {
        .name = "first",
        .sel_entry = 0,
        .num_entries = 3,
        .entries = (struct Entry*)malloc(3 * sizeof(struct Entry))
    };
    init_entries(&gp->groups[0]);

    gp->groups[1] = (struct Group){ .name = "sec" };
    gp->groups[2] = (struct Group) {
        .name = "third",
        .sel_entry = 0,
        .num_entries = 3,
        .entries = (struct Entry*)malloc(3 * sizeof(struct Entry))
    };
    init_entries(&gp->groups[2]);

    gp->groups[3] = (struct Group){ .name = "fourth" };
}

struct App init_app() {
    struct App app = {
        .exit = false,
        .active_pane = Group,
        .group_pane = (struct GroupPane) {
            .num_groups = 0,
            .groups = NULL,
            .sel = 0,
        },
    };

    init_groups(&app.group_pane);
    init_windows(&app);

    return app;
}


int main() {
    initscr();
    cbreak();
    noecho();
    start_color();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);

    init_pair(1, COLOR_RED, COLOR_BLACK);

    struct App app = init_app();

    while (!app.exit) {
        draw(&app);
        update(&app);
    }

    endwin();
    return 0;
}
