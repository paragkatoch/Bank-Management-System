// file_operation.c
//============================================================================

// This file contains logic for locking, reading and writing to files

//============================================================================

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
static int __lock_unlock(int fd, short lock_type, off_t start, size_t len)
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

// Lock whole file
int lock_file_fd(const char *filename, short lock_type)
{
    int fd = open(filename, O_RDWR);
    if (__lock_unlock(fd, lock_type, 0, 0) == -1)
    {
        close(fd);
        return -1;
    }

    return fd;
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

// Lock a record
int lock_record_fd(const char *filename, short lock_type, off_t start, size_t len)
{
    int fd = open(filename, O_RDWR);
    if (__lock_unlock(fd, lock_type, start, len) == -1)
    {
        close(fd);
        return -1;
    }

    return fd;
}

// Unlock a record
int unlock_record(int fd, off_t start, size_t len)
{
    return __lock_unlock(fd, F_UNLCK, start, len);
}

// =====================================================

// CRUD Functions

// =====================================================

// Save record
int record__save(void *rec, size_t size, const char *filename, int lock)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
        return -1;

    // lock entire file
    if ((lock == RECORD_USE_LOCK) && lock_file(fd, F_WRLCK) == -1)
    {
        close(fd);
        return -1;
    }

    ssize_t write_size = write(fd, rec, size);

    (lock == RECORD_USE_LOCK) && unlock_file(fd);
    close(fd);

    return (write_size == size) ? 0 : -1;
}

// Update record at position `pos`
int record__update(void *rec, size_t size, const char *filename, int pos, int lock)
{
    int fd = open(filename, O_RDWR);
    if (fd == -1)
        return -1;

    off_t offset = (off_t)pos * size;

    // lock record
    if ((lock == RECORD_USE_LOCK) && lock_record(fd, F_WRLCK, offset, size) == -1)
    {
        close(fd);
        return -1;
    }

    lseek(fd, offset, SEEK_SET);
    ssize_t write_size = write(fd, rec, size);

    (lock == RECORD_USE_LOCK) && unlock_record(fd, offset, size);
    close(fd);

    return (write_size == size) ? 0 : -1;
}

// Search record
int record__search(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, int lock)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return -1;

    if ((lock == RECORD_USE_LOCK) && lock_file(fd, F_RDLCK) == -1)
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

    (lock == RECORD_USE_LOCK) && unlock_file(fd);
    close(fd);
    return -1;
}

int record__search_cont(void **recs, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, int lock)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return -1;

    if ((lock == RECORD_USE_LOCK) && lock_file(fd, F_RDLCK) == -1)
    {
        close(fd);
        return -1;
    }

    ssize_t n;
    int index = 0;
    int buff_size = size;
    void *rec = malloc(size);
    *recs = (char *)malloc((size_t)buff_size);

    while ((n = read(fd, rec, size)) == size)
    {
        if (index * size >= buff_size)
        {
            buff_size *= 2;
            void *temp = (char *)realloc(*recs, buff_size);

            if (!temp)
            {
                free(temp);
                free(rec);
                (lock == RECORD_USE_LOCK) && unlock_file(fd);
                close(fd);
                return -1;
            }

            *recs = temp;
        }
        if (cmp(rec, ctx))
        {
            memcpy((char *)*recs + (index * size), rec, size);
            index++;
        }
    }

    (lock == RECORD_USE_LOCK) && unlock_file(fd);
    close(fd);
    free(rec);
    return index;
}

// Search and update a record using cmp, ctx and update
int record__search_and_update(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, void (*update)(void *), int lock)
{
    int fd = open(filename, O_RDWR);
    if (fd == -1)
        return -1;

    if ((lock == RECORD_USE_LOCK) && lock_file(fd, F_RDLCK) == -1)
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
            off_t offset = (off_t)index * size;

            if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
            {
                perror("lseek");
                break;
            }

            update(rec);

            ssize_t w = write(fd, rec, size);
            if (w != size)
            {
                perror("write");
                break;
            }

            (lock == RECORD_USE_LOCK) && unlock_file(fd);
            close(fd);
            return index;
        }
        index++;
    }

    (lock == RECORD_USE_LOCK) && unlock_file(fd);
    close(fd);
    return -1;
}

// Search and update records using cmp, ctx and update
int record__search_and_update_cont(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, void (*update)(void *), int lock)
{
    int fd = open(filename, O_RDWR);
    if (fd == -1)
        return -1;

    if ((lock == RECORD_USE_LOCK) && lock_file(fd, F_RDLCK) == -1)
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
            off_t offset = (off_t)index * size;

            if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
            {
                perror("lseek");
                break;
            }

            update(rec);

            ssize_t w = write(fd, rec, size);
            if (w != size)
            {
                perror("write");
                break;
            }

            offset = (off_t)(index + 1) * size;

            if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
            {
                perror("lseek");
                break;
            }
        }
        index++;
    }

    (lock == RECORD_USE_LOCK) && unlock_file(fd);
    close(fd);
    return -1;
}

// Delete records
int record__delete(size_t size, const char *filename, int (*cmp)(void *), int lock)
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

// Get last record
int record_end_record(void *rec, size_t size, const char *filename, int lock)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return -1;

    if ((lock == RECORD_USE_LOCK) && lock_file(fd, F_RDLCK) == -1)
    {
        close(fd);
        return -1;
    }

    off_t offset = (off_t)-1 * size;

    if (lseek(fd, offset, SEEK_END) == -1)
    {
        goto failure;
    }

    ssize_t n;

    if ((n = read(fd, rec, size)) != size)
    {
        goto failure;
    }

    (lock == RECORD_USE_LOCK) && unlock_file(fd);
    close(fd);
    return 0;

failure:
    unlock_file(fd);
    close(fd);
    return -1;
}
