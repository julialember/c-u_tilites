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
void clean_grep(GrepInfo*);
size_t find_line(char *sub);
int find_lines(GrepInfo *);


#define MAXLEN 255
#define MAXLINES 32

int main(int argc, char *argv[]) {
    if (argc < 3) {
        grep_error("You need enter at least 1 arguemnt: [TO FIND]");
        return 1;
    }
    GrepInfo *grep = grep_init();
    if (!grep) {
        return 1;
    }
    argv++;
    while(*argv){
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
        } else if (!grep->pattern) grep->pattern = *argv;
        else if (get_searchfile(grep, argv) == ERROR) return 1;
        argv++;
    }
    if (!grep->pattern) {
        grep_error("please set [PATTERN]");
        clean_grep(grep);
        return 1;
    }
    return find_lines(grep) == ERROR ? 1 : 0;
}

int find_lines(GrepInfo *grep) {
    size_t all_len = 0;
    char buffer[BUFSIZ];   
    char answer_lines[BUFSIZ];
    ssize_t bytes_readed = 0;
    char *start = buffer;
    size_t len = 0;
    while (all_len < BUFSIZ && 
            (bytes_readed = read(grep->where_dis, buffer, BUFSIZ)) > 0) 
        while ((len = find_line(start)) > 0 && all_len + len < BUFSIZ) {
            start[len] = '\0';
            if (strstr(start, grep->pattern)){
                strncpy(answer_lines+all_len, start, len);
                all_len += len;
                answer_lines[all_len++] = '\n';
            }
            start += len+1;
        }
    if (bytes_readed == ERROR) {
        grep_error("error with reading");
        clean_grep(grep);
        return ERROR;
    }
    else if (write(grep->out_dis, answer_lines, all_len) == ERROR) {
        grep_error("error with write");
        clean_grep(grep);
        return ERROR;
    }
    clean_grep(grep);
    return SUCCESS;
}

size_t find_line(char *sub) {
    size_t len = 0;
    while (*(sub+len) != '\0' && *(sub+len) != '\n') 
        len++;
    return len;
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

int get_searchfile(GrepInfo* info, char** argv) {
    info->where_dis = open(*argv, O_RDONLY, 0644);
    if (info->where_dis == -1) {
        grep_error("error with opening file: %s", *argv);
        clean_grep(info);
        return ERROR;
    }
    return SUCCESS; 
}

int get_outfile(GrepInfo* info, char **argv) {
    if (!argv) {
        grep_error("need [OUTPUT] near the [-to]");
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
