#include <ncurses.h>
#include <stdbool.h>

#include "app.h"

#include "dialogbox.c"
#include "group/group.c"
#include "group/entry/entry.c"
#include "update.c"
#include "ui.c"
#include "npassdb.c"

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

    struct App app = open_db("./t/db.npassdb");

    do {
        update(&app);
        draw(&app);
    } while (!app.exit);

    endwin();
    return 0;
}
