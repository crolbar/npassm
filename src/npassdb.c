#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* ser_str(char* str) {
    size_t size = strlen(str);

    char* ser_string = malloc(size + 3);
    strcpy(ser_string, str);

    if (strstr(str, "\"")) {
        for (int i = 0; i < size; i++) {
            if (str[i] == '"') {
                size++;
            }
        }

        ser_string = malloc(size + 3);

        int num_q = 0;
        for (int i = 0; i < strlen(str); i++) {
            if (str[i] == '"') {
                ser_string[i + num_q] = '\\';
                num_q++;
            }
            
            ser_string[i + num_q] = str[i];
        }
    }

    ser_string[size] = '\\';
    ser_string[size + 1] = 'n';
    ser_string[size + 2] = '\0';
    return ser_string;
}

char* ser_entries(struct Entry* entries, int num_entries) {
    int size = 1;
    char* ser_entries = malloc(size);
    strcpy(ser_entries, "");

    char fmt[] = "(%d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\")";

    for (int i = 0; i < num_entries; i++) {
        struct Entry e = entries[i];
        
        char* ser_name = ser_str(e.name);
        char* ser_username = ser_str(e.username);
        char* ser_email = ser_str(e.email);
        char* ser_password = ser_str(e.password);
        char* ser_notes = ser_str(e.notes);

        int e_size = strlen(ser_name) +
            strlen(ser_username) +
            strlen(ser_email) +
            strlen(ser_password) +
            strlen(ser_notes) +
            strlen(fmt) + 1;

        char* ser_e = malloc(e_size);

        sprintf(ser_e, fmt,
            e.sel_field,
            ser_name,
            ser_username,
            ser_email,
            ser_password,
            ser_notes
        );

        size += e_size;
        ser_entries = realloc(ser_entries, size);
        strcat(ser_entries, ser_e);
        free(ser_e);
    }

    return ser_entries;
}

char* ser_groups(struct Group* groups, int num_groups) {
    int size = 1;
    char* ser_groups = malloc(size);
    strcpy(ser_groups, "");

    char group_fmt[] = "(\"%s\" %d [%s])";

    for (int i = 0; i < num_groups; i++) {
        struct Group g = groups[i];

        char* entries = ser_entries(g.entries, g.num_entries);
        char* ser_name = ser_str(g.name);

        char* ser_g = malloc(
                strlen(ser_name) + strlen(entries) + strlen(group_fmt));

        sprintf(ser_g, group_fmt,
            ser_name,
            g.sel_entry,
            entries
       );

        size += strlen(ser_g);

        ser_groups = realloc(ser_groups, size);

        strcat(ser_groups, ser_g);

        free(entries);
        free(ser_g);
    }

    return ser_groups;
}

void save_db(const struct App* app) {
    FILE* f = fopen(app->dbpath, "w");

    char* groups = ser_groups(app->group_pane.groups, app->group_pane.num_groups);
    char* ser_dbname = ser_str(app->dbname);

    fprintf(f,
        "npassdb {\"%s\"}{%d [%s]}",
        ser_dbname,
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

    int num_q = 0;

    while (d->i < d->f_size) {
        if (d->f_conts[d->i] == '"') {
            if (d->f_conts[d->i - 1] == 'n' && d->f_conts[d->i - 2] == '\\') {
                break;
            }

            num_q++;
        }

        d->i++;
    }

    // -2 because of the \n at the end
    int len = (d->i - start) - num_q - 2;

    char* str = malloc(len + 2);

    int skipped_q = 0;
    for (int i = 0; i < len; i++) {
        if (d->f_conts[start + i + skipped_q] == '\\' && d->f_conts[start + i + skipped_q + 1] == '"') {
            skipped_q++;
        }
      
        str[i] = d->f_conts[start + i + skipped_q];
    }

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

struct App init_app(char* path) {
    struct App app = {
        .exit = false,
        .dbpath = path,
        .dbname = "test",
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
    struct App app = init_app(path);
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
        if (d.f_conts[d.i] == '{' && d.i == 8) {
            de_metadata(&d);
        }

        if (d.f_conts[d.i] == '{' && d.i > 8) {
            de_data(&d);
            break;
        }
    }
    

    fclose(f);
    free(d.f_conts);
    return app;
}

bool is_npassdb(char* path) {
    FILE* f = fopen(path, "r");

    char* f_conts = malloc(8 * sizeof(char));

    if (fread(f_conts, sizeof(char), 7, f) != 7) {
        printf("Error occured while reading file.");
        exit(1);
    };

    fclose(f);

    return strstr(f_conts, "npassdb");
}
