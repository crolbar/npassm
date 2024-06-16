#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* ser_entries(struct Entry* entries, int num_entries) {
    int size = 1;
    char* ser_entries = malloc(size);
    strcpy(ser_entries, "");

    char fmt[] = "(%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\")";

    for (int i = 0; i < num_entries; i++) {
        struct Entry e = entries[i];
        
        int e_size = strlen(e.name) +
            strlen(e.username) +
            strlen(e.password) +
            strlen(e.email) +
            strlen(e.notes) +
            strlen(fmt) + 1;

        char* ser_e = malloc(e_size);

        sprintf(ser_e, fmt,
            e.sel_field,
            e.name,
            e.username,
            e.email,
            e.password,
            e.notes
        );

        size += e_size;
        ser_entries = realloc(ser_entries, size);
        strcat(ser_entries, ser_e);
        free(ser_e);
    }

    return ser_entries;
}

char* ser_group(struct Group g) {
    char fmt[] = "(\"%s\" %d [%s])";

    char* entries = ser_entries(g.entries, g.num_entries);

    int size = strlen(g.name) + strlen(entries) + strlen(fmt);
    char* ser_g = malloc(size);

    sprintf(ser_g, fmt,
        g.name,
        g.sel_entry,
        entries
    );

    free(entries);

    return ser_g;
}

char* ser_groups(struct Group* groups, int num_groups) {
    int size = 1;
    char* ser_groups = malloc(size);
    strcpy(ser_groups, "");

    for (int i = 0; i < num_groups; i++) {
        char* g = ser_group(groups[i]);

        size += strlen(g);
        ser_groups = realloc(ser_groups, size);

        strcat(ser_groups, g);
        free(g);
    }

    return ser_groups;
}

void save_db(const struct App* app, char* path) {
    FILE* f = fopen(path, "w");

    char* groups = ser_groups(app->group_pane.groups, app->group_pane.num_groups);

    fprintf(f,
        "{\"%s\"}{%d [%s]}",
        app->dbname,
        app->group_pane.sel,
        groups
    );

    free(groups);

    fclose(f);
}


struct Deserializer {
    char* f_conts;
    long f_size;
    int i;
    struct App* app;
};


// `d->f_conts[d->i]` has to be the opening quote.
// when it returns the string, `d->f_conts[d->i]` will be the closing quote
char* de_str(struct Deserializer* d) {
    int start = ++d->i;

    while (d->i < d->f_size && d->f_conts[d->i] != '"') {
        d->i++;
    }

    int len = d->i - start;
    char* str = malloc(len + 1);

    strncpy(str, &d->f_conts[start], len);
    str[len] = '\0';

    return str;
}

// `d->f_conts[d->i]` has to be the char befor the num.
// when it returns the int, `d->f_conts[d->i]` will be the char after the num
int de_int(struct Deserializer* d) {
    int len = 0;
    int num = 0;

    for (++d->i; d->i < d->f_size; d->i++) {
        if (d->f_conts[d->i] == ' ') {
            break;
        }
        
        num = (len) ?
            num * 10 + (d->f_conts[d->i] - '0') :
            num + (d->f_conts[d->i] - '0');

        len += 1;
    }

    return num;
}

void de_entries(struct Deserializer* d) {
    struct Group* g = &d->app->group_pane.groups[d->app->group_pane.num_groups - 1];
    g->num_entries = 0;
    g->entries = malloc(sizeof(struct Entry));

    for (++d->i; d->i < d->f_size; d->i++) {
        if (d->f_conts[d->i] == ']') {
            break;
        }

        if (d->f_conts[d->i] == '(') {
            g->num_entries += 1;
            g->entries = realloc(g->entries, g->num_entries * sizeof(struct Entry));
            struct Entry* e = &g->entries[g->num_entries - 1];


            e->sel_field = de_int(d);
            // skip the whitespace
            d->i++;

            for (int i = 0; i < 5; i++) {
                switch (i) {
                    case 0:
                        e->name = de_str(d);
                        break;
                    case 1:
                        e->username = de_str(d);
                        break;
                    case 2:
                        e->email = de_str(d);
                        break;
                    case 3:
                        e->password = de_str(d);
                        break;
                    case 4:
                        e->notes = de_str(d);
                        break;
                }

                // skip closing quote && whitespace
                if (i != 4) {
                    d->i += 2;
                }
            }
        }
    }
}

void de_group(struct Deserializer* d) {
    struct Group* g = &d->app->group_pane.groups[d->app->group_pane.num_groups - 1];

    for (++d->i; d->i < d->f_size; d->i++) {
        if (d->f_conts[d->i] == ')') {
            break;
        }

        if (d->f_conts[d->i] == '[') {
            de_entries(d);
        }

        if (d->f_conts[d->i] == '"') { 
            g->name = de_str(d);

            // d->i is the closing quote +1 to get char before num
            d->i += 1;

            g->sel_entry = de_int(d);
        }
    }
}

void de_data(struct Deserializer* d) {
    if (d->app->group_pane.sel == -1) {
        d->app->group_pane.sel = de_int(d);
    }

    for (++d->i; d->i < d->f_size; d->i++) {
        if (d->f_conts[d->i] == '}') {
           break;
        }

        if (d->f_conts[d->i] == '(') {
            d->app->group_pane.num_groups += 1;

            d->app->group_pane.groups = 
                realloc(
                    d->app->group_pane.groups,
                    d->app->group_pane.num_groups * sizeof(struct Group)
                );

            de_group(d);
        }
    }
}

void de_metadata(struct Deserializer* d) {
    for (++d->i; d->i < d->f_size; d->i++) {
        if (d->f_conts[d->i] == '}') {
            break;
        }

        switch (d->f_conts[d->i]) {
            case '"': 
                if (!d->app->dbname) {
                    d->app->dbname = de_str(d);
                    break;
                }
                break;
        }
    }
}

struct App init_app() {
    struct App app = {
        .exit = false,
        .panes = {
            .active = Group,
        },
        .group_pane = {
            .sel = -1,
            .groups = malloc(sizeof(struct Group)),
            .num_groups = 0,
        },
        .entry_pane = {
            .pass_hiden = true,
        },
        .dialogbox = {
            .title = NULL,
            .risized = false,
        }
    };

    init_windows(&app);

    return app;
}

struct App open_db(char* path) {
    struct App app = init_app();
    struct Deserializer d = { .app = &app };


    FILE* f = fopen(path, "r");


    fseek(f, 0, SEEK_END);
    d.f_size = ftell(f);
    rewind(f);


    d.f_conts = malloc((d.f_size + 1) * sizeof(char));

    if (fread(d.f_conts, sizeof(char), d.f_size, f) != d.f_size) {
        printf("Error occured while reading db file.");
        exit(1);
    };
    d.f_conts[d.f_size] = '\0';

    
    for (d.i = 0; d.i < d.f_size; d.i++) {
        if (d.f_conts[d.i] == '{' && d.i == 0) {
            de_metadata(&d);
        }

        if (d.f_conts[d.i] == '{' && d.i > 0) {
            de_data(&d);
            break;
        }
    }
    

    fclose(f);
    free(d.f_conts);
    return app;
}
