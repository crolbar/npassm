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

void save_db(struct App* app, char* path) {
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
