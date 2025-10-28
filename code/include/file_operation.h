// file_operation.h
//============================================================================

// This file contains logic for locking, reading and writing to files

//============================================================================

#include <sys/types.h>
#include <stddef.h>

#ifndef RECORD_UTILS_H
#define RECORD_UTILS_H

// =====================================================
// Lock Functions
// =====================================================
int __lock_unlock(int fd, short lock_type, off_t start, size_t len);
int lock_file(int fd, short lock_type);
int unlock_file(int fd);
int lock_record(int fd, short lock_type, off_t start, size_t len);
int unlock_record(int fd, off_t start, size_t len);

// =====================================================
// CRUD Functions
// =====================================================
int record__save(void *rec, size_t size, const char *filename);
int record__update(void *rec, size_t size, const char *filename, int pos);
int record__search(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx);
int record__search_and_update(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, void (*update)(void *));
int record__search_and_update_cont(void *rec, size_t size, const char *filename, int (*cmp)(void *, void *), void *ctx, void (*update)(void *));
int record__delete(size_t size, const char *filename, int (*cmp)(void *));

#endif
