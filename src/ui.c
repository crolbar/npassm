#include "app.h"
#include <ncurses.h>

void draw(struct App* app) {
    if (render_bars(app)) { }
    if (render_groups(app)) { }
    if (app->panes.active == PaneDialogBox) {
        render_bialogbox(&app->dialogbox);
    }

    if (app->panes.active == PanePassgen) {
        render_passgen(app);
    }

    doupdate();
}
