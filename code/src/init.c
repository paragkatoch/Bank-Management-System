// init.c
//============================================================================

// This file contains init function for the server

//============================================================================

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

// Create default admin user
static void __create_admin()
{
    User user;
    init_user(&user, 1, ADMIN_ROLE, 0, 1, "Admin", "23", "IIITB", "1000110001", "admin", "admin123");
    record__save(&user, sizeof(user), USER_DB);

    printf("User database initialized with default admin account\n");
}

static void __create_employees()
{
    User employee1;
    User employee2;
    User employee3;
    User employee4;

    init_user(&employee1, 2, EMPLOYEE_ROLE, 0, 1, "Devanshi", "23", "IIITB", "1000110001", "devanshi", "12345");
    init_user(&employee2, 3, EMPLOYEE_ROLE, 0, 1, "Diksha", "23", "IIITB", "1000110001", "diksha", "12345");
    init_user(&employee3, 4, EMPLOYEE_ROLE, 0, 1, "Parag", "23", "IIITB", "1000110001", "parag", "12345");
    init_user(&employee4, 5, MANAGER_ROLE, 0, 1, "Manager", "23", "IIITB", "1000110001", "manager", "manager123");

    record__save(&employee1, sizeof(User), USER_DB);
    record__save(&employee2, sizeof(User), USER_DB);
    record__save(&employee3, sizeof(User), USER_DB);
    record__save(&employee4, sizeof(User), USER_DB);

    printf("User database initialized with default employee accounts\n");
}

static void __create_accounts()
{
    __create_admin();
    __create_employees();
}

static void __create_db_files()
{
    int fd;

    fd = open(ACCOUNT_DB, O_CREAT, 0644);
    close(fd);

    fd = open(FEEDBACK_DB, O_CREAT, 0644);
    close(fd);

    fd = open(LOAN_DB, O_CREAT, 0644);
    close(fd);

    fd = open(TRANSACTION_DB, O_CREAT, 0644);
    close(fd);

    fd = open(USER_DB, O_CREAT, 0666);
    close(fd);
}

// Initialize db's and inital users
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
    __create_db_files();

    // create admin user
    __create_accounts();

    return 0;
}
