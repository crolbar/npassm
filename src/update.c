#include <ncurses.h>
#include "app.h"
#include "group/entry/entry.h"

void handle_pane_focus_change(struct App* app, bool left) {
    if (!left) {
        if (
                app->active_pane == Group &&
                app->group_pane.groups[app->group_pane.sel].num_entries != 0
           ) 
        {
            app->active_pane = Entry;

            app->group_pane.win = newwin(LINES, COLS * 0.10, 0, 0);
            app->entry_pane.win = newwin(LINES, COLS * 0.40, 0, COLS * 0.10);

        } else if (app->active_pane == Entry) 
        {
            app->active_pane = EntryFields;
        }
    } else {
        if (app->active_pane == Entry) 
        {
            app->active_pane = Group;

            app->group_pane.win = newwin(LINES, COLS * 0.15, 0, 0);
            app->entry_pane.win = newwin(LINES, COLS * 0.35, 0, COLS * 0.15);

        } else if (app->active_pane == EntryFields) 
        {
            app->active_pane = Entry;
        }
    }

}

void update(struct App *app) {
    switch (getch()) {

        case KEY_UP:
        case 'k':
            if (app->active_pane == Group) {
                group_select_prev(&app->group_pane.sel);
            } else if (app->active_pane == Entry) {
                entry_sel_prev(&app->group_pane.groups[app->group_pane.sel].sel_entry);
            } else if (app->active_pane == EntryFields) {
                entry_field_sel_prev(&app->entry_pane.sel_field);
            }

            break;

        case KEY_DOWN:
        case 'j':
            if (app->active_pane == Group) {
                group_select_next(&app->group_pane);
            } else if (app->active_pane == Entry) {
                entry_sel_next(&app->group_pane.groups[app->group_pane.sel]);
            } else if (app->active_pane == EntryFields) {
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

        case KEY_RESIZE:
            init_windows(app);
            break;
    }
}
