/*
 * syscalls.c
 *
 * Minimal newlib syscall implementation for STM32F407.
 * printf() output is sent through ARM semihosting.
 *
 * IMPORTANT:
 *   - Run under the debugger with semihosting enabled.
 *   - If you run without a debugger, the BKPT 0xAB instruction may stop/fault.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#define SEMIHOSTING_SYS_WRITEC   0x03

static inline int semihosting_call(int reason, void *arg)
{
    register int r0 __asm("r0") = reason;
    register void *r1 __asm("r1") = arg;

    __asm volatile (
        "bkpt 0xAB"
        : "+r" (r0)
        : "r" (r1)
        : "memory"
    );

    return r0;
}

static void semihosting_putchar(char c)
{
    semihosting_call(SEMIHOSTING_SYS_WRITEC, &c);
}

int _write(int file, char *ptr, int len)
{
    (void)file;

    for (int i = 0; i < len; i++)
    {
        semihosting_putchar(ptr[i]);
    }

    return len;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

int _close(int file)
{
    (void)file;
    errno = EBADF;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;

    if (st != NULL)
    {
        st->st_mode = S_IFCHR;
    }

    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

off_t _lseek(int file, off_t offset, int whence)
{
    (void)file;
    (void)offset;
    (void)whence;
    return 0;
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

void _exit(int status)
{
    (void)status;
    while (1)
    {
    }
}
