#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int where_dis;
    int out_dis;
    char *pattern;
} GrepInfo;

#define SUCCESS 0
#define ERROR -1
void grep_error(const char *format, ...);

GrepInfo *grep_init();
int get_outfile(GrepInfo*, char**);
int get_searchfile(GrepInfo*, char**);
int get_pattern(GrepInfo*, char**);
void clean_grep(GrepInfo*);
size_t find_line(char *sub);

#define MAXLEN 255
#define MAXLINES 32

int main(int argc, char *argv[]) {
    if (argc < 2) {
        grep_error("You need enter at least 1 arguemnt: [TO FIND]");
        return 1;
    }
    GrepInfo *grep = grep_init();
    if (!grep) {
        return 1;
    }
    argv++;
    while (*argv){
        if ((*argv)[0] == '-') {
            if (strcmp(*argv, "-to") == 0) {
                if (get_outfile(grep, ++argv) == ERROR) return 1;
            }
            else if (strcmp(*argv, "-") == 0) grep->where_dis = STDIN_FILENO;
            else {
                grep_error("unknown argument: %s", *argv);
                clean_grep(grep);
                return 1;
            } 
        } else if (!grep->pattern) {
            if (get_pattern(grep, argv) == ERROR) 
                    return 1;
        }
        else if (get_searchfile(grep, argv) == ERROR) return 1;
        argv++;
    }
    char buffer[BUFSIZ];   
    char answer_lines[MAXLEN * MAXLINES];
    size_t all_len = 0;
    ssize_t bytes_readed = 0;
    char *start = buffer;
    int len = 0;
    char temp = 0;
    while ((bytes_readed = read(grep->where_dis, buffer, BUFSIZ)) > 0) 
        while ((len = find_line(start)) > 0) {
            temp = start[len];
            start[len] = '\0';
            if (strstr(start, grep->pattern)){
                strncpy(answer_lines+all_len, start, len);
                all_len += len;
                answer_lines[all_len-1] = '\n';
            }
            start[len] = temp;
            start += len+1;
        }
    if (bytes_readed == -1) {
        grep_error("error with reading");
        clean_grep(grep);
        return 1;
    }
    else if (write(grep->out_dis, answer_lines, all_len) == -1) {
        grep_error("error with writing");
        clean_grep(grep);
        return 1;
    };
    clean_grep(grep);
    return 0;
}

size_t find_line(char *sub) {
    if (!sub) return -1;
    size_t len = 0;
    while (*(sub+len) != '\0' && *(sub+len) != '\n') 
        len++;
    if (*(sub+len) == '\0') return -1;
    return len == 0 ? -1 : len;
}

void clean_grep(GrepInfo* f) {
    if (!f) return;
    if (f->out_dis && f->out_dis != STDOUT_FILENO) close(f->out_dis);
    if (f->where_dis && f->where_dis != STDIN_FILENO) close(f->where_dis);
    free(f);
}

GrepInfo *grep_init() {
    GrepInfo *set = (void*)malloc(sizeof(GrepInfo));
    if (!set) {
        grep_error("error with memory allocate");
        return NULL;
    }
    set->pattern = NULL;
    set->out_dis = STDOUT_FILENO;
    set->where_dis = STDIN_FILENO;
    return set;
}

int get_pattern(GrepInfo* info, char** argv) {
    if (!info) {
        grep_error("error with [PATTERN]");
        clean_grep(info);
        return ERROR;
    } else 
        info->pattern = *argv;
    return SUCCESS;
}

int get_searchfile(GrepInfo* info, char** argv) {
    if (!info) {
        grep_error("error with [SEARCH IN] file");
        clean_grep(info);
        return ERROR;
    } else {
        info->where_dis = open(*argv, O_RDONLY, 0644);
        if (info->where_dis == -1) {
            grep_error("error with opening file: %s", *argv);
            clean_grep(info);
            return -1;
        }
    }
    return SUCCESS; 
}

int get_outfile(GrepInfo* info, char **argv) {
    if (!argv || !*argv) {
        grep_error("need [OUTPUT] near the [-to]");
        clean_grep(info);
        return ERROR;
    }
    else if (!info) {
        grep_error("error with [TO FIND] searching");
        clean_grep(info);
        return ERROR;
    } else {
        info->out_dis = open(*argv, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (info->out_dis == -1) {
            grep_error("error with opening file: %s", *argv);
            clean_grep(info);
            return ERROR;
        }
    }
    return SUCCESS;  
}

inline void grep_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "grep: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

