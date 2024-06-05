#ifndef APP
#define APP

#include "group/group.h"
#include "group/entry/entry.h"

#include <ncurses.h>

enum Panes {
    Group,
    Entry,
    EntryFields,
};

struct App {
    int exit;
    enum Panes active_pane;

    struct GroupPane group_pane;
    struct EntryPane entry_pane;
};

bool render_groups(struct App *app);
bool render_entry_pane(
        struct EntryPane* ep,
        enum Panes active_pane,
        struct Group* group);

void init_windows(struct App* app);

#endif
