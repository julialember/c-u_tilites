#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void cat_error(const char *format, ...);
FILE *work_with_arguments(int argc, char *argv[], FILE *to_read[], int *files_readed);

#define MAXFILES 32
int main(int argc, char *argv[]) {
    FILE *to_read[MAXFILES];
    int files_readed = 0;
    to_read[files_readed] = stdin;
    FILE *output = work_with_arguments(argc, argv, to_read, &files_readed); 
    if (output == NULL) {
        for (int i = 0; i < files_readed; i++) {
            if (to_read[i] != stdin) fclose(to_read[i]);
        };
        return 1;
    }
    char buffer[BUFSIZ];
    size_t bytes_readed = 0;
    for (int i = 0; i < files_readed; i++) {
        while ((bytes_readed = fread(buffer, 1, sizeof buffer, to_read[i])) > 0) 
            fwrite(buffer, 1, bytes_readed, output); 
        fclose(to_read[i]);
        to_read[i] = NULL;
    }
    fclose(output);
    return 0;
}


FILE *work_with_arguments(int argc, char *argv[], FILE *to_read[], int *files_readed) {
    for (int i = 1; i < argc && *files_readed < MAXFILES; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '\0') to_read[(*files_readed)++] = stdin;
            else if (strcmp(argv[i], "-to") != 0) {
                cat_error("unknown argumnent %s", argv[i]);
                return NULL;
            }
            else if (argc > i + 1){
                FILE *output_file = fopen(argv[i+1], "w+");
                if (output_file == NULL) {
                    cat_error("can't open file: %s", argv[i+1]);
                }
                return output_file;
            }
            else return stdout;
        }
        else {
            FILE *next_from = fopen(argv[i], "r");
            if (next_from == NULL) cat_error("can't open file: %s", argv[i]);
            else to_read[(*files_readed)++] = next_from;
        } 
    };
    if (*files_readed >= MAXFILES) {
        cat_error("%d files maximum!\nfiles after %s -was not readed", MAXFILES, argv[*files_readed]);
    }
    return stdout;
}

inline void cat_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "cat: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}
