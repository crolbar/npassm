#ifndef APP
#define APP

#include "dialogbox.h"
#include "group/group.h"
#include "group/entry/entry.h"

#include <ncurses.h>

enum PaneTypes {
    Group,
    Entry,
    EntryFields,
    DialogBox,
};

struct Panes {
    enum PaneTypes active;
    enum PaneTypes prev_active;
};

struct App {
    int exit;
    char* dbname;

    struct Panes panes;
    struct DialogBox dialogbox;
    struct GroupPane group_pane;
    struct EntryPane entry_pane;
};

bool render_groups(struct App *app);
bool render_entry_pane(
        struct EntryPane* ep,
        enum PaneTypes active_pane,
        struct Group* group);

void init_windows(struct App* app);
void init_dialogbox_windows(struct DialogBox* db);

void save_db(const struct App* app, char* path);
struct App open_db(char* path);

#endif
