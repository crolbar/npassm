#include <stdbool.h>
#include "../app.h"

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

            if (g.num_entries > 0) {
                if (render_entry_pane(&app->entry_pane, app->panes.active, &g)) {}
            } else {
                werase(app->entry_pane.win); werase(app->entry_pane.info_win);
                wnoutrefresh(app->entry_pane.win); wnoutrefresh(app->entry_pane.info_win);
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

void group_select_prev(int* group_sel) {
    if (*group_sel > 0) *group_sel -= 1;
}

void group_select_next(struct GroupPane* gp) {
    if (gp->sel < gp->num_groups - 1) gp->sel++;
}
