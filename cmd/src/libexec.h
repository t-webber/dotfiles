#pragma once

#include "lib.h"
#include <unistd.h>

pid_t fork_checked(void);
void fork_wait(pid_t pid);

#define exl(cmd, ...)                                                          \
        {                                                                      \
                int res = execlp(cmd, __VA_ARGS__, NULL);                      \
                panic("Failed to execute %s: exited with code %d", cmd, res)   \
        }

#define forked_exl(cmd, ...)                                                   \
        {                                                                      \
                pid_t pid = fork_checked();                                    \
                if (pid == 0) exl(cmd, __VA_ARGS__);                           \
                fork_wait(pid);                                                \
        }

#define read_simple_exl(buf_size, ...)                                         \
        int fildes[2];                                                         \
        if (pipe(fildes)) { panic("Failed to create fildes pipe"); }           \
                                                                               \
        pid_t pid = fork_checked();                                            \
                                                                               \
        if (pid == 0) {                                                        \
                if (close(fildes[0])) { panic("Failed to free reader"); }      \
                if (dup2(fildes[1], STDOUT_FILENO) < 0) {                      \
                        panic("Failed to link stdout to writer");              \
                }                                                              \
                if (close(fildes[1])) { panic("Failed to free writer") };      \
                                                                               \
                exl(__VA_ARGS__);                                              \
        }                                                                      \
                                                                               \
        if (close(fildes[1])) { panic("Failed to free writer"); }              \
                                                                               \
        ssize_t nbytes;                                                        \
        char *buffer = malloc(sizeof(char) * buf_size);                        \
                                                                               \
        if ((nbytes = read(fildes[0], buffer, buf_size - 1)) == 0) {           \
                panic("Failed to read output of command")                      \
        }                                                                      \
                                                                               \
        fork_wait(pid);                                                        \
        if (close(fildes[0])) { panic("Failed to free reader"); }              \
                                                                               \
        buffer[nbytes + 2] = '\0';

void exv(Args args);
void forked_exv(Args args);
