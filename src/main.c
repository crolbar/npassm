#include <ncurses.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>

#include "app.h"

#include "dialogbox.c"
#include "group/group.c"
#include "entry/entry.c"
#include "update.c"
#include "ui.c"
#include "npassdb.c"
#include "passgen.c"

void init_dialogbox_windows(struct DialogBox* db) {
    const int DIALOGBOX_WIN_HEIGHT = LINES * 0.25;
    const int DIALOGBOX_WIN_WIDTH = COLS * 0.25;
    const int DIALOGBOX_WIN_START_Y = LINES * 0.37;
    const int DIALOGBOX_WIN_START_X = COLS * 0.38;
    
    const int INPUT_BOX_BORDER_WIN_HEIGHT = 3;
    const int INPUT_BOX_BORDER_WIN_WIDTH = (COLS * 0.25) - 4;
    const int INPUT_BOX_BORDER_WIN_START_Y = (LINES * 0.37) + 4;
    const int INPUT_BOX_BORDER_WIN_START_X = (COLS * 0.38) + 2;

    const int INPUT_BOX_WIN_HEIGHT = 1;
    const int INPUT_BOX_WIN_WIDTH = (COLS * 0.25) - 6;
    const int INPUT_BOX_WIN_START_Y = (LINES * 0.37) + 5;
    const int INPUT_BOX_WIN_START_X = (COLS * 0.38) + 3;

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

    db->confirm_yes_win = derwin(db->win,
            3,
            10,
            DIALOGBOX_WIN_HEIGHT / 2,
            DIALOGBOX_WIN_WIDTH * 0.25
    );

    db->confirm_no_win = derwin(db->win,
            3,
            10,
            DIALOGBOX_WIN_HEIGHT / 2,
            (DIALOGBOX_WIN_WIDTH * 0.75) - 10
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
    const int INFO_WIN_WIDTH = COLS * 0.49;
    const int INFO_WIN_START_Y = 0;
    const int INFO_WIN_START_X = COLS * 0.51;

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

    const int PASSGEN_WIN_HEIGHT = LINES / 2;
    const int PASSGEN_WIN_WIDTH = COLS * 0.25;
    const int PASSGEN_WIN_START_Y = LINES * 0.25;
    const int PASSGEN_WIN_START_X = COLS * 0.37;

    app->passgen.win = newwin(
            PASSGEN_WIN_HEIGHT,
            PASSGEN_WIN_WIDTH,
            PASSGEN_WIN_START_Y,
            PASSGEN_WIN_START_X
    );

    app->passgen.pass_border_win = newwin(
            5,
            PASSGEN_WIN_WIDTH - 10,
            PASSGEN_WIN_START_Y + 4,
            PASSGEN_WIN_START_X + 5
    );

    app->passgen.pass_win = newwin(
            3,
            PASSGEN_WIN_WIDTH - 12,
            PASSGEN_WIN_START_Y + 5,
            PASSGEN_WIN_START_X + 6
    );

    refresh();
}

char* handle_args(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            printf(
                "npassm, a ncurses tui password manager!\n\n"
                "Usage: npassm <PATH>\n\n"
                "Provide a path to a db file created by npassm,\n or a non existent file and it will create a db file.\n"
            );
            exit(0);
        }
    }

    char* path = argv[1];
    char* parent_path = dirname(strdup(path));

    struct stat statbuf;
    struct stat p_statbuf;

    // path exists
    bool exists = !stat(path, &statbuf);

    // path is not dir
    if (S_ISDIR(statbuf.st_mode)) {
        printf("Path is a directory.\n");
        exit(0);
    }
    
    // parent exists
    if (stat(parent_path, &p_statbuf) != 0) {
        printf("Invalid path.\n");
        exit(0);
    }

    // parent is dir
    if (!S_ISDIR(p_statbuf.st_mode)) {
        printf("Invalid path. Parent is not an directory.\n");
        exit(0);
    }


    if (!exists) {
        struct App a = init_app(path);
        save_db(&a);
    } else if (!is_npassdb(path)) {
        printf("Wrong file format.\n");
        exit(0);
    }

    return path;
}

int main(int argc, char** argv) {
    char* path = handle_args(argc, argv);

    initscr();
    cbreak();
    raw();
    noecho();
    start_color();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(200);
    srand(time(NULL));

    init_pair(1, 1, 0);
    init_pair(2, 8, 0);
    init_pair(3, 1, 5);

    struct App app = open_db(path);

    do {
        update(&app);
        draw(&app);
    } while (!app.exit);

    endwin();
    return 0;
}
