#include "app.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>


struct Init {
    WINDOW* win;
    WINDOW* pass_input_border_win;
    WINDOW* pass_input_win;

    WINDOW* dbname_input_border_win;
    WINDOW* dbname_input_win;

    int sel;

    char* password;
    char* dbname;

    char* title;
    char* error;
};

void render_init(const struct Init* i) {
    WINDOW* win = i->win;

    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(1));

    int h, w;
    getmaxyx(win, h, w);


    mvwprintw(win, 5, 5, "`Tab` to switch field");

    mvwprintw(win, h * 0.35, (w / 2) - strlen(i->title) / 2, "%s", i->title);

    if (i->error) {
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, h * 0.75, (w / 2) - strlen(i->error) / 2, "%s", i->error);
        wattroff(win, COLOR_PAIR(1));
    }


    {
        mvwprintw(win, (LINES / 2) - 2, (COLS / 2) - 32, "Enter name for the database");

        wattron(i->dbname_input_border_win, COLOR_PAIR(2));
        box(i->dbname_input_border_win, 0, 0);
        wattroff(i->dbname_input_border_win,  COLOR_PAIR(2));

        mvwprintw(i->dbname_input_win, 0, 0, "%s", i->dbname);
    }

    {
        mvwprintw(win, (LINES / 2) + 2, (COLS / 2) - 32, "Enter password");

        wattron(i->pass_input_border_win, COLOR_PAIR(2));
        box(i->pass_input_border_win, 0, 0);
        wattroff(i->pass_input_border_win,  COLOR_PAIR(2));

        mvwprintw(i->pass_input_win, 0, 0, "%s", i->password);
    }

    wnoutrefresh(win);

    if (i->sel) {
        wnoutrefresh(i->dbname_input_win);

        wnoutrefresh(i->pass_input_border_win);
        wnoutrefresh(i->pass_input_win);
    } else {
        wnoutrefresh(i->pass_input_win);

        wnoutrefresh(i->dbname_input_border_win);
        wnoutrefresh(i->dbname_input_win);
    }
}


int update_init(struct Init* i) {
    int c = getch();

    switch (c) {
        case 10:
            if (strlen(i->password) && strlen(i->dbname)) {
                return 1;
            } else {
                return 2;
            }
            break;

        case 9: 
        case KEY_UP:
        case KEY_DOWN:
            i->sel = !i->sel;
            break;

        case 8:
        case 127:
        case KEY_BACKSPACE:
                up_str_pop(i->win, (i->sel) ? &i->password : &i->dbname, c != KEY_BACKSPACE);
            break;

        default:
            if (c != -1) {
                if (strlen(i->password) < 64) {
                    up_str_push(i->win, (i->sel) ? &i->password : &i->dbname, c);
                }
            }
    }

    if (c == 3) {
        endwin();
        exit(0);
    };

    if (c != -1) {
        return -1;
    };

    return 0;
}

struct Init init_init(char* path, bool create) {
    char* title_fmt = "%s DB: %s";
    char* act = (create) ? "Create" : "Change name or password for";

    char* title = malloc(strlen(title_fmt) + strlen(path) + strlen(act));
    sprintf(title, title_fmt, act, path);

    char* pass = malloc(1);
    pass[0] = '\0';
    char* name = malloc(1);
    name[0] = '\0';
    return (struct Init){
        .password = pass,
        .dbname = name,
        .sel = 0,

        .title = title,
        .error = NULL,

        .win = newwin(LINES - 2, COLS - 2, 1, 1),

        .dbname_input_win = newwin(1, 64, (LINES / 2) + 1, (COLS / 2) - 31),
        .dbname_input_border_win = newwin(3, 66, LINES / 2, (COLS / 2) - 32),

        .pass_input_win = newwin(1, 64, (LINES / 2) + 5, (COLS / 2) - 31),
        .pass_input_border_win = newwin(3, 66, (LINES / 2) + 4, (COLS / 2) - 32),
    };
}

void init_loop(struct Init i, struct App* app) {
    curs_set(1);

    while (true) {
        int s = update_init(&i);
        if (s == 1) {
            erase();
            refresh();
            curs_set(0);
            free(i.title);

            app->password = i.password;
            app->dbname = i.dbname;

            break;
        } else if (s == 2) {
            i.error = "Some fields are empty";
        } else if (s == -1) {
            i.error = NULL;
        }

        render_init(&i);
        doupdate();
    }
}

struct App create_db(char* path) {
    struct Init i = init_init(path, true);
    struct App app = init_app(path);

    init_loop(i, &app);

    return app;
}

void edit_db(struct App* app) {
    struct Init i = init_init(app->dbpath, false);
    free(i.password);
    free(i.dbname);
    i.password = app->password;
    i.dbname = app->dbname;

    erase();
    init_loop(i, app);
}
