#include "../app.h"
#include <stdlib.h>
#include <string.h>

bool render_entry_info(
        struct EntryPane* ep,
        struct Entry entry,
        enum PaneTypes active_pane) 
{
    WINDOW* win = ep->info_win;

    if (active_pane == EntryFields) {
        wattron(win, COLOR_PAIR(1));
    }
    box(win, 0, 0);
    mvwprintw(win, 0, 1, "%s", entry.name);
    wattroff(win, COLOR_PAIR(1));


    for (int i = 0; i < 4; i++) {
        if (i == entry.sel_field) {
            wattron(ep->field_windows[i], COLOR_PAIR(1));
        }
        switch (i) {
            case 0:
                mvwprintw(ep->field_windows[i], 0, 0, "Username: %s", entry.username);
                break;
            case 1:
                mvwprintw(ep->field_windows[i], 0, 0, "Email: %s", entry.email);
                break;
            case 2:
                mvwprintw(ep->field_windows[i], 0, 0, "Password: %s",
                        (ep->pass_hiden && strlen(entry.password)) ? "***********" : entry.password);
                break;
            case 3:
                mvwprintw(ep->field_windows[i], 0, 0, "Notes: %s", entry.notes);
                break;
        }
        wattroff(ep->field_windows[i], COLOR_PAIR(1));
    }



    wnoutrefresh(win);
    return true;
}

bool render_entry_pane(
        struct EntryPane* ep,
        enum PaneTypes active_pane,
        struct Group* group) 
{
    WINDOW* win = ep->win;

    if (active_pane == Entry) {
        wattron(win, COLOR_PAIR(1));
    }

    box(win, 0, 0);
    mvwprintw(win, 0, 1, "Entries from %s", group->name);

    wattroff(win, COLOR_PAIR(1));

    for (int i = 0; i < group->num_entries; i++) {
        struct Entry* e = &group->entries[i];

        if (i == group->sel_entry) {
            wattron(win, COLOR_PAIR(1));
            if (render_entry_info(ep, *e, active_pane)) {}
        }

        mvwprintw(
            win,
            2 + i * 2,
            2,
            "%s", e->name
        );

        wattroff(win, COLOR_PAIR(1));
    }

    wnoutrefresh(win);
    return true;
}

void entry_remove(struct EntryPane* ep, struct Group* g) {
    if (g->num_entries) {
        for (int i = g->sel_entry; i < g->num_entries; i++) {
            g->entries[i] = g->entries[i + 1];
        }

        g->entries = realloc(g->entries, (g->num_entries - 1) * sizeof(struct Entry));

        g->num_entries--;

        if (g->sel_entry >= g->num_entries) {
            g->sel_entry = g->num_entries - 1;
        }

        werase(ep->info_win);
        werase(ep->win);
        wnoutrefresh(ep->info_win);
        wnoutrefresh(ep->win);
    }
}

void entry_add(struct EntryPane* ep, struct Group* g) {
    g->sel_entry = g->num_entries;
    g->entries = realloc(g->entries, (g->num_entries + 1) * sizeof(struct Entry));

    g->entries[g->num_entries] = (struct Entry){
        .email = "",
        .password = "",
        .notes = "",
        .username = "",
        .name = malloc(20 * sizeof(char)),
    };

    sprintf(g->entries[g->num_entries].name, "New Entry %d", g->num_entries);

    g->num_entries++;

    werase(ep->info_win);
    werase(ep->win);
    wnoutrefresh(ep->info_win);
    wnoutrefresh(ep->win);
}


void entry_field_sel_next(int* sel_field) {
    if (*sel_field < 3) *sel_field += 1;
}

void entry_field_sel_prev(int* sel_field) {
    if (*sel_field > 0) *sel_field -= 1;
}

void entry_sel_next(struct Group* g) {
    if (g->sel_entry < g->num_entries - 1) g->sel_entry++;
}

void entry_sel_prev(int* sel_entry) {
    if (*sel_entry > 0) *sel_entry -= 1;
}
