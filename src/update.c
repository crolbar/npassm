#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <threads.h>
#include "app.h"

void handle_pane_focus_change(struct App* app, bool left) {
    if (!left) {
        if (
                app->panes.active == PaneGroup &&
                app->group_pane.num_groups
           ) 
        {
            app->panes.active = PaneEntry;

            app->group_pane.win = newwin(LINES, COLS * 0.10, 0, 0);
            app->entry_pane.win = newwin(LINES, COLS * 0.40, 0, COLS * 0.10);

        } else if 
            (
                app->panes.active == PaneEntry &&
                app->group_pane.groups[app->group_pane.sel].num_entries != 0
            ) 
        {
            app->panes.active = PaneEntryFields;
        }
    } else {
        if (app->panes.active == PaneEntry) 
        {
            app->panes.active = PaneGroup;

            app->group_pane.win = newwin(LINES, COLS * 0.15, 0, 0);
            app->entry_pane.win = newwin(LINES, COLS * 0.35, 0, COLS * 0.15);

        } else if (app->panes.active == PaneEntryFields) 
        {
            app->panes.active = PaneEntry;
        }
    }

}

char** get_focused_item(struct App* app) {
    char** str = NULL;

    if (app->group_pane.num_groups) {
        struct Group* g = &app->group_pane.groups[app->group_pane.sel];
        struct Entry* e = NULL;

        if (g->num_entries) {
             e = &g->entries[g->sel_entry];
        }

        switch (app->panes.active) {
            case PaneGroup:
                str = &g->name;
                break;
            case PaneEntry:
                if (e)
                    str = &e->name;
                break;
            case PaneEntryFields:
                switch (e->sel_field) {
                    case 0:
                        str = &e->username;
                        break;
                    case 1:
                        str = &e->email;
                        break;
                    case 2:
                        str = &e->password;
                        break;
                    case 3:
                        str = &e->notes;
                        break;
                }
                break;
            default:
                break;
        }
    }
    return str;
}

void copy(const char *text) {
    FILE *p;
    if (getenv("WAYLAND_DISPLAY")) {
        p = popen("wl-copy", "w");
    } else {
        p = popen("xclip -selection clipboard", "w");
    }

    fprintf(p, "%s", text);
    pclose(p);
}

void update(struct App* app) {
    int c = getch();

    if (app->panes.active != PaneDialogBox) {
        switch (c) {

            case KEY_UP:
            case 'k':
                if (app->panes.active == PaneGroup) {
                    group_select_prev(&app->group_pane.sel);
                } else if (app->panes.active == PaneEntry) {
                    entry_sel_prev(&app->group_pane.groups[app->group_pane.sel].sel_entry);
                } else if (app->panes.active == PaneEntryFields) {
                    struct Group* g = &app->group_pane.groups[app->group_pane.sel];
                    entry_field_sel_prev(&g->entries[g->sel_entry].sel_field);
                }

                werase(app->entry_pane.info_win);
                werase(app->entry_pane.win);
                wnoutrefresh(app->entry_pane.info_win);
                wnoutrefresh(app->entry_pane.win);

                break;

            case KEY_DOWN:
            case 'j':
                if (app->panes.active == PaneGroup) {
                    group_select_next(&app->group_pane);
                } else if (app->panes.active == PaneEntry) {
                    entry_sel_next(&app->group_pane.groups[app->group_pane.sel]);
                } else if (app->panes.active == PaneEntryFields) {
                    struct Group* g = &app->group_pane.groups[app->group_pane.sel];
                    entry_field_sel_next(&g->entries[g->sel_entry].sel_field);
                }
                
                werase(app->entry_pane.info_win);
                werase(app->entry_pane.win);
                wnoutrefresh(app->entry_pane.info_win);
                wnoutrefresh(app->entry_pane.win);

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

            case 's':
                save_db(app);
                break;

            case 'd':
                if (app->panes.active != PaneEntryFields) {
                    set_dialogbox_title(app, c);
                    start_confirm(&app->panes, &app->dialogbox,  c);
                }
                break;

            case 'p':
                app->entry_pane.pass_hiden = !app->entry_pane.pass_hiden;

                werase(app->entry_pane.info_win);
                wnoutrefresh(app->entry_pane.info_win);
                break;

            case 'c':
                copy(*get_focused_item(app));
                break;

            case 'a':
                if (app->panes.active == PaneEntry) {
                    entry_add(&app->entry_pane, &app->group_pane.groups[app->group_pane.sel]);
                } else if (app->panes.active == PaneGroup) {
                    group_add(&app->group_pane);
                }

            case 'r': 
                {
                    char** str = get_focused_item(app);

                    if (str) {
                        set_dialogbox_title(app, c);
                        start_editing(&app->panes, &app->dialogbox, str);
                    }
                } break;
        }
    } else {
        if (app->dialogbox.is_editing) {
            switch (c) {
                case 19: /*ctrl+s*/ case 24: /*ctrl+x*/ case 27: /*esc*/
                    stop_editing(app, c == 19);
                    break;

                case 8:
                case KEY_BACKSPACE:
                    mod_str_pop(&app->dialogbox, c == 8);
                    break;

                //me no likey tab
                case 9: break;

                case 10:
                    {
                        struct Group g = app->group_pane.groups[app->group_pane.sel];
                        if (
                                app->panes.prev_active != PaneEntryFields ||
                                g.entries[g.sel_entry].sel_field != 3
                           )
                        {
                            stop_editing(app, true);
                            break;
                        }
                    }

                default: 
                    if (c != -1) {
                        if (
                                (
                                 app->panes.prev_active == PaneGroup &&
                                 strlen(app->dialogbox.mod_str) == 11
                                ) ||
                                (
                                 app->panes.prev_active == PaneEntry &&
                                 strlen(app->dialogbox.mod_str) == 32
                                )
                           ) 
                        { break; }

                        handle_keypress(&app->dialogbox, c);
                    }
            }
        } else {
            switch (c) {
                case 'l':
                case KEY_RIGHT:
                    app->dialogbox.is_yes = false;
                    break;
                case 'h':
                case KEY_LEFT:
                    app->dialogbox.is_yes = true;
                    break;

                case 27:
                case 24:
                case 10:
                    stop_confirm(app, c != 10);
                    break;
            }
        }
    }

    if (c == KEY_RESIZE) init_windows(app);
    if (c == 3) app->exit = true;
}
