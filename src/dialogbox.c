#include "app.h"
#include "group/entry/entry.h"
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
        werase(db->win);
        wnoutrefresh(db->win);
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
int get_curr_line_len(char* str, int w) {
    if (strstr(str, "\n")) {
        for (int i = strlen(str); i >= 0; i--) {
            if (str[i] == '\n') {
                return (strlen(str) - i) % w;
            }
        }
    }

    return strlen(str) % w;
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

    { // increase window height if we reach the end of the line or we input a new line
        if (
            !get_curr_line_len(db->mod_str,
                    getmaxx(db->input_box_win))
            || c == '\n'
           ) 
        {
            int h, w;
            getmaxyx(db->input_box_win, h, w);

            if (!db->risized) db->risized = true;

            wresize(db->input_box_win, ++h, w);

            getmaxyx(db->input_box_border_win, h, w);
            wresize(db->input_box_border_win, ++h, w);

            getmaxyx(db->win, h, w);
            wresize(db->win, ++h, w);
        }
    }
}

void set_dialogbox_title(struct App* app, bool renaming) {
    char* action;
    char* sub;

    struct Group g = app->group_pane.groups[app->group_pane.sel];
    struct Entry e;
    if (g.num_entries) {
        e = g.entries[g.sel_entry];
    }
    
    switch (app->panes.active) {
        case Group: 
            {
                if (renaming) {
                    action = "Rename Group";
                } else {
                    action = "Set name for";
                };

                sub = g.name;
            } break;
        case Entry:
            if (renaming) {
                action = "Rename Entry";
            } else {
                action = "Set name for";
            }

            sub = e.name;

            break;
        case EntryFields:
            switch (e.sel_field) {
                case 0:
                    action = "Set username for";
                    break;
                case 1:
                    action = "Set email for";
                    break;
                case 2:
                    action = "Set password for";
                    break;
                case 3:
                    action = "Set notes for";
                    break;
            }

            sub = e.name;

            break;
        default: break;
    }

    char tmp[300];
    sprintf(tmp, "%s %s", action, sub);

    int window_width = getmaxx(app->dialogbox.win);
    if (strlen(tmp) > window_width - 2) {
        tmp[window_width - 7] = '.';
        tmp[window_width - 6] = '.';
        tmp[window_width - 5] = '\0';
    }

    app->dialogbox.title = (char*)malloc((strlen(tmp) + 1) * sizeof(char));
    strcpy(app->dialogbox.title, tmp);
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

void stop_editing(struct App* app, bool save) {
    if (
            !strlen(app->dialogbox.mod_str) &&
            app->panes.prev_active != EntryFields
       )
    {
        return;
    }

    app->panes.active = app->panes.prev_active;

    if (save) {
        int l = strlen(app->dialogbox.mod_str);

        *app->dialogbox.origin_str = (char*)malloc(++l * sizeof(char));

        strcpy(*app->dialogbox.origin_str, app->dialogbox.mod_str);

        WINDOW* wins[] = {
            app->group_pane.win,
            app->entry_pane.win,
            app->entry_pane.info_win
        };

        for (int i = 0; i < 3; i++) {
            werase(wins[i]);
            wrefresh(wins[i]);
        }
    }

    free(app->dialogbox.mod_str);
    app->dialogbox.mod_str = NULL;
    app->dialogbox.origin_str = NULL;

    werase(app->dialogbox.win);
    wnoutrefresh(app->dialogbox.win);
    curs_set(0);
}
