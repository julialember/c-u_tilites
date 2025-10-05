#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>

void cat_error(const char *format, ...);
int work_with_arguments(int argc, char *argv[], int to_read[], int *files_readed);
void close_all(int files_to_close[], int files);

#define CLOSE_OUTPUT(a) if ((a) != (STDOUT_FILENO)) close(a)
#define CLOSE_INTPUT(a) if ((a) != (STDIN_FILENO)) close(a)
#define MAXFILES 32
int main(int argc, char *argv[]) {
    int to_read[MAXFILES];
    int files_readed = 0;
    int output = work_with_arguments(argc, argv, to_read, &files_readed); 
    if (output == -1) {
        close_all(to_read, files_readed);
        return 1;
    }
    if (files_readed == 0) to_read[files_readed++] = STDIN_FILENO;
    char buffer[BUFSIZ];
    ssize_t bytes_readed = 0;;
    for (int i = 0; i < files_readed; i++) {
        while ((bytes_readed = read(to_read[i], buffer, sizeof buffer)) > 0) 
            if (write(output, buffer, bytes_readed) == -1) {
                cat_error("error with writing");
                close_all(to_read, files_readed);
                CLOSE_OUTPUT(output);
                return 1;
            }; 
        if (bytes_readed == -1) {
            cat_error("error with reading");
            close_all(to_read, files_readed);
            CLOSE_OUTPUT(output);
            return 1;
        } 
        CLOSE_INTPUT(to_read[i]);
    }
    CLOSE_OUTPUT(output);
    return 0; 
}

void close_all(int files_to_close[], int files) {
    for (int i = 0; i < files; i++) 
        CLOSE_INTPUT(files_to_close[i]);
}

int work_with_arguments(int argc, char *argv[], int *to_read, int *files_readed) {
    int output_file = STDOUT_FILENO;
    for (int i = 1; i < argc && *files_readed < MAXFILES; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '\0') to_read[(*files_readed)++] = STDIN_FILENO;
            else if (strcmp(argv[i], "-to") != 0) {
                cat_error("unknown argumnent %s", argv[i]);
                return -1;
            }
            else if (argc > i + 1){
                output_file = open(argv[++i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_file == -1) {
                    cat_error("can't open file: %s", argv[i]);
                    return -1;
                }
            }
            else {
                cat_error("need to [OUTPUT] file near the [-to]");
                return -1;
            }
        }
        else {
            int next_from = open(argv[i], O_RDONLY, 0644);
            if (next_from == -1) cat_error("can't open file: %s", argv[i]);
            else to_read[(*files_readed)++] = next_from;
        } 
    };
    if (*files_readed >= MAXFILES) {
        cat_error("%d files maximum!\nfiles after %s -was not readed", MAXFILES, argv[*files_readed]);
    }
    return output_file;
}

inline void cat_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "cat: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}
