#ifndef BLD_H_
#define BLD_H_

/* TODO:
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>


#if !defined(_WIN32)
    #include <sys/wait.h>
    #include <sys/stat.h>
    #include <unistd.h>

    typedef struct timespec file_time_t;
#else // !defined(_WIN32)
#endif // !defined(_WIN32)


typedef struct
{
    char *data;
    size_t size, capacity;
} bld_sb_t;


int
bld_wait_for_multiple(pid_t *pids, int count);

int
bld_wait_for(pid_t pid);

int
bld_exists(const char *path);

int
bld_file_time_cmp(file_time_t a, file_time_t b);

file_time_t
bld_mod_time(const char *path);

pid_t
bld_execute_vargs_async(const char *fmt, va_list args);

pid_t
bld_execute_argv_async(char *argv[]);

int
bld_execute_argv(char *argv[]);

pid_t
bld_execute_str_async(const char *str);

int
bld_execute_str(const char *str);

pid_t
bld_execute_async(const char *fmt, ...);

int
bld_execute(const char *fmt, ...);

pid_t
bld_run_vargs_async(const char *name, const char *fmt, va_list args);

pid_t
bld_run_argv_async(const char *name, char *argv[]);

int
bld_run_argv(const char *name, char *argv[]);

pid_t
bld_run_str_async(const char *name, const char *str);

int
bld_run_str(const char *name, const char *str);

pid_t
bld_run_async(const char *name, const char *fmt, ...);

int
bld_run(const char *name, const char *fmt, ...);

pid_t
bld_run_program_async(const char *name);

int
bld_run_program(const char *name);

char *
bld_strf(const char *fmt, ...);

int
bld_str_eq(const char *str_a, const char *str_b);

int
bld_contains(const char *str, int argc, char *argv[]);

int
bld_find(const char *str, int count, const char **list);

char *
bld_join(const char *delim, int count, const char **list);

pid_t
bld_cc_params_async(const char **params, int param_count);

int
bld_cc_params(const char **params, int param_count);

pid_t
bld_cc_params_nt_async(const char **params);

int
bld_cc_params_nt(const char **params);

void
bld_sb_append(bld_sb_t *sb, const char *s);

void
bld_sb_append_multiple(bld_sb_t *sb, const char **sl, int count);

void
bld_sb_append_multiple_nt(bld_sb_t *sb, const char **sl);

int
bld_try_rebuild_self(char *self, int argc, char *argv[]);


#define BLD_LEN(a) (sizeof(a) / sizeof(*(a)))

#define BLD_SB_APPEND(sb, ...) \
    bld_sb_append_multiple_nt(sb, ((const char*[]) { __VA_ARGS__, NULL }))

#define BLD_CC(...) \
    bld_cc_params_nt(((const char*[]){ __VA_ARGS__, NULL }))

#define BLD_CC_ASYNC(...) \
    bld_cc_params_nt_async(((const char*[]){ __VA_ARGS__, NULL }))

#define BLD_GCC_WARNINGS     "-Wall -Wextra -pedantic"
#define BLD_GCC_WARNINGS_OFF "-Wno-deprecated-declarations -Wno-missing-field-initializers"


#define BLD_STRETCHY_T(data_type, size_type) struct { data_type *data; size_type count, capacity; }

#define BLD_STRETCHY_FOR(_dck, _type, _elem) \
    for (_type *_elem = (_dck).data; _elem < (_dck).data + (_dck).count; ++_elem)

/* argument must be an 'lvalue', except for `...` */
#define BLD_STRETCHY_PUSH(_dck, ...)                                                    \
do {                                                                                    \
    if ((_dck).count == (_dck).capacity) {                                              \
        (_dck).capacity = (_dck).capacity ? (_dck).capacity * 2                         \
                                          : 4096 / sizeof(*((_dck).data));              \
        (_dck).data = realloc((_dck).data, sizeof(*((_dck).data)) * (_dck).capacity);   \
        assert((_dck).data);                                                            \
    }                                                                                   \
    (_dck).data[(_dck).count] = __VA_ARGS__;                                            \
    (_dck).count++;                                                                     \
} while (0)

/* argument must be an 'lvalue', except for `amount` */
#define BLD_STRETCHY_RESERVE(_dck, _amount)                                             \
do {                                                                                    \
    if ((_dck).count + (_amount) > (_dck).capacity) {                                   \
        if ((_dck).capacity == 0) {                                                     \
            (_dck).capacity = 4096 / sizeof(*((_dck).data));                            \
        }                                                                               \
        while ((_dck).count + (_amount) > (_dck).capacity) {                            \
            (_dck).capacity *= 2;                                                       \
        }                                                                               \
        (_dck).data = realloc((_dck).data, sizeof(*((_dck).data)) * (_dck).capacity);   \
        assert((_dck).data);                                                            \
    }                                                                                   \
} while (0)

#define BLD_REBUILD_SELF(_argc, _argv) \
do { \
    int _res = bld_try_rebuild_self(__FILE__, _argc, _argv); \
    if (_res != -1) \
        exit(_res); \
} while (0)


#if defined(BLD_IMPLEMENTATION)

#if !defined(_WIN32)


int
bld_wait_for(pid_t pid)
{
    siginfo_t info;

    if (waitid(P_ALL, pid, &info, WEXITED) == -1) {
        fprintf(stderr, "waitid(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return info.si_status;
}

int
bld_wait_for_multiple(pid_t *pids, int count)
{
    for (int i = 0; i < count; ++i) {
        int ret = bld_wait_for(pids[i]);

        if (ret)
            return ret;
    }

    return 0;
}

pid_t
bld_execute_str_async(const char *str)
{
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "fork(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", str, NULL);

        fprintf(stderr, "execve(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return pid;
}

pid_t
bld_execute_vargs_async(const char *fmt, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    int to_write = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char *buffer = malloc(to_write + 1);
    assert(buffer);

    int written = vsnprintf(buffer, to_write + 1, fmt, args_copy);
    va_end(args_copy);

    assert(written == to_write);

    pid_t pid = bld_execute_str_async(buffer);

    free(buffer);

    return pid;
}

int
bld_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

pid_t
bld_execute_argv_async(char *argv[])
{
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "fork(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execv(argv[0], argv);

        fprintf(stderr, "execv(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return pid;
}

int
bld_file_time_cmp(file_time_t a, file_time_t b)
{
    int res = (a.tv_sec > b.tv_sec) - (a.tv_sec < b.tv_sec);

    if (res == 0)
        return (a.tv_nsec > b.tv_nsec) - (a.tv_nsec < b.tv_nsec);

    return res;
}

file_time_t
bld_mod_time(const char *path)
{
    struct stat st = {0};

    if (lstat(path, &st)) {
        fprintf(stderr, "lstat(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return (file_time_t) { st.st_mtime };
}

pid_t
bld_cc_params_async(const char **params, int param_count)
{
    bld_sb_t sb = {0};

    bld_sb_append(&sb, "cc");

    for (int i = 0; i < param_count; ++i) {
        bld_sb_append(&sb, " ");
        bld_sb_append(&sb, params[i]);
    }

    pid_t pid = bld_execute_str_async(sb.data);

    free(sb.data);

    return pid;
}

pid_t
bld_run_argv_async(const char *name, char *argv[])
{
    char *prog = bld_strf("./%s");

    BLD_STRETCHY_T (const char *, int) new_argv = {0};

    BLD_STRETCHY_PUSH(new_argv, prog);

    for (int i = 0; argv[i]; ++i) {
        BLD_STRETCHY_PUSH(new_argv, argv[i]);
    }

    BLD_STRETCHY_PUSH(new_argv, NULL);


    pid_t pid = bld_execute_argv_async((char**)new_argv.data);

    free(prog);
    free(new_argv.data);

    return pid;
}

pid_t
bld_run_str_async(const char *name, const char *str)
{
    char *new_str = bld_strf("./%s %s", name, str);

    pid_t pid = bld_execute_str_async(new_str);

    free(new_str);

    return pid;
}

pid_t
bld_run_vargs_async(const char *name, const char *fmt, va_list args)
{
    char *new_fmt = bld_strf("./%s %s", name, fmt);

    pid_t pid = bld_execute_vargs_async(new_fmt, args);

    free(new_fmt);

    return pid;
}


#else // !defined(_WIN32)


#endif // !defined(_WIN32)


int
bld_execute_argv(char *argv[])
{
    return bld_wait_for(bld_execute_argv_async(argv));
}

pid_t
bld_execute_async(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    pid_t pid = bld_execute_vargs_async(fmt, args);

    va_end(args);

    return pid;
}

int
bld_execute(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    pid_t pid = bld_execute_vargs_async(fmt, args);

    va_end(args);

    return bld_wait_for(pid);
}

char *
bld_strf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);

    int to_write = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    va_end(args);

    char *buffer = malloc(to_write + 1);
    assert(buffer);

    vsnprintf(buffer, to_write + 1, fmt, args_copy);
    va_end(args_copy);

    return buffer;
}

int
bld_str_eq(const char *str_a, const char *str_b)
{
    for (; *str_a && *str_b; ++str_a, ++str_b) {
        if (*str_a != *str_b)
            return 0;
    }

    return *str_a == *str_b;
}

pid_t
bld_cc_params_nt_async(const char **params)
{
    int count = 0;
    for (; params[count]; ++count)
        ;;

    return bld_cc_params_async(params, count);
}

int
bld_cc_params(const char **params, int param_count)
{
    return bld_wait_for(bld_cc_params_async(params, param_count));
}

int
bld_cc_params_nt(const char **params)
{
    return bld_wait_for(bld_cc_params_nt_async(params));
}

int
bld_contains(const char *str, int argc, char *argv[])
{
    for (int i = 0; i < argc; ++i) {
        if (bld_str_eq(argv[i], str))
            return 1;
    }

    return 0;
}

int
bld_find(const char *str, int count, const char **list)
{
    for (int i = 0; i < count; ++i) {
        if (bld_str_eq(list[i], str))
            return i;
    }

    return -1;
}

char *
bld_join(const char *delim, int count, const char **list)
{
    size_t memory_size = 1;

    if (count > 0) {
        memory_size += (count - 1) * strlen(delim);
    }

    for (int i = 0; i < count; ++i) {
        memory_size += strlen(list[i]);
    }

    char *buffer = malloc(memory_size);
    assert(buffer);

    char *p = buffer;

    for (int i = 0; i < count; ++i) {
        if (i) {
            for (const char *s = delim; *s; ++s) {
                *(p++) = *s;
            }
        }

        for (const char *s = list[i]; *s; ++s) {
            *(p++) = *s;
        }
    }

    *p = 0;

    return buffer;
}

void
bld_sb_append(bld_sb_t *sb, const char *s)
{
    size_t s_len = strlen(s) + 1;
    size_t new_size = sb->size + s_len;

    if (new_size > sb->capacity) {
        if (sb->capacity == 0) {
            sb->capacity = 4096;
        }

        while (sb->capacity < new_size) {
            sb->capacity *= 2;
        }

        sb->data = realloc(sb->data, sb->capacity);
        assert(sb->data);
    }

    for (size_t i = 0; i < s_len; ++i) {
        sb->data[sb->size + i] = s[i];
    }

    sb->size += s_len - 1;
}

void
bld_sb_append_multiple(bld_sb_t *sb, const char **sl, int count)
{
    for (int i = 0; i < count; ++i) {
        bld_sb_append(sb, sl[i]);
    }
}

int
bld_try_rebuild_self(char *self, int argc, char *argv[])
{
    if (bld_file_time_cmp(bld_mod_time(self), bld_mod_time(argv[0])) < 0)
        return -1;

    printf("Recompiling '%s'.\n", self);

    int exe_len = strlen(argv[0]);

    char *move_dest = malloc(exe_len + 5);
    assert(move_dest);

    memcpy(move_dest, argv[0], exe_len);

    move_dest[exe_len + 0] = '.';
    move_dest[exe_len + 1] = 'o';
    move_dest[exe_len + 2] = 'l';
    move_dest[exe_len + 3] = 'd';
    move_dest[exe_len + 4] = '\0';

    if (bld_exists(move_dest)) {
        remove(move_dest);
    }

    rename(argv[0], move_dest);

    int res = BLD_CC(self, "-o", argv[0]);
    if (res) {
        rename(move_dest, argv[0]);
        return res;
    }

    return bld_execute_argv(argv);
}

int
bld_run_argv(const char *name, char *argv[])
{
    return bld_wait_for(bld_run_argv_async(name, argv));
}

int
bld_run_str(const char *name, const char *str)
{
    return bld_wait_for(bld_run_str_async(name, str));
}

pid_t
bld_run_async(const char *name, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    pid_t pid = bld_run_vargs_async(name, fmt, args);

    va_end(args);

    return pid;
}

int
bld_run(const char *name, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    pid_t pid = bld_run_vargs_async(name, fmt, args);

    va_end(args);

    return bld_wait_for(pid);
}

pid_t
bld_run_program_async(const char *name)
{
    return bld_run_async(name, "");
}

int
bld_run_program(const char *name)
{
    return bld_wait_for(bld_run_program_async(name));
}


#endif // defined(BLD_IMPLEMENTATION)

#endif // BLD_H_
