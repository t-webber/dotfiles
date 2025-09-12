#pragma once

#include "lib.h"
#include <sys/types.h>
#include <unistd.h>

pid_t fork_checked(void);
void fork_wait(pid_t pid);
__wur bool is_dbg(void);

#define exl(first, ...)                                                        \
        {                                                                      \
                if (is_dbg()) print_inline_variadic(__VA_ARGS__);              \
                int res = execlp(first, __VA_ARGS__);                          \
                epanic("Failed to execute %s: exited with code %d",            \
                       first,                                                  \
                       res)                                                    \
        }

#define exl1(cmd) exl(cmd, cmd, NULL);

#define exln(cmd, ...) exl(cmd, __VA_ARGS__, NULL);

#define exldn(cmd, ...) exl(cmd, cmd, __VA_ARGS__, NULL);

#define forked_exldn(cmd, ...)                                                 \
        if (fork_and_wait()) exldn(cmd, __VA_ARGS__);

#define __read_simple_exl_maker(__exl_func, buf_size, buffer, ...)             \
        int fildes[2];                                                         \
        if (pipe(fildes)) { epanic("Failed to create fildes pipe"); }          \
                                                                               \
        pid_t pid = fork_checked();                                            \
                                                                               \
        if (pid == 0) {                                                        \
                if (close(fildes[0])) { epanic("Failed to free reader"); }     \
                if (dup2(fildes[1], STDOUT_FILENO) < 0) {                      \
                        epanic("Failed to link stdout to writer");             \
                }                                                              \
                if (close(fildes[1])) { epanic("Failed to free writer") };     \
                                                                               \
                __exl_func(__VA_ARGS__);                                       \
        }                                                                      \
                                                                               \
        if (close(fildes[1])) { epanic("Failed to free writer"); }             \
                                                                               \
        ssize_t nbytes;                                                        \
        char *buffer = malloc(sizeof(char) * buf_size);                        \
                                                                               \
        if ((nbytes = read(fildes[0], buffer, buf_size - 1)) == 0) {           \
                epanic("Failed to read output of command")                     \
        }                                                                      \
                                                                               \
        fork_wait(pid);                                                        \
        if (close(fildes[0])) { epanic("Failed to free reader"); }             \
                                                                               \
        buffer[nbytes + 2] = '\0';

#define read_simple_exldn(buf_size, buffer, ...)                               \
        __read_simple_exl_maker(exldn, buf_size, buffer, __VA_ARGS__)
#define read_simple_exl1(buf_size, buffer, ...)                                \
        __read_simple_exl_maker(exl1, buf_size, buffer, __VA_ARGS__)

_Noreturn __nonnull() void exvd(Args args);
__nonnull() void forked_exvd(Args args);

_Noreturn __nonnull() void exl_notif(const_str message);

#define exl_corenotif(...)                                                     \
        {                                                                      \
                char message[128];                                             \
                sprintf(message, __VA_ARGS__);                                 \
                exl_notif(message);                                            \
        }

_Noreturn __nonnull() void exl_err_notif(const_str msg);

__wur bool fork_and_wait(void);
