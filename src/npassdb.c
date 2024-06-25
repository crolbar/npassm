#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crypto.c"

const bool ENC = true;

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

        free(ser_string);
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
        free(ser_name);
        free(ser_username);
        free(ser_email);
        free(ser_password);
        free(ser_notes);
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
        free(ser_name);
        free(ser_g);
    }

    return ser_groups;
}

void save_db(const struct App* app) {
    FILE* f = fopen(app->dbpath, "w");

    char* groups = ser_groups(app->group_pane.groups, app->group_pane.num_groups);
    char* ser_dbname = ser_str(app->dbname);


    char* fmt = "{\"%s\"}{%d %d [%s]}";

    int ser_db_size = strlen(fmt) + strlen(ser_dbname) + strlen(groups);

    char* ser_db = malloc(ser_db_size);

    sprintf(ser_db, fmt,
        ser_dbname,
        app->panes.active,
        app->group_pane.sel,
        groups
    );

    if (ENC) {
        fwrite("epassdb", 7, 1, f);
        char* cipher = encrypt_db(f, app->password, (unsigned char*)ser_db, &ser_db_size);

        fwrite(&ser_db_size, sizeof(ser_db_size), 1, f);
        fwrite(cipher, 1, ser_db_size, f);

        free(cipher);
    } else {
        fwrite("npassdb", 7, 1, f);
        fwrite(ser_db, strlen(ser_db), 1, f);
    }


    free(ser_db);
    free(groups);
    free(ser_dbname);

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

    int skipped_backslash = 0;
    for (int i = 0; i < len; i++) {
        if (d->f_conts[start + i + skipped_backslash] == '\\' && d->f_conts[start + i + skipped_backslash + 1] == '"') {
            skipped_backslash++;
        }
      
        str[i] = d->f_conts[start + i + skipped_backslash];
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
    d->app->panes.active = de_int(d);
    d->app->group_pane.sel = de_int(d);

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
        .dbname = NULL,
        .top_bar_info = malloc(1),
        .panes = {
            .active = 0,
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
        },
        .passgen = {
            .sel = 0,
            .numbers = true,
            .symbols = true,
            .slider = 1,
        }
    };

    init_windows(&app);

    return app;
}

bool open_db(struct App* app, char* password) {
    struct Deserializer d = { .app = app };


    FILE* f = fopen(app->dbpath, "r");


    fseek(f, 0, SEEK_END);
    d.f_size = ftell(f);
    rewind(f);


    d.f_conts = malloc((d.f_size + 1) * sizeof(char));

    char* form[7];
    if (fread(form, 7, 1, f)) {};


    if (!strcmp((char*)form, "epassdb")) {
        char* ser_db = decrypt_db(f, (int*)&d.f_size, password);

        free(d.f_conts);

        if (ser_db == NULL) {
            free(ser_db);
            return true;
        }

        d.f_conts = malloc((d.f_size + 1) * sizeof(char));

        strcpy(d.f_conts, ser_db);

        free(ser_db);
    } else if (!strcmp((char*)form, "npassdb")) {
        if (fread(d.f_conts, sizeof(char), d.f_size - 7, f) != d.f_size - 7) {
            printf("Error occured while reading db file.\n");
            exit(1);
        };

    } else {
        printf("Wrong file format.\n");
        exit(0);
    }
    
    d.f_conts[d.f_size] = '\0';

    for (d.i = 0; d.i < d.f_size; d.i++) {
        if (d.f_conts[d.i] == '{' &&  d.i == 0) {
            de_metadata(&d);
        }

        if (d.f_conts[d.i] == '{' && d.i > 0) {
            de_data(&d);
            break;
        }
    }

    fclose(f);
    free(d.f_conts);
    return false;
}

bool is_npassdb(char* path) {
    FILE* f = fopen(path, "r");

    char* form[7];
    if (fread(form, 7, 1, f)) {};

    fclose(f);

    if (strcmp((char*)form, "epassdb") && strcmp((char*)form, "npassdb")) {
        endwin();
        printf("Wrong file format.\n");
        exit(0);
    }

    return true;
}


// `d->f_conts[d->i]` has to be the opening quote.
// when it returns the string, `d->f_conts[d->i]` will be the closing quote
char* csv_de_str(struct Deserializer* d, bool skip) {
    int start = ++d->i;

    int num_q = 0;
    while (d->f_size) {
        if (d->f_conts[d->i] == '"') {
            if (d->f_conts[d->i + 1] != '"') {
                break;
            } else {
                num_q++;
                d->i++;
            }
        }
        d->i++;
    }

    if (skip) {
        return NULL;
    }

    int len = (d->i - start) - num_q;
    char* str = malloc(len + 1);

    int skipped_quotes = 0;
    for (int i = 0; i < len; i++) {
        if (d->f_conts[i + start + skipped_quotes] == '"' && d->f_conts[i + start + skipped_quotes - 1] == '"') {
            skipped_quotes++;
        }
        str[i] = d->f_conts[i + start + skipped_quotes];
    }
    str[len] = '\0';

    return str;
}

// "Group","Title","Username","Password","URL","Notes","TOTP","Icon","Last Modified","Created"
void csv_de_entry(struct Deserializer* d) {
    char* group_name = csv_de_str(d, 0);
    // skit the closing quote and comma
    d->i += 2;

    char* entry_name = csv_de_str(d, 0);
    d->i += 2;

    char* entry_username = csv_de_str(d, 0);
    d->i += 2;

    char* entry_password = csv_de_str(d, 0);
    d->i += 2;
    
    { // skip URL
        void* v = csv_de_str(d, 1);
        d->i += 2;
    }

    char* entry_notes = csv_de_str(d, 0);
    d->i += 2;

    // skip till new line (next entry)
    for (int i = 1; i <= 4; i++) {
        void* v = csv_de_str(d, 1);
        if (i != 4) {
            d->i += 2;
        }
    }


    struct GroupPane* gp = &d->app->group_pane;

    // append group
    if (!gp->num_groups || strcmp(gp->groups[gp->num_groups - 1].name, group_name)) {
        gp->num_groups++;

        gp->groups = realloc(gp->groups, gp->num_groups * sizeof(struct Group));

        struct Group* g = &gp->groups[gp->num_groups - 1];

        *g = (struct Group){
            .sel_entry = 0,
            .num_entries = 0,
            .entries = malloc(sizeof(struct Entry)),
            .name = group_name,
        };
    }

    { // append entry
        struct Group* g = &gp->groups[gp->num_groups - 1];
        g->num_entries++;

        g->entries = realloc(g->entries, g->num_entries * sizeof(struct Entry));

        g->entries[g->num_entries - 1] = (struct Entry){
            .sel_field = 0,
            .name = entry_name,
            .username = entry_username,
            .email = malloc(1),
            .password = entry_password,
            .notes = entry_notes,
        };

        g->entries[g->num_entries - 1].email[0] = '\0';
    }
}

void import_keepass_csv(struct App* app, char* import_path) {
    struct Deserializer d = { .app = app };

    FILE* f = fopen(import_path, "r");

    fseek(f, 0, SEEK_END);
    d.f_size = ftell(f);
    rewind(f);


    d.f_conts = malloc((d.f_size + 1) * sizeof(char));

    if (fread(d.f_conts, sizeof(char), d.f_size, f) != d.f_size) {
        endwin();
        printf("Error occured while reading import file.\n");
        exit(1);
    };

    const char csv_keepass_first_line[] = "\"Group\",\"Title\",\"Username\",\"Password\",\"URL\",\"Notes\",\"TOTP\",\"Icon\",\"Last Modified\",\"Created\"";
    char* first_line_buf = malloc(strlen(csv_keepass_first_line) + 1);

    for (d.i = 0; d.i < d.f_size - 1; d.i++) {
        if (!strcmp((char*)csv_keepass_first_line, first_line_buf)) {
            if (d.f_conts[d.i] == '\n' && d.f_conts[d.i + 1] == '"') {
                d.i++;
                csv_de_entry(&d);
            }
       } else {
            if (d.f_conts[d.i] == '\n') {
                if (strcmp(csv_keepass_first_line, first_line_buf)) {
                    endwin();
                    printf("The csv file that you provided doesn't seem generated from keepassxc.\n");
                    exit(1);
                }
            }
            first_line_buf[d.i] = d.f_conts[d.i];
        }
    }

    d.app->group_pane.sel = 0;

    free(first_line_buf);
    fclose(f);
}
