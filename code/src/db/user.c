// user.c
//============================================================================

// This file contains logic and UI related to user

//============================================================================

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

//============================================================================

// Init User

//============================================================================

// Init User
void init_user(User *user, int userId, int role, int s_a, int a_a, char *name, char *age, char *address, char *phone, char *username, char *password)
{
    memset(user, 0, sizeof(User));

    user->userId = userId;
    user->role = role;
    user->session_active = s_a;
    user->account_active = a_a;

    safe_strncpy(user->name, name, sizeof(user->name));
    safe_strncpy(user->age, age, sizeof(user->age));
    safe_strncpy(user->address, address, sizeof(user->address));
    safe_strncpy(user->phone, phone, sizeof(user->phone));
    safe_strncpy(user->username, username, sizeof(user->username));
    safe_strncpy(user->password, password, sizeof(user->password));
}

//============================================================================

// User Login

//============================================================================

// struct to contain username and password
typedef struct
{
    char username[20];
    char password[20];
} __ul_user;
// function to update session of User
static void __ul_update_session(void *rec)
{
    User *user = (User *)rec;
    user->session_active = 1;
}
// function to compare user with username, password and session
static int __ul_cmp_user(void *rec, void *ctx)
{
    User *user = (User *)rec;
    __ul_user *login = (__ul_user *)ctx;

    if ((strcmp(user->username, login->username) == 0) &&
        (strcmp(user->password, login->password) == 0) &&
        (user->session_active == 0))
    {
        return 1; // match found
    }
    return 0; // no match
}
// function to find user on the basis of username and password
static int __ul_find_user(char *username, char *password)
{
    User tempUser;
    __ul_user ul_user;

    safe_strncpy(ul_user.username, username, sizeof(ul_user.username) - 1);
    safe_strncpy(ul_user.password, password, sizeof(ul_user.password) - 1);

    int index = record__search_and_update(&tempUser, sizeof(tempUser), USER_DB, &__ul_cmp_user, &ul_user, &__ul_update_session);

    if (index == -1)
    {
        return 0;
    }
    else
    {
        logged_in_user = tempUser;
        logged_in_user_index = index;
        return 1;
    }
}

// UI - User login handler
void user_login()
{
    int fd = clientfd;
    int attempts = 3;
    const char *err_msg = NULL;

    char *username = NULL;
    char *password = NULL;

    while (attempts > 0)
    {
        clear_terminal(fd);

        send_message(fd, "=====================================");
        send_message(fd, "         Welcome to Login Portal     ");
        send_message(fd, "=====================================\n");

        // --- Username ---
        send_message(fd, "\nEnter username (-1 to exit): ");
        if (receive_message(fd, &username) < 0)
        {
            err_msg = "Internal server error";
            goto cleanup;
        }
        else if (strcmp(username, "-1") == 0)
            goto cleanup;

        // --- Password ---
        send_message(fd, "\nEnter password (-1 to exit): ");
        if (receive_message(fd, &password) < 0)
        {
            err_msg = "Internal server error";
            goto cleanup;
        }
        else if (strcmp(password, "-1") == 0)
            goto cleanup;

        int login_successful = __ul_find_user(username, password);

        if (login_successful)
        {
            clear_terminal(fd);
            send_message(fd, "\nLogin successful!");
            send_message(fd, "Proceeding");

            // Simple timer
            for (int i = 0; i < 3; i++)
            {
                send_message(fd, ".");
                fflush(stdout);
                sleep(1);
            }

            goto success;
        }
        else
        {
            attempts--;
            char msg[128];
            snprintf(msg, sizeof(msg), "\nLogin failed!. Attempts left: %d", attempts);
            send_message(fd, msg);
            sleep(2);
        }
    }

cleanup:
    if (err_msg)
    {
        send_message(fd, err_msg);
        sleep(2);
    }
    free(username);
    free(password);
    return;

success:
    free(username);
    free(password);
    return showStartScreen();
}

//============================================================================

// Create new employee

//============================================================================

// Create and save new user
static int __uce_prompt_user_input(int fd, const char *message, char **out)
{
    send_message(fd, message);
    if (receive_message(fd, out) < 0)
    {
        send_message(fd, "\nError receiving input.\n");
        sleep(2);
        return -2;
    }

    if (strcmp(*out, "-1") == 0)
    {
        send_message(fd, "\nOperation cancelled by user.\n");
        sleep(2);
        return -1;
    }

    return 0;
}

// Create new employee
void user_create_employee()
{

    int fd = clientfd;
    User user;
    char *name = NULL;
    char *address = NULL;
    char *phone = NULL;
    char *username = NULL;
    char *password = NULL;
    char *age = NULL;

    int userId = 2001;      // auto-generate
    int role = 2;           // 1 = admin, 2 = employee
    int session_active = 0; // not logged in
    int account_active = 1; // account active

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "     Employee Creation Portal        \n");
    send_message(fd, "=====================================\n");

    send_message(fd, "\n Use -1 to cancel \n\n");

    // get updated details
    if (__uce_prompt_user_input(fd, "\nEnter employee name: ", &name) != 0 ||
        __uce_prompt_user_input(fd, "\nEnter employee age: ", &age) != 0 ||
        __uce_prompt_user_input(fd, "\nEnter employee address: ", &address) != 0 ||
        __uce_prompt_user_input(fd, "\nEnter phone number: ", &phone) != 0 ||
        __uce_prompt_user_input(fd, "\nEnter username: ", &username) != 0 ||
        __uce_prompt_user_input(fd, "\nEnter password: ", &password) != 0)
        goto cleanup;

    // create user with new details
    init_user(&user, userId, role, session_active, account_active,
              name, age, address, phone, username, password);

    clear_terminal(fd);
    send_message(fd, "\nEmployee account created successfully!\n");

    // save user to db
    if (record__save(&user, sizeof(User), USER_DB) != 0)
        send_message(fd, "Unable to save employee details in database.\n");
    else
        send_message(fd, "Employee details saved to database.\n");

    send_message(fd, "\n\n\nPress enter to continue...");
    receive_message(fd, &name);

    // cleanup
cleanup:
    free(name);
    free(age);
    free(address);
    free(phone);
    free(username);
    free(password);
    return showStartScreen();
}

//============================================================================

// Create new customer

//============================================================================

// UI - Create new customer
void user_create_customer() {}

//============================================================================

// Change User details

//============================================================================

// user login compare function for searching user record
static int __ucud_cmp_username(void *rec, void *ctx)
{
    User *user = (User *)rec;
    // __user_login_username *login = (__user_login_username *)ctx;
    char *username = (char *)ctx;

    if (strcmp(user->username, username) == 0)
    {
        return 1; // match found
    }
    return 0; // no match
}
// find user record based only on username
static int __ucud_find_user_with_username(char *username, User *tempUser)
{
    int index = record__search(tempUser, sizeof(User), USER_DB, __ucud_cmp_username, username);
    return index;
}
// prompt user input
static int __ucud_prompt_user_input_update(int fd, const char *message, char **out, char *original)
{
    send_message(fd, message);

    if (receive_message(fd, out) < 0)
        return -1; // communication error

    if (strcmp(*out, "-1") == 0)
        return -1; // cancel update

    if (strcmp(*out, "0") == 0)
    {
        free(*out);
        *out = strdup(original);
        send_message(fd, "\nskipped");
        sleep(1);
        return 0; // skip field
    }

    return 1;
}

// UI - Change User details
void user_change_user_details()
{
    int fd = clientfd;
    User user;
    char *name = NULL;
    char *address = NULL;
    char *phone = NULL;
    char *username = NULL;
    char *password = NULL;
    char *age = NULL;

    clear_terminal(fd);
    send_message(fd, "=====================================\n");
    send_message(fd, "        Enter details to update       \n");
    send_message(fd, "=====================================\n");

    // Get username to update
    if (__uce_prompt_user_input(fd, "\nEnter username to update details for: ", &username) != 0)
        goto cleanup;

    int index = __ucud_find_user_with_username(username, &user);
    if (index == -1)
    {
        send_message(fd, "\nUser not found.\n");
        sleep(2);
        goto cleanup;
    }

    clear_terminal(fd);
    send_message(fd, "\nEnter 0 to skip a field, or -1 to cancel update entirely.\n");

    // Name
    send_message(fd, "\nCurrent name is: ");
    send_message(fd, user.name);
    if (__ucud_prompt_user_input_update(fd, "\nEnter new name: ", &name, user.name) == -1)
        goto cleanup;

    // Age
    clear_terminal(fd);
    send_message(fd, "\nCurrent age is: ");
    send_message(fd, user.age);
    if (__ucud_prompt_user_input_update(fd, "\nEnter new age: ", &age, user.age) == -1)
        goto cleanup;

    // Address
    clear_terminal(fd);
    send_message(fd, "\nCurrent Address is: ");
    send_message(fd, user.address);
    if (__ucud_prompt_user_input_update(fd, "\nEnter new address: ", &address, user.address) == -1)
        goto cleanup;

    // Phone
    clear_terminal(fd);
    send_message(fd, "\nCurrent phone number is: ");
    send_message(fd, user.phone);
    if (__ucud_prompt_user_input_update(fd, "\nEnter new phone number: ", &phone, user.phone) == -1)
        goto cleanup;

    // Username
    clear_terminal(fd);
    send_message(fd, "\nCurrent username is: ");
    send_message(fd, user.username);
    if (__ucud_prompt_user_input_update(fd, "\nEnter new username: ", &username, user.username) == -1)
        goto cleanup;

    // Password
    clear_terminal(fd);
    send_message(fd, "\nCurrent password is: ");
    send_message(fd, user.password);
    if (__ucud_prompt_user_input_update(fd, "\nEnter new password: ", &password, user.password) == -1)
        goto cleanup;

    // create user with new details
    init_user(&user, user.userId, user.role, user.session_active, user.account_active,
              name, age, address, phone, username, password);

    // Save updated record to the database
    clear_terminal(fd);
    if (record__update(&user, sizeof(User), USER_DB, index) == 0)
        send_message(fd, "\nUser details updated successfully!\n");
    else
        send_message(fd, "\nError updating user details in database.\n");

    send_message(fd, "\n\n\nPress enter to continue...");
    receive_message(fd, &name);

cleanup:
    free(name);
    free(address);
    free(phone);
    free(username);
    free(password);
    free(age);
    return showStartScreen();
}

//============================================================================

// View user details

//============================================================================

// UI - View user details
void user_view_user_details() {}

//============================================================================

// Change Password

//============================================================================

// UI - Change Password
void user_change_password() {}

//============================================================================

// Activate user

//============================================================================

// UI - Activate user
void user_activate_user() {}

//============================================================================

// Deactivate user

//============================================================================

// UI - Deactivate user
void user_deactivate_user() {}

//============================================================================

// Change user role

//============================================================================

// prompt user input
static int __ucur_prompt_user_input_update(int fd, const char *message, char **out)
{
    send_message(fd, message);

    if (receive_message(fd, out) < 0)
        return -1; // communication error

    if (strcmp(*out, "-1") == 0)
        return -1; // cancel update

    return 1;
}

// UI - Change user role
void user_change_user_role()
{

    int fd = clientfd;
    User user;
    char *username = NULL;
    char *role = NULL;

    clear_terminal(fd);
    send_message(fd, "=====================================\n");
    send_message(fd, "   Enter username to update role     \n");
    send_message(fd, "=====================================\n");

    // Get username to update
    if (__uce_prompt_user_input(fd, "Enter username to update details for (-1 to cancel): ", &username) != 0)
        goto cleanup;

    int index = __ucud_find_user_with_username(username, &user);
    if (index == -1)
    {
        send_message(fd, "User not found.\n");
        sleep(2);
        goto cleanup;
    }

    send_message(fd, "\nEnter -1 to cancel the update entirely.\n");
    send_message(fd, "\nChoose the new role for the user:");
    send_message(fd, "1- Admin\n2- Employee\n3- Manager\n");

    // ROLE
    char buf[100];
    snprintf(buf, sizeof(buf), "Current role is: %d", user.role);
    send_message(fd, buf);

    int res = __ucur_prompt_user_input_update(fd, "\nEnter your choice: ", &role);
    if (res == -1)
        goto cleanup;
    if (res == 1)
        user.role = atoi(role);

    clear_terminal(fd);
    // Save updated record to the database
    if (record__update(&user, sizeof(User), USER_DB, index) == 0)
        send_message(fd, "\nUser role updated successfully!\n");
    else
        send_message(fd, "\nError updating role in database.\n");

    send_message(fd, "\n\n\nPress enter to continue...");
    receive_message(fd, &username);

cleanup:
    free(username);
    free(role);
    return showStartScreen();
}

//============================================================================

// Logout

//============================================================================

// Logout
void user_logout()
{
    logged_in_user.session_active = 0;
    record__update(&logged_in_user, sizeof(logged_in_user), USER_DB, logged_in_user_index);

    logged_in_user = (User){0};
    logged_in_user_index = -1;
    return user_login();
}

//============================================================================

// Logout everyone

//============================================================================

// return true for every cmp
static int __ule_true_func(void *a, void *b)
{
    return 1;
}
// update session to 0
static void __ule_update_session(void *val)
{
    User *user = (User *)val;
    user->session_active = 0;
}

// Logout everyone
void user_logout_everyone()
{
    printf("\nLogging out everyone before starting\n");
    User user;
    record__search_and_update_cont(&user, sizeof(user), USER_DB, &__ule_true_func, NULL, &__ule_update_session);
    return;
}
