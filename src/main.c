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
#include "unlock_db.c"
#include "input.c"
#include "init_db.c"

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
    const int PASSGEN_WIN_WIDTH = COLS * 0.35;
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

void cleanup(struct App* app) {
    for (int i = 0; i < app->group_pane.num_groups; i++) {
        struct Group* g = &app->group_pane.groups[i];
        for (int j = 0; i < g->num_entries; i++) {
            struct Entry* e = &g->entries[i];
            free(e->name);
            free(e->username);
            free(e->email);
            free(e->password);
            free(e->notes);
        }

        free(g->entries);
        free(g->name);
    }
    free(app->group_pane.groups);
    free(app->password);
    free(app->dbname);
    free(app->dbpath);
}

char* handle_args(int argc, char** argv) {
    char* import_path = NULL;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            endwin();
            printf(
                "npassm, a ncurses tui password manager!\n\n"
                "Usage: npassm <PATH>\n\n"
                "Provide a path to a db file created by npassm,\n or a non existent file and it will create a db file.\nIf a path was not provided `$HOME/db.npassdb` will be used.\n\n"
                "Options:\n"
                "-h, --help              Print this help message\n"
                "-i, --import <PATH>     Import entries form a keepassxc csv. Cannot import to an existing npassm db.\n"
            );
            exit(0);
        }

        if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--import")) {
            if (i + 1 != argc) {
                import_path = argv[i + 1];
            }
        }
    }

    char* path = NULL;

    if (
            argc == 1 || 
            (
                !strcmp(argv[argc - 2],  "-i") ||
                !strcmp(argv[argc - 2], "--import")
            )
       ) 
    {
        char* name = "db.npassdb";
        char* home = getenv("HOME");
        int size = strlen(name) + strlen(home) + 2;
        path = malloc(size);
        sprintf(path, "%s/%s", home, name);
        path[size] = '\0';
    } else {
        path = malloc(strlen(argv[argc - 1]) + 1);
        strcpy(path, argv[argc - 1]);
        path[strlen(argv[argc - 1])] = '\0';
    }

    char* parent_path = dirname(strdup(path));


    struct stat statbuf;
    // path exists
    bool exists = !stat(path, &statbuf);
    {
        // path is not dir
        if (S_ISDIR(statbuf.st_mode)) {
            endwin();
            printf("Path is a directory.\n");
            exit(0);
        }
    }

    if (import_path) {
        struct stat i_statbuf;
        // import_path exists
        if (stat(import_path, &i_statbuf)) {
            endwin();
            printf("Import doestn't exist.\n");
            exit(0);
        }

        // import_path is not dir
        if (S_ISDIR(i_statbuf.st_mode)) {
            endwin();
            printf("Import path is a directory.\n");
            exit(0);
        }
    }


    {
        struct stat p_statbuf;
        // parent exists
        if (stat(parent_path, &p_statbuf) != 0) {
            endwin();
            printf("Invalid path.\n");
            exit(0);
        }

        // parent is dir
        if (!S_ISDIR(p_statbuf.st_mode)) {
            endwin();
            printf("Invalid path. Parent is not an directory.\n");
            exit(0);
        }
    }

    free(parent_path);

    if (!exists) {
        struct App a = create_db(path, import_path);
        save_db(&a);
    } else {
        is_npassdb(path);

        if (import_path != NULL)  {
            endwin();
            printf("If you want to import, provide a non existent file and it will create a db file with the entries from the keepassxc csv export.\n");
            exit(0);
        }
    }

    return path;
}

int main(int argc, char** argv) {
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

    char* path = handle_args(argc, argv);

    struct App app = unlock_db(path);

    do {
        update(&app);
        draw(&app);
    } while (!app.exit);

    cleanup(&app);
    endwin();
    return 0;
}
