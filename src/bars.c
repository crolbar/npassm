#include "app.h"
#include <stdlib.h>
#include <string.h>

bool render_bars(struct App* app) {
    WINDOW* tbw = app->top_bar_win;
    WINDOW* bbw = app->btm_bar_win;


    {
        wattron(tbw, COLOR_PAIR(1));
        mvwprintw(tbw, 0, 0, "|");
        mvwprintw(tbw, 0, getmaxx(tbw) - 1, "|");
        wattroff(tbw, COLOR_PAIR(1));

        wattron(tbw, COLOR_PAIR(2));

        char* fmt = "`$` to change db name/password | db: %s | path: %s";

        mvwprintw(tbw, 0,
                (getmaxx(tbw) - 2) - ((strlen(fmt) - 4) + strlen(app->dbname) + strlen(app->dbpath)),
                fmt, app->dbname, app->dbpath);

        wattroff(tbw, COLOR_PAIR(2));

        mvwprintw(tbw, 0, 1, "%s", app->top_bar_info);
    }



    {
        werase(bbw);

        wattron(bbw, COLOR_PAIR(1));
        mvwprintw(bbw, 0, 0, "|");
        mvwprintw(bbw, 0, getmaxx(bbw) - 1, "|");
        wattroff(bbw, COLOR_PAIR(1));

        switch (app->panes.active) {
            case PaneGroup:
                mvwprintw(bbw, 0, 1, " <hjkl> to move. <r> rename ,<a> add, <d> delete group. <c> copy sel group name.");
                break;

            case PaneEntry:
                mvwprintw(bbw, 0, 1, " <hjkl> to move. <r> rename ,<a> add, <d> delete entry. <c> copy sel entry name.");
                break;

            case PaneEntryFields:
                {
                    struct Group g = app->group_pane.groups[app->group_pane.sel];
                    switch (g.entries[g.sel_entry].sel_field) {
                        case 0:
                            mvwprintw(bbw, 0, 1, " <hjkl> to move. <r> edit username. <c> copy username.");
                            break;
                        case 1:
                            mvwprintw(bbw, 0, 1, " <hjkl> to move. <r> edit email. <c> copy email.");
                            break;
                        case 2:
                            mvwprintw(bbw, 0, 1, " <hjkl> to move. <r> edit password. <c> copy password. <g> generate random password.");
                            break;
                        case 3:
                            mvwprintw(bbw, 0, 1, " <hjkl> to move. <r> edit notes. <c> copy notes.");
                            break;
                    }
                } break;

            case PaneDialogBox:
                if (app->dialogbox.is_editing) {
                    mvwprintw(bbw, 0, 1, " <ctrl+s, Enter> to save & exit. <ctrl+x> to exit without save.  ");
                }  else {
                    mvwprintw(bbw, 0, 1, " <h> to yes. <l> to no. <Enter> confirm. ");
                }
                break;
            case PanePassgen:
                    mvwprintw(bbw, 0, 1, " <jk> move up down. <hl> increase/decrease length. <ctrl+s/[x]> exit and [don't] apply. <c> copy pass. ");
                break;
        }

        char* end = " <s> to save db. <q> to quit & save ";
        mvwprintw(bbw, 0, getmaxx(bbw) - 1 - strlen(end), "%s", end);
    }


    wnoutrefresh(tbw);
    wnoutrefresh(bbw);

    return true;
}

void set_info_msg(struct App* app, char c) {
    switch (c) {
        case 's': 
            {
                char* fmt = "database has been saved to %s";

                int size = strlen(fmt) + strlen(app->dbpath) + 1;
                app->top_bar_info = malloc(strlen(fmt) + strlen(app->dbpath) + 1);

                sprintf(app->top_bar_info, fmt, app->dbpath);
                app->top_bar_info[size] = '\0';
            } break;

        case 'd':
            {
                char* sub = "";
                char* fmt = "";
                if (app->panes.active == PaneEntry) {
                    fmt = "entry \"%s\" has been deleted";
                    struct Group g = app->group_pane.groups[app->group_pane.sel];
                    sub = g.entries[g.sel_entry].name;
                } else if (app->panes.active == PaneGroup) {
                    fmt = "group \"%s\" has been deleted";
                    sub = app->group_pane.groups[app->group_pane.sel].name;
                }

                int size = strlen(fmt) + strlen(app->dbpath) + 1;
                app->top_bar_info = malloc(size);

                sprintf(app->top_bar_info, fmt, sub);
                app->top_bar_info[size] = '\0';
            } break;

        case 'p':
            {
                char* fmt = "passwords have been %s";
                char* act = (app->entry_pane.pass_hiden) ? "hidden" : "shown";

                int size = strlen(fmt) + strlen(app->dbpath) + 1;
                app->top_bar_info = malloc(strlen(fmt) + strlen(act) + 1);

                sprintf(app->top_bar_info, fmt, act);
                app->top_bar_info[size] = '\0';
            } break;

        case 'c':
            {
                char* fmt = "";
                char* sub = "";
                char* field = "name";
                int num = 0;

                struct Group* g = &app->group_pane.groups[app->group_pane.sel];

                fmt = "%s of %s %d copied";
                if (app->panes.active == PaneGroup) {
                    sub = "group";
                    num = app->group_pane.sel;
                } else if (app->panes.active == PaneEntry) {
                    sub = "entry";
                    num = g->sel_entry;
                } else if (app->panes.active == PaneEntryFields) {
                    switch (g->entries[g->sel_entry].sel_field) {
                        case 0:
                            field = "username";
                            break;
                        case 1:
                            field = "email";
                            break;
                        case 2:
                            field = "password";
                            break;
                        case 3:
                            field = "notes";
                            break;
                    }

                    sub = "entry";
                    num = g->sel_entry;
                }


                int size = strlen(fmt) + strlen(app->dbpath) + 1;
                app->top_bar_info = malloc(strlen(fmt) + sizeof(num) + strlen(sub) + strlen(field) + 1);

                sprintf(app->top_bar_info, fmt, field, sub, num);
                app->top_bar_info[size] = '\0';
            } break;
    }
}
