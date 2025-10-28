#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "db/user.h"
#include "file_operation.h"
#include "init.h"
#include "config.h"
#include "db/user.h"

#define DEFAULT_NAME "Admin"
#define DEFAULT_USERNAME "admin"
#define DEFAULT_PASSWORD "admin123"

void create_admin()
{
    User user;
    init_user(&user, 1001, 1, 0, 1, DEFAULT_NAME, "23", "IIITB", "1000110001", DEFAULT_USERNAME, DEFAULT_PASSWORD);
    record__save(&user, sizeof(user), USER_DB);

    printf("User database initialized with default admin account\n");
    printf("Username: %s\n", user.username);
    printf("Password: %s\n", user.password);
}

/**
 * Initialize db's and inital users
 */
int init()
{
    int fd;

    printf("Init running...\n");

    // Quit if db already exits
    fd = open(USER_DB, O_RDONLY);
    if (fd >= 0)
    {
        printf("User database already exists. Exiting...\n");
        close(fd);

        // logout everyone from last run
        user_logout_everyone();
        return 0;
    }

    printf("User database doesn't exists. Creating...\n");
    fd = open(USER_DB, O_CREAT, 0666);
    close(fd);

    // create admin user
    create_admin();

    return 0;
}
