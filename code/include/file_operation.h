// file_operation.h
//============================================================================

// This file contains logic for locking, reading and writing to files

//============================================================================

#include <sys/types.h>
#include <stddef.h>

#ifndef RECORD_UTILS_H
#define RECORD_UTILS_H

#define RECORD_USE_LOCK 1
#define RECORD_NOT_USE_LOCK 0

// =====================================================
// Lock Functions
// =====================================================
int lock_file(int fd, short lock_type);
int unlock_file(int fd);
int lock_record(int fd, short lock_type, off_t start, size_t len);
int unlock_record(int fd, off_t start, size_t len);
int lock_file_fd(const char *filename, short lock_type);
int lock_record_fd(const char *filename, short lock_type, off_t start, size_t len);

// =====================================================
// CRUD Functions
// =====================================================
int record__save(void *rec, size_t size, const char *filename, int lock);
int record__update(void *rec, size_t size, const char *filename, int pos, int lock);
int record__update_fd(int fd, void *rec, size_t size, int pos);
int record__search(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, int lock);
int record__search_fd(int fd, void *rec, size_t size, int (*cmp)(void *, void *), void *ctx);
int record__search_cont(void **recs, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, int lock);
int record__search_and_update(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, void (*update)(void *), int lock);
int record__search_and_update_cont(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, void (*update)(void *), int lock);
int record__delete(size_t size, const char *filename, int (*cmp)(void *), int lock);
int record_end_record(void *rec, size_t size, const char *filename, int lock);

#endif
