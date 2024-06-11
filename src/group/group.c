#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../app.h"
#include "ncurses.h"

bool render_groups(struct App *app) {
    WINDOW* win = app->group_pane.win;

    if (app->panes.active == Group) {
        wattron(win, COLOR_PAIR(1));
    }

    box(win, 0, 0);
    mvwprintw(win, 0, 1, "Groups");

    wattroff(win, COLOR_PAIR(1));

    for (int i = 0; i < app->group_pane.num_groups; i++) {
        struct Group g = app->group_pane.groups[i];

        if (i == app->group_pane.sel) {
            wattron(win, COLOR_PAIR(1));

            if (g.num_entries == 0) {
                werase(app->entry_pane.win); werase(app->entry_pane.info_win);
                wnoutrefresh(app->entry_pane.win); wnoutrefresh(app->entry_pane.info_win);
            }

            if (app->group_pane.num_groups) {
                if (render_entry_pane(&app->entry_pane, app->panes.active, &g)) {}
            }
        }

        mvwprintw(
            win,
            2 + i * 2,
            2,
            "%s", g.name
        );


        wattroff(win, COLOR_PAIR(1));
    }

    wnoutrefresh(win);
    return true;
}

void group_remove(struct GroupPane* gp) {
    if (gp->num_groups) {
        for (int i = gp->sel; i < gp->num_groups; i++) {
            gp->groups[i] = gp->groups[i + 1];
        }

        gp->groups = realloc(gp->groups, (gp->num_groups - 1) * sizeof(struct Group));

        gp->num_groups--;

        if (gp->sel >= gp->num_groups) {
            gp->sel = gp->num_groups - 1;
        }

        werase(gp->win);
        wrefresh(gp->win);
    }
}

void group_add(struct GroupPane* gp) {
    gp->sel = gp->num_groups;
    gp->groups = realloc(gp->groups, (gp->num_groups + 1) * sizeof(struct Group));

    gp->groups[gp->num_groups] = (struct Group){
        .sel_entry = 0,
        .num_entries = 0,
        .entries = malloc(sizeof(struct Entry)),
        .name = malloc(20 * sizeof(char)),
    };

    sprintf(gp->groups[gp->num_groups].name, "New Group %d", gp->num_groups);

    gp->num_groups++;
    werase(gp->win);
    wnoutrefresh(gp->win);
}

void group_select_prev(int* group_sel) {
    if (*group_sel > 0) *group_sel -= 1;
}

void group_select_next(struct GroupPane* gp) {
    if (gp->sel < gp->num_groups - 1) gp->sel++;
}
