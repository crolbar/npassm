#include "app.h"
#include "ncurses.h"
#include <stdlib.h>
#include <string.h>


struct Unlock {
    WINDOW* win;
    WINDOW* pass_input_border_win;
    WINDOW* pass_input_win;

    char* password;
    char* title;
    char* error;
};

void render_unlock(const struct Unlock* u) {
    WINDOW* win = u->win;

    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(1));

    wattron(u->pass_input_border_win, COLOR_PAIR(2));
    box(u->pass_input_border_win, 0, 0);
    wattroff(u->pass_input_border_win,  COLOR_PAIR(2));

    int h, w;
    getmaxyx(win, h, w);

    mvwprintw(win, h * 0.35, (w / 2) - strlen(u->title) / 2, "%s", u->title);

    if (u->error) {
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, h * 0.75, (w / 2) - strlen(u->error) / 2, "%s", u->error);
        wattroff(win, COLOR_PAIR(1));
    }

    mvwprintw(win, (LINES / 2) - 2, (COLS / 2) - 32, "Enter password");

    mvwprintw(u->pass_input_win, 0, 0, "%s", u->password);

    wnoutrefresh(win);
    wnoutrefresh(u->pass_input_border_win);
    wnoutrefresh(u->pass_input_win);
}

int update_unlock(struct Unlock* u) {
    int c = getch();


    switch (c) {
        case 10:
            return 1;
            break;

        case 8:
        case 127:
        case KEY_BACKSPACE:
                up_str_pop(u->win, &u->password, c != KEY_BACKSPACE);
            break;

        default:
            if (c != -1) {
                if (strlen(u->password) < 64) {
                    up_str_push(u->win, &u->password, c);
                }
            }
    }

    if (c == 3) {
        endwin();
        exit(0);
    };

    if (c != -1) {
        return 2;
    };

    return 0;
}

struct Unlock init_unlock(char* path) {
    char* title_fmt = "Unlock DB: %s";

    char* title = malloc(strlen(title_fmt) + strlen(path));
    sprintf(title, title_fmt, path);

    char* pass = malloc(1);
    pass[0] = '\0';
    return (struct Unlock){
        .password = pass,

        .title = title,
        .error = NULL,

        .win = newwin(LINES - 2, COLS - 2, 1, 1),
        .pass_input_win = newwin(1, 64, (LINES / 2) + 1, (COLS / 2) - 31),
        .pass_input_border_win = newwin(3, 66, LINES / 2, (COLS / 2) - 32),
    };
}

struct App unlock_db(char* path) {
    struct Unlock u = init_unlock(path);
    struct App app = init_app(path);

    curs_set(1);

    while (true) {
        int s = update_unlock(&u);
        if (s == 1) {
            if (open_db(&app, u.password)) {
                u.error = "Wrong password";
            } else {
                erase();
                refresh();
                curs_set(0);
                app.password = u.password;

                return app;
            }
        } else if (s == 2) {
            u.error = NULL;
        }

        render_unlock(&u);
        doupdate();
    }
}
