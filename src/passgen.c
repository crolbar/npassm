#include <math.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "app.h"

bool render_passgen(const struct App* app) {
    WINDOW* win = app->passgen.win;

    werase(win);

    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(1));

    wattron(app->passgen.pass_border_win, COLOR_PAIR(2));
    box(app->passgen.pass_border_win, 0, 0);
    wattroff(app->passgen.pass_border_win, COLOR_PAIR(2));

    mvwprintw(win, 2, 5, "%s", app->passgen.title);

    mvwprintw(app->passgen.pass_win, 0, 0, "%s", app->passgen.genpassword);


    mvwprintw(win, 10, 5, "Length: ");
    for (int j = 1; j <= 16; j++) {
        if (app->passgen.slider >= j) {
            mvwprintw(win, 10, 12 + j, "#");
        } else {
            mvwprintw(win, 10, 12 + j, "-");
        } 
    }
    mvwprintw(win, 10, 12 + 18, "%d", app->passgen.slider * 8);

    mvwprintw(win, 10, 12 + 18 + 5, "Entropy: %.2f*",
            (app->passgen.slider * 8) * log2(52 + (10 * app->passgen.numbers) + (28 * app->passgen.symbols)));


    for (int i = 0; i < 4; i++) {
        if (app->passgen.sel == i) {
            wattron(win, COLOR_PAIR(1));
        }
        switch (i) {
            case 0:
                mvwprintw(win, (i + 7) * 2, 5, "Regenerate");
                break;

            case 1:
                mvwprintw(win, (i + 7) * 2, 5, "[%c] Numbers", (app->passgen.numbers) ? 'x' : ' ');
                break;

            case 2:
                mvwprintw(win, (i + 7) * 2, 5, "[%c] Symbols", (app->passgen.symbols) ? 'x' : ' ');
                break;

            case 3:
                mvwprintw(win, (i + 7) * 2, 5, "Apply");
                break;
        }
        wattroff(win, COLOR_PAIR(1));
    }


    wnoutrefresh(win);
    wnoutrefresh(app->passgen.pass_border_win);
    wnoutrefresh(app->passgen.pass_win);
    return true;
}

static const char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const char NUMBERS[] = "1234567890";
static const char SYMBOLS[] = "!@#$%^&*()`~'\"[]{}+=\\|;:</?>";

char* generate_password(struct Passgen p) {
    int len = p.slider * 8;
    char* buf = malloc(len + 1);

    int charset_size = strlen(ALPHABET) 
        + (strlen(NUMBERS) * p.numbers) 
        + (strlen(SYMBOLS) * p.symbols);

    char* charset = malloc(charset_size + 1);

    strcpy(charset, ALPHABET);
    if (p.numbers) strcat(charset, NUMBERS);
    if (p.symbols) strcat(charset, SYMBOLS);
    
    for (int i = 0; i < len; i++) {
        buf[i] = charset[rand() % strlen(charset)];
    }

    free(charset);

    buf[len] = '\0';

    return buf;
}

void start_passgen(struct App* app) {
    struct Group g = app->group_pane.groups[app->group_pane.sel];
    struct Entry e = g.entries[g.sel_entry];


    if (
            e.sel_field == 2 &&
            app->panes.active == PaneEntryFields
       ) 
    {
        app->panes.prev_active = app->panes.active;
        app->panes.active = PanePassgen;

        char* fmt = "Set random password for entry %s";
        app->passgen.title = malloc(strlen(fmt) + strlen(e.name));
        sprintf(app->passgen.title, fmt, e.name);

        app->passgen.genpassword = generate_password(app->passgen);
    }
}

void stop_passgen(struct App* app) {
    app->panes.active = app->panes.prev_active;

    free(app->passgen.title);
    free(app->passgen.genpassword);

    werase(app->passgen.win);
    wnoutrefresh(app->passgen.win);
}

void set_password(struct App* app) {
    struct Group* g = &app->group_pane.groups[app->group_pane.sel];

    g->entries[g->sel_entry].password = malloc(strlen(app->passgen.genpassword) + 1);
    strcpy(g->entries[g->sel_entry].password, app->passgen.genpassword);

    g->entries[g->sel_entry].password[strlen(app->passgen.genpassword)] = '\0';

    stop_passgen(app);
    werase(app->entry_pane.info_win);
}

void handle_enter_passgen(struct App* app) {
    switch (app->passgen.sel) {
        case 0:
            free(app->passgen.genpassword);
            app->passgen.genpassword = generate_password(app->passgen);
            werase(app->passgen.pass_win);
            break;
        case 1:
            app->passgen.numbers = !app->passgen.numbers;
            break;
        case 2:
            app->passgen.symbols = !app->passgen.symbols;
            break;
        case 3:
            set_password(app);
            break;
    }
}
