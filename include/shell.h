#ifndef _SHELL_H_
#define _SHELL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h> /* For setlocale() */

#ifdef WINDOWS
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#define chdir _chdir
#define getcwd _getcwd
#define read _read
#define write _write
#define isatty _isatty
#define access _access
#define open _open
#define close _close
#define fstat _fstat
#define F_OK 0
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
/* Only define ssize_t if it's not already defined */
#if !defined(ssize_t) && !defined(_SSIZE_T_DEFINED)
typedef long long ssize_t;
#define _SSIZE_T_DEFINED
#endif
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

/* for read/write buffers */
#define READ_BUF_SIZE 1024
#define WRITE_BUF_SIZE 1024
#define BUF_FLUSH -1

/* for command chaining */
#define CMD_NORM 0
#define CMD_OR 1
#define CMD_AND 2
#define CMD_CHAIN 3

/* for convert_number() */
#define CONVERT_LOWERCASE 1
#define CONVERT_UNSIGNED 2

/* 1 if using system getline() */
#define USE_GETLINE 0
#define USE_STRTOK 0

#define HIST_FILE ".simple_shell_history"
#define HIST_MAX 4096

/* Avoid conflict with system environ */
#ifdef WINDOWS
/* Use _environ from stdlib.h, don't redeclare it */
#define shell_environ _environ
#else
extern char **environ;
#define shell_environ environ
#endif

/* Message IDs for localization */
enum message_id
{
    MSG_WELCOME,
    MSG_CMD_NOT_FOUND,
    MSG_PERMISSION_DENIED,
    MSG_MEMORY_ERROR,
    MSG_FILE_NOT_FOUND,
    MSG_INVALID_ARG,
    MSG_TOO_MANY_ARGS,
    MSG_NOT_ENOUGH_ARGS,
    MSG_CANNOT_OPEN_FILE,
    MSG_CANNOT_WRITE_FILE,
    MSG_HELP_HINT,
    MSG_EXIT,
    MSG_HISTORY_CLEARED,
    MSG_ENV_NOT_FOUND,
    MSG_ENV_SET,
    MSG_ENV_UNSET,
    MSG_DIR_CHANGED,
    MSG_CANNOT_CHANGE_DIR,
    MSG_ALIAS_CREATED,
    MSG_ALIAS_NOT_FOUND,
    MSG_ALIAS_REMOVED,
    MSG_CMD_EXECUTED,
    MSG_CMD_FAILED,
    MSG_SYNTAX_ERROR,
    MSG_PROMPT,
    MSG_COUNT
};

/* Localization functions */
int set_language(int lang_code);
int get_language(void);
const char *get_message(int msg_id);
int detect_system_language(void);
int init_locale(void);

/**
 * struct liststr - singly linked list
 * @num: the number field
 * @str: a string
 * @next: points to the next node
 */
typedef struct liststr
{
    int num;
    char *str;
    struct liststr *next;
} list_t;

/**
 *struct passinfo - contains pseudo-arguements to pass into a function,
 *		allowing uniform prototype for function pointer struct
 *@arg: a string generated from getline containing arguements
 *@argv: an array of strings generated from arg
 *@path: a string path for the current command
 *@argc: the argument count
 *@line_count: the error count
 *@err_num: the error code for exit()s
 *@linecount_flag: if on count this line of input
 *@fname: the program filename
 *@env: linked list local copy of environ
 *@env_array: custom modified copy of environ from LL env
 *@history: the history node
 *@alias: the alias node
 *@env_changed: on if environ was changed
 *@status: the return status of the last exec'd command
 *@cmd_buf: address of pointer to cmd_buf, on if chaining
 *@cmd_buf_type: CMD_type ||, &&, ;
 *@readfd: the fd from which to read line input
 *@histcount: the history line number count
 */
typedef struct passinfo
{
    char *arg;
    char **argv;
    char *path;
    int argc;
    unsigned int line_count;
    int err_num;
    int linecount_flag;
    char *fname;
    list_t *env;
    list_t *history;
    list_t *alias;
    char **env_array; /* Renamed from environ to avoid conflict */
    int env_changed;
    int status;

    char **cmd_buf;   /* pointer to cmd ; chain buffer, for memory mangement */
    int cmd_buf_type; /* CMD_type ||, &&, ; */
    int readfd;
    int histcount;
} info_t;

#define INFO_INIT                                                            \
    {NULL, NULL, NULL, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, NULL, \
     0, 0, 0}

/**
 *struct builtin - contains a builtin string and related function
 *@type: the builtin command flag
 *@func: the function
 */
typedef struct builtin
{
    char *type;
    int (*func)(info_t *);
} builtin_table;

/* toem_shloop.c */
int hsh(info_t *, char **);
int find_builtin(info_t *);
void find_cmd(info_t *);
void fork_cmd(info_t *);

/* toem_parser.c */
int is_cmd(info_t *, char *);
char *dup_chars(char *, int, int);
char *find_path(info_t *, char *, char *);

/* loophsh.c */
int loophsh(char **);

/* toem_errors.c */
void _eputs(char *);
int _eputchar(char);
int _putfd(char c, int fd);
int _putsfd(char *str, int fd);

/* toem_string.c */
int _strlen(char *);
int _strcmp(char *, char *);
char *starts_with(const char *, const char *);
char *_strcat(char *, char *);

/* toem_string1.c */
char *_strcpy(char *, char *);
char *shell_strdup(const char *); /* Renamed from _strdup to avoid conflict */
void _puts(char *);
int _putchar(char);

/* toem_exits.c */
char *_strncpy(char *, char *, int);
char *_strncat(char *, char *, int);
char *_strchr(char *, char);

/* toem_tokenizer.c */
char **strtow(char *, char *);
char **strtow2(char *, char);

/* toem_realloc.c */
char *_memset(char *, char, unsigned int);
void ffree(char **);
void *_realloc(void *, unsigned int, unsigned int);

/* toem_memory.c */
int bfree(void **);

/* toem_atoi.c */
int interactive(info_t *);
int is_delim(char, char *);
int _isalpha(int);
int _atoi(char *);

/* toem_errors1.c */
int _erratoi(char *);
void print_error(info_t *, char *);
int print_d(int, int);
char *convert_number(long int, int, int);
void remove_comments(char *);

/* toem_builtin.c */
int _myexit(info_t *);
int _mycd(info_t *);
int _myhelp(info_t *);

/* toem_builtin1.c */
int _myhistory(info_t *);
int _myalias(info_t *);
int _mylang(info_t *); /* New language command */
int _mytest(info_t *); /* Test command for UTF-8 and Arabic */

/*toem_getline.c */
ssize_t get_input(info_t *);
int _getline(info_t *, char **, size_t *);
void sigintHandler(int);

/* toem_getinfo.c */
void clear_info(info_t *);
void set_info(info_t *, char **);
void free_info(info_t *, int);

/* toem_environ.c */
char *_getenv(info_t *, const char *);
int _myenv(info_t *);
int _mysetenv(info_t *);
int _myunsetenv(info_t *);
int populate_env_list(info_t *);

/* toem_getenv.c */
char **get_environ(info_t *);
int _unsetenv(info_t *, char *);
int _setenv(info_t *, char *, char *);

/* toem_history.c */
char *get_history_file(info_t *info);
int write_history(info_t *info);
int read_history(info_t *info);
int build_history_list(info_t *info, char *buf, int linecount);
int renumber_history(info_t *info);

/* toem_lists.c */
list_t *add_node(list_t **, const char *, int);
list_t *add_node_end(list_t **, const char *, int);
size_t print_list_str(const list_t *);
int delete_node_at_index(list_t **, unsigned int);
void free_list(list_t **);

/* toem_lists1.c */
size_t list_len(const list_t *);
char **list_to_strings(list_t *);
size_t print_list(const list_t *);
list_t *node_starts_with(list_t *, char *, char);
ssize_t get_node_index(list_t *, list_t *);

/* toem_vars.c */
int is_chain(info_t *, char *, size_t *);
void check_chain(info_t *, char *, size_t *, size_t, size_t);
int replace_alias(info_t *);
int replace_vars(info_t *);
int replace_string(char **, char *);

/* UTF-8 and Arabic support functions */
int get_utf8_char_length(char first_byte);
int read_utf8_char(char *buffer, int max_size);
int is_rtl_char(int unicode_codepoint);
int utf8_to_codepoint(char *utf8_char, int length);
int codepoint_to_utf8(int codepoint, char *utf8_char);
void configure_terminal_for_utf8(void);
int set_text_direction(int is_rtl);

/* UTF-8 output functions */
void _puts_utf8(char *str);
void _eputs_utf8(char *str);
int _putsfd_utf8(char *str, int fd);
void print_prompt_utf8(info_t *info);

#endif
