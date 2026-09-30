/**
 * @file syscalls.c
 * @brief Minimal newlib system calls for bare-metal GCC builds.
 */

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "stm32_template_port.h"

int _write(int file, char *ptr, int len)
{
    const stm32_template_port_t *port = stm32_template_port_get();
    (void)file;
    if ((ptr == NULL) || (len <= 0)) return 0;
    if ((port == NULL) || (port->console_uart == NULL)) return len;
    return (HAL_UART_Transmit(port->console_uart, (uint8_t *)ptr,
                             (uint16_t)len, 1000U) == HAL_OK) ? len : -1;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    errno = ENOSYS;
    return -1;
}

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    (void)file;
    if (st != NULL) st->st_mode = S_IFCHR;
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

