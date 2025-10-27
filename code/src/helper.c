#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "communication.h"
#include "config.h"
#include "db/user.h"

/**
 * Copy src to dest string and set end deliminator
 */
void safe_strncpy(char *dest, const char *src, size_t n)
{
    if (src)
    {
        strncpy(dest, src, n - 1);
        dest[n - 1] = '\0';
    }
    else
    {
        dest[0] = '\0';
    }
}

/**
 * Clear user terminal
 */
void clear_terminal(int fd)
{
    // Send ANSI escape sequence to clear screen and move cursor to top
    send_message(fd, "\033[2J\033[H");
}

/**
 * Handle Internal Server error
 */
void server_error()
{
    send_message(clientfd, "\n\n Internal Server Error\n\n");
    send_message(clientfd, "Logging out...");
    sleep(1);
    user_logout();
}