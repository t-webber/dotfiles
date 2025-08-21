#pragma once

#include "lib.h"
#include <unistd.h>

pid_t fork_checked(void);
void fork_wait(pid_t pid);

#define exl(cmd, ...)                                                          \
	{                                                                      \
		int res = execlp(cmd, __VA_ARGS__, NULL);                      \
		panic("Failed to execute %s: exited with code %d.\n",          \
		      cmd,                                                     \
		      res)                                                     \
	}

#define forked_exl(cmd, ...)                                                   \
	{                                                                      \
		pid_t pid = fork_checked();                                    \
		if (pid == 0)                                                  \
			exl(cmd, __VA_ARGS__);                                 \
		fork_wait(pid);                                                \
	}

void exv(Args args);
void forked_exv(Args args);
