// Copyright (C) 2021 mana
//
// This file is part of ddrescue-windows.
//
// ddrescue-windows is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// ddrescue-windows is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ddrescue-windows.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGEMT		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGBUS		10
#define SIGSEGV		11
#define SIGSYS		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGURG		16
#define SIGSTOP		17
#define SIGTSTP		18
#define SIGCONT		19
#define SIGCHLD		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGIO		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGINFO		29
#define SIGUSR1		30
#define SIGUSR2		31

#define SIGPOLL	SIGIO
#define SIGPWR	SIGINFO
#define SIGIOT	SIGABRT

#include <csignal>
#define NOMINMAX
#include <Windows.h>
#include <string>
#include <vector>
#define SA_ONSTACK	0x00000001
#define SA_RESTART	0x00000002
#define SA_NOCLDSTOP	0x00000004
#define SA_NODEFER	0x00000008
#define SA_RESETHAND	0x00000010
#define SA_NOCLDWAIT	0x00000020
#define SA_SIGINFO	0x00000040

#define STDIN_FILENO 0

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define S_IFMT  00170000
#define S_IFSOCK 0140000
#define S_IFLNK	 0120000
#define S_IFREG  0100000
#define S_IFBLK  0060000
#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFIFO  0010000
#define S_ISUID  0004000
#define S_ISGID  0002000
#define S_ISVTX  0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

typedef unsigned int         mode_t;

struct siginfo_t
{
    // int      si_signo;    /* Signal number */
    // int      si_errno;    /* An errno value */
    // int      si_code;     /* Signal code */
    // int      si_trapno;   /* Trap number that caused
    //                          hardware-generated signal
    //                          (unused on most architectures) */
    // pid_t    si_pid;      /* Sending process ID */
    // uid_t    si_uid;      /* Real user ID of sending process */
    // int      si_status;   /* Exit value or signal */
    // clock_t  si_utime;    /* User time consumed */
    // clock_t  si_stime;    /* System time consumed */
    // sigval_t si_value;    /* Signal value */
    // int      si_int;      /* POSIX.1b signal */
    // void* si_ptr;      /* POSIX.1b signal */
    // int      si_overrun;  /* Timer overrun count; POSIX.1b timers */
    // int      si_timerid;  /* Timer ID; POSIX.1b timers */
    // void* si_addr;     /* Memory location which caused fault */
    // long     si_band;     /* Band event (was int in
    //                          glibc 2.3.2 and earlier) */
    // int      si_fd;       /* File descriptor */
    // short    si_addr_lsb; /* Least significant bit of address
    //                          (since kernel 2.6.32) */
};

typedef struct {
	// unsigned long sig[_NSIG_WORDS];
} sigset_t;

struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t*, void*);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
};


int sigaction(int signum, const struct sigaction* act,
    struct sigaction* oldact);

int sigemptyset(sigset_t* set);

#define TCIFLUSH 0

int tcflush(int fildes, int queue_selector);

struct PhyDiskInfo
{
    std::wstring Name;
    std::wstring Model;
    long long Size;
    long long BytesPerSector;
    long long TotalCylinders;
};

HRESULT GetDevicesInfo();

int __cdecl OpenLowLevel(
    _In_z_ char const* _FileName,
    _In_   int         _OpenFlag,
    ...
);

long long __cdecl LseekFixed(
    _In_ int  _FileHandle,
    _In_ long long _Offset,
    _In_ int  _Origin
);
