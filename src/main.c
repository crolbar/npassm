#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>

#include "app.h"

#include "dialogbox.c"
#include "group/group.c"
#include "group/entry/entry.c"
#include "update.c"
#include "ui.c"

void init_dialogbox_windows(struct DialogBox* db) {
    db->win = newwin(LINES * 0.25, COLS * 0.25, LINES * 0.37, COLS * 0.35);
    db->input_box_border_win = newwin(3, (COLS * 0.25) - 4, (LINES * 0.37) + 4, (COLS * 0.35) + 2);
    db->input_box_win = newwin(1, (COLS * 0.25) - 6, (LINES * 0.37) + 5, (COLS * 0.35) + 3);
}

void init_windows(struct App* app) {
    erase();

    app->group_pane.win = newwin(LINES, COLS * 0.15, 0, 0);
    app->entry_pane  = (struct EntryPane) {
        .win = newwin(LINES, COLS * 0.35, 0, COLS * 0.15),
        .info_win = newwin(LINES, COLS * 0.49, 0, COLS * 0.50), 
    };
    init_dialogbox_windows(&app->dialogbox);

    refresh();
}

void init_entries(struct Group* group) {
    group->entries[0] = (struct Entry) {
        .name = "gmail",
        .username = "g username",
        .email = "g@gmail.com",
        .password = "12345",
        .notes = "gmail notes aosntehu aosentu aosneuq",
    };
    group->entries[1] = (struct Entry) {
        .name = "proton",
        .username = "p username",
        .email = "p@proton.me",
        .password = "12345",
        .notes = "proton notes aosntehu aosentu aosneuq",
    };
    group->entries[2] = (struct Entry) {
        .name = "discord",
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
        .panes = {
            .active = Group,
        },
        .group_pane = (struct GroupPane) {
            .num_groups = 0,
            .groups = NULL,
            .sel = 0,
        },
        .dialogbox = {
            .title = "test"
        }
    };

    init_groups(&app.group_pane);
    init_windows(&app);

    return app;
}


int main() {
    initscr();
    cbreak();
    raw();
    noecho();
    start_color();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(200);

    init_pair(1, 1, 0);
    init_pair(2, 8, 0);
    init_pair(3, 1, 5);

    struct App app = init_app();

    do {
        update(&app);
        draw(&app);
    } while (!app.exit);

    endwin();
    return 0;
}
