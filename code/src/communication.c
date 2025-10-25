#include "communication.h"
#include "config.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/**
 * Send message using MSG_DELIM
 * Returns 0 on success, -1 on failure
 */
int send_message(int fd, const char *msg)
{
    size_t msg_len = strlen(msg);
    size_t delim_len = strlen(MSG_DELIM);

    if (write(fd, msg, msg_len) != msg_len)
        return -1;
    if (write(fd, MSG_DELIM, delim_len) != delim_len)
        return -1;
    return 0;
}

/**
 * Receive a message until delimiter is found
 * Returns -
 *  - 0 on success,
 *  - -1 on error/disconnect
 */
int receive_message(int fd, char **buffer)
{
    char win[16] = {0}, c;
    size_t buf_size = 128;
    size_t i = 0, dlen = strlen(MSG_DELIM);

    *buffer = (char *)malloc(buf_size);
    if (!*buffer)
    {
        return -1;
    }

    while (read(fd, &c, 1) > 0)
    {
        // if buffer full
        if (i + 1 >= buf_size)
        {
            // increase buffer size by 2
            buf_size *= 2;

            *buffer = (char *)realloc(*buffer, buf_size);
            if (!*buffer)
                return -1;
        }

        (*buffer)[i++] = c;

        // check for Msg_delim
        memmove(win, win + 1, dlen - 1);
        win[dlen - 1] = c;

        if ((dlen == 1 && win[0] == MSG_DELIM[0]) || (dlen > 1 && strncmp(win, MSG_DELIM, dlen) == 0))
        {
            break;
        }
    }

    // set last char of buffer to end deliminator
    (*buffer)[i - dlen + (dlen == 1 ? 1 : 0)] = '\0';

    return 0;
}