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
    const int DIALOGBOX_WIN_HEIGHT = LINES * 0.25;
    const int DIALOGBOX_WIN_WIDTH = COLS * 0.25;
    const int DIALOGBOX_WIN_START_Y = LINES * 0.37;
    const int DIALOGBOX_WIN_START_X = COLS * 0.35;
    
    const int INPUT_BOX_BORDER_WIN_HEIGHT = 3;
    const int INPUT_BOX_BORDER_WIN_WIDTH = (COLS * 0.25) - 4;
    const int INPUT_BOX_BORDER_WIN_START_Y = (LINES * 0.37) + 4;
    const int INPUT_BOX_BORDER_WIN_START_X = (COLS * 0.35) + 2;

    const int INPUT_BOX_WIN_HEIGHT = 1;
    const int INPUT_BOX_WIN_WIDTH = (COLS * 0.25) - 6;
    const int INPUT_BOX_WIN_START_Y = (LINES * 0.37) + 5;
    const int INPUT_BOX_WIN_START_X = (COLS * 0.35) + 3;

    db->win = newwin(
            DIALOGBOX_WIN_HEIGHT,
            DIALOGBOX_WIN_WIDTH,
            DIALOGBOX_WIN_START_Y,
            DIALOGBOX_WIN_START_X
    );
    db->input_box_border_win = newwin(
            INPUT_BOX_BORDER_WIN_HEIGHT,
            INPUT_BOX_BORDER_WIN_WIDTH,
            INPUT_BOX_BORDER_WIN_START_Y,
            INPUT_BOX_BORDER_WIN_START_X
    );
    db->input_box_win = newwin(
            INPUT_BOX_WIN_HEIGHT,
            INPUT_BOX_WIN_WIDTH,
            INPUT_BOX_WIN_START_Y,
            INPUT_BOX_WIN_START_X
    );
}

void init_windows(struct App* app) {
    erase();

    const int GROUP_WIN_HEIGHT = LINES;
    const int GROUP_WIN_WIDTH = COLS * 0.15;
    const int GROUP_WIN_START_Y = 0;
    const int GROUP_WIN_START_X = 0;


    const int ENTRY_WIN_HEIGHT = LINES;
    const int ENTRY_WIN_WIDTH = COLS * 0.35;
    const int ENTRY_WIN_START_Y = 0;
    const int ENTRY_WIN_START_X = GROUP_WIN_WIDTH;

    const int INFO_WIN_HEIGHT = LINES;
    const int INFO_WIN_WIDTH = COLS * 0.50;
    const int INFO_WIN_START_Y = 0;
    const int INFO_WIN_START_X = COLS * 0.50;

    app->group_pane.win = newwin(
            GROUP_WIN_HEIGHT,
            GROUP_WIN_WIDTH,
            GROUP_WIN_START_Y,
            GROUP_WIN_START_X
    );
    app->entry_pane.win = newwin(
            ENTRY_WIN_HEIGHT,
            ENTRY_WIN_WIDTH,
            ENTRY_WIN_START_Y,
            ENTRY_WIN_START_X
    );
    app->entry_pane.info_win = newwin(
            INFO_WIN_HEIGHT,
            INFO_WIN_WIDTH,
            INFO_WIN_START_Y,
            INFO_WIN_START_X
    );

    for (int i = 0; i < 4; i++) {
        app->entry_pane.field_windows[i] = derwin(
                app->entry_pane.info_win,
                (i == 3) ? LINES * 0.50 : 1,
                INFO_WIN_WIDTH - 4,
                (i + 1) * 2,
                2
        );
    }

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
        .entry_pane = {
            .pass_hiden = true,
        },
        .dialogbox = {
            .title = NULL,
            .risized = false,
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
