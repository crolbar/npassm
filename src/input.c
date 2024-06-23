#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

void up_str_pop(WINDOW* win, char** mod_str, bool word) {
    int l = strlen(*mod_str);

    if (l > 0) {

        if (word) {
            char* str = strdup(*mod_str);
            for (l -= 1; l > 0; l--) {
                if (str[l] == ' ' || str[l] == '\n') {
                    break;
                } 
            }
        } else {
            l--; 
        }

        char* str = (char*)malloc((l + 1) * sizeof(char));
        strncpy(str, *mod_str, l);

        str[l] = '\0';

        free(*mod_str);
        *mod_str = str;

        werase(win);
        wnoutrefresh(win);
    }
}

void up_str_push(WINDOW* win, char** mod_str, char c) {
    int l = strlen(*mod_str);

    char* str = (char*)malloc((l + 2) * sizeof(char));
    if (str == NULL) exit(EXIT_FAILURE);

    strcpy(str, *mod_str);
    str[l] = c;
    str[l + 1] = '\0';

    free(*mod_str);
    *mod_str = str;

    werase(win);
    wnoutrefresh(win);
}
