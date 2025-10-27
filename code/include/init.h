#include <stddef.h>

#ifndef INIT_H
#define INIT_H

int init();

void create_admin();
void logout_everyone();
void update_session(void *val);
int true_func(void *a, void *b);

#endif