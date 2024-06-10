#include "app.h"
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

bool render_bialogbox(struct DialogBox* db) {
    {
        WINDOW* win = db->win;

        wattron(win, COLOR_PAIR(1));

        box(win, 0, 0);
        mvwprintw(win, 0, 1, "Dialog Box");
        wattroff(win, COLOR_PAIR(1));

        mvwprintw(win,
            2,
            (getmaxx(win) / 2) - strlen(db->title) / 2,
            "%s", db->title
        );

        wnoutrefresh(win);
    }

    {
        WINDOW* input_box_border_win = db->input_box_border_win;

        wattron(input_box_border_win, COLOR_PAIR(2));
        box(input_box_border_win, 0, 0);
        wattroff(input_box_border_win, COLOR_PAIR(2));

        wnoutrefresh(input_box_border_win);
    }

    
    {
        WINDOW* input_box_win = db->input_box_win;
        werase(input_box_win);

        mvwprintw(
            input_box_win,
            0, 0,
            "%s", db->mod_str
        );

        wnoutrefresh(input_box_win);
    }

    return true;
}

void size_down_db_windows(struct DialogBox* db) {
    int h, w;
    getmaxyx(db->input_box_win, h, w);

    if (!db->risized) db->risized = true;

    wresize(db->input_box_win, --h, w);

    getmaxyx(db->input_box_border_win, h, w);
    wresize(db->input_box_border_win, --h, w);

    getmaxyx(db->win, h, w);
    wresize(db->win, --h, w);
}

void mod_str_pop(struct DialogBox* db, bool word) {
    int l = strlen(db->mod_str);

    if (db->mod_str[l - 1] == '\n') {
        size_down_db_windows(db);
    }

    if (word) {
        for (l -= 1; l > 0; l--) {
            if (db->mod_str[l] == ' ' || db->mod_str[l] == '\n') {
                break;
            } 
        }
    }

    // allows for deleting the first word
    if (l == 0) l += 1; 

    if (l > 0) {
        char* str = (char*)malloc(l * sizeof(char));
        strncpy(str, db->mod_str, l - 1);

        str[l - 1] = '\0';

        free(db->mod_str);
        db->mod_str = str;
    }

    werase(db->win);
    wnoutrefresh(db->win);
}

void handle_keypress(struct DialogBox* db, char c) {
    int l = strlen(db->mod_str);

    char* str = (char*)malloc((l + 2) * sizeof(char));
    if (str == NULL) exit(EXIT_FAILURE);

    strcpy(str, db->mod_str);
    str[l] = c;
    str[l + 1] = '\0';

    free(db->mod_str);
    db->mod_str = str;

    werase(db->win);
    wnoutrefresh(db->win);

    { // increase window height if we reach the end of the line of we input a new line
        int h, w;
        getmaxyx(db->input_box_win, h, w);

        int curr_line_len;
        if (strstr(db->mod_str, "\n")) {
            for (int i = strlen(db->mod_str); i >= 0; i--) {
                if (db->mod_str[i] == '\n') {
                    curr_line_len = (strlen(db->mod_str) - i) % w;
                    break;
                }
            }
        } else {
            curr_line_len = strlen(db->mod_str) % w;
        }

        if (!curr_line_len || c == '\n') {
            if (!db->risized) db->risized = true;

            wresize(db->input_box_win, ++h, w);

            getmaxyx(db->input_box_border_win, h, w);
            wresize(db->input_box_border_win, ++h, w);

            getmaxyx(db->win, h, w);
            wresize(db->win, ++h, w);
        }
    }
}

void start_editing(struct Panes* panes, struct DialogBox* db, char** origin) {
    panes->prev_active = panes->active;
    panes->active = DialogBox;
    db->origin_str = origin;
    db->mod_str = strdup(*origin);
    curs_set(1);

    if (db->risized) {
        db->risized = false;
        init_dialogbox_windows(db);
    }


    { // increase window height based on lines that wrap around (and how much times an line wrap around) and new lines
        int h, w;
        getmaxyx(db->input_box_win, h, w);

        int nl_num = 0;
        int sub_str_wrap_num = 0;

        int tmp = 0;

        for (int i = 0; i < strlen(db->mod_str); i++) {
            if (db->mod_str[i] == '\n') {
                nl_num++;

                if (i - tmp >= w) {
                    sub_str_wrap_num += (i - tmp) / w;
                }

                tmp = i;
            }
        }
        if (strlen(db->mod_str) - tmp >= w) {
            sub_str_wrap_num += (strlen(db->mod_str) - tmp) / w;
        }

        if (sub_str_wrap_num || nl_num) {
            if (!db->risized) {
                db->risized = true;
            }

            int i = sub_str_wrap_num + nl_num;

            wresize(db->input_box_win, i + 1, w);

            getmaxyx(db->input_box_border_win, h, w);
            wresize(db->input_box_border_win, h += i, w);

            getmaxyx(db->win, h, w);
            wresize(db->win, h += i, w);
        }
    }
}

void stop_editing(struct Panes* panes, struct DialogBox* db, WINDOW* gw, bool save) {
    panes->active = panes->prev_active;

    if (save) {
        int l = strlen(db->mod_str);

        *db->origin_str = (char*)malloc((l + 1) * sizeof(char));

        strcpy(*db->origin_str, db->mod_str);
        werase(gw);
        wnoutrefresh(gw);
    }

    free(db->mod_str);
    db->mod_str = NULL;
    db->origin_str = NULL;

    werase(db->win);
    wnoutrefresh(db->win);
    curs_set(0);
}
