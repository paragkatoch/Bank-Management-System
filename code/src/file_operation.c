#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include "file_operation.h"

// =====================================================
// Lock Function
// =====================================================

// Lock unlock operation
int __lock_unlock(int fd, short lock_type, off_t start, size_t len)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET;
    lock.l_start = start;
    lock.l_len = len;
    return fcntl(fd, F_SETLKW, &lock);
}

// Lock whole file
int lock_file(int fd, short lock_type)
{
    return __lock_unlock(fd, lock_type, 0, 0);
}

// Unlock whole file
int unlock_file(int fd)
{
    return __lock_unlock(fd, F_UNLCK, 0, 0);
}

// Lock a record
int lock_record(int fd, short lock_type, off_t start, size_t len)
{
    return __lock_unlock(fd, lock_type, start, len);
}

// Unlock a record
int unlock_record(int fd, off_t start, size_t len)
{
    return __lock_unlock(fd, F_UNLCK, start, len);
}

// =====================================================
// CRUD Functions
// =====================================================

/**
 * Save record
 *
 * Returns -
 *  - 0, on success
 *  - -1, on failure
 */
int record__save(void *rec, size_t size, const char *filename)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
        return -1;

    // lock entire file
    if (lock_file(fd, F_WRLCK) == -1)
    {
        close(fd);
        return -1;
    }

    ssize_t write_size = write(fd, rec, size);

    unlock_file(fd);
    close(fd);

    return (write_size == size) ? 0 : -1;
}

/**
 * Update record at position `pos`
 *
 * Returns -
 *  - 0, on success
 *  - -1, on failure
 */
int record__update(void *rec, size_t size, const char *filename, int pos)
{
    int fd = open(filename, O_RDWR);
    if (fd == -1)
        return -1;

    off_t offset = (off_t)pos * size;

    // lock record
    if (lock_record(fd, F_WRLCK, offset, size) == -1)
    {
        close(fd);
        return -1;
    }

    lseek(fd, offset, SEEK_SET);
    ssize_t write_size = write(fd, rec, size);

    unlock_record(fd, offset, size);
    close(fd);

    return (write_size == size) ? 0 : -1;
}

/**
 * Search record
 *
 * Returns -
 *  - Pos, on sucess
 *  - -1, on failure
 */
int record__search(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return -1;

    if (lock_file(fd, F_RDLCK) == -1)
    {
        close(fd);
        return -1;
    }

    ssize_t n;
    int index = 0;

    while ((n = read(fd, rec, size)) == size)
    {
        if (cmp(rec, ctx))
        {
            unlock_file(fd);
            close(fd);
            return index;
        }
        index++;
    }

    unlock_file(fd);
    close(fd);
    return -1;
}

/**
 * Delete records
 *
 * Returns -
 *  - 0, on success
 *  - -1, on failure
 */
int record__delete(size_t size, const char *filename, int (*cmp)(void *))
{
    int fd = open(filename, O_RDWR);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    // Create temp file
    char tmpname[] = "/tmp/banking_system_tmpXXXXXX";
    int tmp_fd = mkstemp(tmpname);
    if (tmp_fd == -1)
    {
        perror("mkstemp");
        close(fd);
        return -1;
    }

    void *buf = malloc(size);
    if (!buf)
    {
        perror("malloc");
        close(fd);
        close(tmp_fd);
        return -1;
    }

    off_t offset = 0;
    ssize_t n;

    // Copy records from fd to tmp_fd

    // Read each record
    while (1)
    {
        // lock record
        if (lock_record(fd, F_RDLCK, offset, size) == -1)
            break;

        // move the curosr
        if (lseek(fd, offset, SEEK_SET) == -1)
        {
            unlock_record(fd, offset, size);
            break;
        }

        // read record
        n = read(fd, buf, size);
        if (n <= 0)
        {
            unlock_record(fd, offset, size);
            break;
        }

        // if not the record to be deleted write it to tmp file
        if (!cmp(buf))
            write(tmp_fd, buf, size);

        // unlock record
        unlock_record(fd, offset, size);
        offset += size;
    }

    // Rewrite whole file

    // lock file
    if (lock_file(fd, F_WRLCK) == -1)
    {
        perror("lock_file");
        free(buf);
        close(fd);
        close(tmp_fd);
        return -1;
    }

    // Truncate and rewrite from tmp file
    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    lseek(tmp_fd, 0, SEEK_SET);

    while ((n = read(tmp_fd, buf, size)) > 0)
        write(fd, buf, n);

    // Cleanup

    unlock_file(fd);
    free(buf);
    close(tmp_fd);
    close(fd);
    unlink(tmpname);

    return 0;
}