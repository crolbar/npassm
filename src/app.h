#ifndef APP
#define APP

#include "dialogbox.h"
#include "group/group.h"
#include "entry/entry.h"
#include "passgen.h"

#include <ncurses.h>

enum PaneTypes {
    PaneGroup,
    PaneEntry,
    PaneEntryFields,
    PaneDialogBox,
    PanePassgen,
};

struct Panes {
    enum PaneTypes active;
    enum PaneTypes prev_active;
};

struct App {
    int exit;
    char* dbname;
    char* password;
    char* dbpath;

    struct Panes panes;
    struct DialogBox dialogbox;
    struct GroupPane group_pane;
    struct EntryPane entry_pane;
    struct Passgen passgen;
};

bool render_groups(struct App *app);
bool render_entry_pane(
        struct EntryPane* ep,
        enum PaneTypes active_pane,
        struct Group* group);

bool render_passgen(const struct App* app);

void init_windows(struct App* app);
void init_dialogbox_windows(struct DialogBox* db);

void save_db(const struct App* app);
bool open_db(struct App* app, char* password);
bool is_npassdb(char* path);
struct App init_app(char* path);
void import_keepass_csv(struct App* app, char* import_path);

void edit_db(struct App* app);

void up_str_pop(WINDOW* win, char** mod_str, bool word);
void up_str_push(WINDOW* win, char** mod_str, char c);

#endif
