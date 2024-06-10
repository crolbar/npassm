#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <threads.h>
#include "app.h"

void handle_pane_focus_change(struct App* app, bool left) {
    if (!left) {
        if (
                app->panes.active == Group &&
                app->group_pane.groups[app->group_pane.sel].num_entries != 0
           ) 
        {
            app->panes.active = Entry;

            app->group_pane.win = newwin(LINES, COLS * 0.10, 0, 0);
            app->entry_pane.win = newwin(LINES, COLS * 0.40, 0, COLS * 0.10);

        } else if (app->panes.active == Entry) 
        {
            app->panes.active = EntryFields;
        }
    } else {
        if (app->panes.active == Entry) 
        {
            app->panes.active = Group;

            app->group_pane.win = newwin(LINES, COLS * 0.15, 0, 0);
            app->entry_pane.win = newwin(LINES, COLS * 0.35, 0, COLS * 0.15);

        } else if (app->panes.active == EntryFields) 
        {
            app->panes.active = Entry;
        }
    }

}

void update(struct App *app) {
    int c = getch();

    if (app->panes.active != DialogBox) {
        switch (c) {

            case KEY_UP:
            case 'k':
                if (app->panes.active == Group) {
                    group_select_prev(&app->group_pane.sel);
                } else if (app->panes.active == Entry) {
                    entry_sel_prev(&app->group_pane.groups[app->group_pane.sel].sel_entry);
                } else if (app->panes.active == EntryFields) {
                    entry_field_sel_prev(&app->entry_pane.sel_field);
                }

                break;

            case KEY_DOWN:
            case 'j':
                if (app->panes.active == Group) {
                    group_select_next(&app->group_pane);
                } else if (app->panes.active == Entry) {
                    entry_sel_next(&app->group_pane.groups[app->group_pane.sel]);
                } else if (app->panes.active == EntryFields) {
                    entry_field_sel_next(&app->entry_pane);
                }

                break;

            case KEY_RIGHT:
            case 'l':
                handle_pane_focus_change(app, false);
                break;

            case KEY_LEFT:
            case 'h':
                handle_pane_focus_change(app, true);
                break;

            case 'q':
                app->exit = true;
                break;

            case 'r':
                start_editing(&app->panes,
                        &app->dialogbox,
                        &app->group_pane.groups[app->group_pane.sel].name);
                break;
        }
    } else {
        if (strstr(getenv("TERM"), "xterm")) {
            if (c == 127) {
                c = 263;
            } else 
            if (c == 263) {
                c = 8;
            }
        }

        switch (c) {
            case 19: /*ctrl+s*/ case 24: /*ctrl+x*/ case 27: /*esc*/
                stop_editing(&app->panes, &app->dialogbox, app->group_pane.win, c == 19);
                break;

            case 8:
            case KEY_BACKSPACE:
                mod_str_pop(&app->dialogbox, c == 8);
                break;

            //me no likey tab
            case 9: break;
            
            default:
                if (c != -1) {
                    handle_keypress(&app->dialogbox, c);
                }
        }
    }

    if (c == KEY_RESIZE) init_windows(app);
    if (c == 3) app->exit = true;
}
