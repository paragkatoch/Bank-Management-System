#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "communication.h"
#include "file_operation.h"
#include "db/user.h"
#include "helper.h"
#include "start_screen.h"

// =======================================
// User method Declarations
// =======================================

/**
 * Init User
 */

void init_user(User *user, int userId, int role, int s_a, int a_a, char *name, int age, char *address, char *phone, char *username, char *password)
{
    memset(user, 0, sizeof(User));

    user->userId = userId;
    user->role = role;
    user->session_active = s_a;
    user->account_active = a_a;
    user->age = age;

    safe_strncpy(user->name, name, sizeof(user->name));
    safe_strncpy(user->address, address, sizeof(user->address));
    safe_strncpy(user->phone, phone, sizeof(user->phone));
    safe_strncpy(user->username, username, sizeof(user->username));
    safe_strncpy(user->password, password, sizeof(user->password));
}

/**
 * Login
 */

// username password struct
typedef struct
{
    char username[20];
    char password[20];
} __user_login_usernamePassword;

// user login compare function for searching user record
int __user_login_compare_username_password(void *rec, void *ctx)
{
    User *user = (User *)rec;
    __user_login_usernamePassword *login = (__user_login_usernamePassword *)ctx;

    if ((strcmp(user->username, login->username) == 0) &&
        (strcmp(user->password, login->password) == 0) &&
        (user->session_active == 0))
    {
        return 1; // match found
    }
    return 0; // no match
}
// find user record based on username and password
int __user_login_find_user_record(char *username, char *password)
{
    User tempUser;
    __user_login_usernamePassword username_password;

    strncpy(username_password.username, username, sizeof(username_password.username) - 1);
    strncpy(username_password.password, password, sizeof(username_password.password) - 1);

    int index = record__search(&tempUser, sizeof(tempUser), USER_DB, &__user_login_compare_username_password, &username_password);

    if (index == -1)
    {
        return 0;
    }
    else
    {
        // TODO: mulitple session can be created since we don't lock from start to end
        // soln use the update cmp function...if found the user with everything correct update its session there hehe

        logged_in_user = tempUser;
        logged_in_user.session_active = 1;

        logged_in_user_index = index;
        record__update(&logged_in_user, sizeof(logged_in_user), USER_DB, logged_in_user_index);

        return 1;
    }
}

/**
 * User login handler
 */
void user_login()
{
    char *username = NULL;
    char *password = NULL;
    int attempts = 3;
    int fd = clientfd;

    while (attempts > 0)
    {
        clear_terminal(fd);

        send_message(fd, "=====================================");
        send_message(fd, "         Welcome to Login Portal     ");
        send_message(fd, "=====================================\n");

        // --- Username ---
        send_message(fd, "Enter username (-1 to go back): ");
        if (receive_message(fd, &username) < 0)
        {
            send_message(fd, "Internal server error");
            break;
        }

        if (strcmp(username, "-1") == 0)
        {
            free(username);
            break;
        }

        // --- Password ---
        send_message(fd, "Enter password (-1 to go back): ");
        if (receive_message(fd, &password) < 0)
        {
            send_message(fd, "Internal server error");
            free(username);
            break;
        }
        if (strcmp(password, "-1") == 0)
        {
            free(username);
            free(password);
            break;
        }

        int login_successful = __user_login_find_user_record(username, password);

        if (login_successful)
        {
            clear_terminal(fd);
            send_message(fd, "Login successful! ✅");
            send_message(fd, "Proceeding");

            // Simple timer
            for (int i = 0; i < 3; i++)
            {
                send_message(fd, ".");
                fflush(stdout);
                sleep(1);
            }
            free(username);
            free(password);
            return showStartScreen();
        }
        else
        {
            attempts--;
            char msg[128];
            snprintf(msg, sizeof(msg), "\nLogin failed!. Attempts left: %d", attempts);
            send_message(fd, msg);
            sleep(2);

            free(username);
            free(password);
        }
    }
}

// Create and save new user
void user_create_employee() {}

void user_create_customer() {}

// Update user details
void user_change_user_details() {}

// View user details
void user_view_user_details() {}

// Change Password
void user_change_password() {}

// Activate user
void user_activate_user() {}

// Deactivate user
void user_deactivate_user() {}

// Change user role
void user_change_user_role() {}

// Logout
void user_logout()
{
    logged_in_user.session_active = 0;
    record__update(&logged_in_user, sizeof(logged_in_user), USER_DB, logged_in_user_index);

    logged_in_user = (User){0};
    logged_in_user_index = -1;
    // memset(&logged_in_user, 0, sizeof(logged_in_user));
    return user_login();
}

// void user_logout_everyone(){

// }