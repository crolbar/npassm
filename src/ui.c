#include "app.h"
#include <ncurses.h>

void draw(struct App* app) {
    if (render_groups(app)) { }
    if (app->panes.active == PaneDialogBox) {
        render_bialogbox(&app->dialogbox);
    }

    doupdate();
}
