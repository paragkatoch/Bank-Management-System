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
#include "db/account.h"

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
} username_password;
// function to update session of User
static void __update_user_session(void *rec)
{
    User *user = (User *)rec;
    user->session_active = 1;
}
// function to compare user with username, password and session
static int __compare_user_username_password(void *rec, void *ctx)
{
    User *user = (User *)rec;
    username_password *login = (username_password *)ctx;

    if ((strcmp(user->username, login->username) == 0) &&
        (strcmp(user->password, login->password) == 0) &&
        (user->session_active == 0) &&
        (user->account_active == 1))
    {
        return 1; // match found
    }
    return 0; // no match
}
// function to find user on the basis of username and password
static int __find_user_based_on_username_password(char *username, char *password)
{
    User tempUser;
    username_password usernamePassword;

    safe_strncpy(usernamePassword.username, username, sizeof(usernamePassword.username) - 1);
    safe_strncpy(usernamePassword.password, password, sizeof(usernamePassword.password) - 1);

    int index = record__search_and_update(&tempUser, sizeof(tempUser), USER_DB, &__compare_user_username_password, &usernamePassword, &__update_user_session, RECORD_USE_LOCK);

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

    char *username = NULL;
    char *password = NULL;

    while (attempts > 0)
    {
        clear_terminal(fd);

        send_message(fd, "=====================================");
        send_message(fd, "         Welcome to Login Portal     ");
        send_message(fd, "=====================================\n");

        if (prompt_user_input(fd, "\nEnter username (-1 to exit): ", &username) != 0 ||
            prompt_user_input(fd, "\nEnter password (-1 to exit): ", &password) != 0)
        {
            goto cleanup;
        }

        int login_successful = __find_user_based_on_username_password(username, password);

        if (login_successful)
        {
            clear_terminal(fd);
            send_message(fd, "\nLogin successful!");
            send_message(fd, "Proceeding");

            // Simple timer
            for (int i = 0; i < 3; i++)
            {
                send_message(fd, ".");
                sleep(1);
            }

            goto success;
        }
        else
        {
            attempts--;
            send_message(fd, makeString("\nLogin failed!. Attempts left: %d", attempts));
            sleep(2);
        }
    }

cleanup:
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

    int userId = generateUniqueUserId();
    int role = EMPLOYEE_ROLE;
    int session_active = SESSION_INACTIVE;
    int account_active = ACCOUNT_ACTIVE;

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "     Employee Creation Portal        \n");
    send_message(fd, "=====================================\n");

    send_message(fd, "\n Use -1 to cancel \n\n");

    // get updated details
    if (prompt_user_input(fd, "\nEnter employee name: ", &name) != 0 ||
        prompt_user_input(fd, "\nEnter employee age: ", &age) != 0 ||
        prompt_user_input(fd, "\nEnter employee address: ", &address) != 0 ||
        prompt_user_input(fd, "\nEnter phone number: ", &phone) != 0 ||
        prompt_user_input(fd, "\nEnter username: ", &username) != 0 ||
        prompt_user_input(fd, "\nEnter password: ", &password) != 0)
        goto cleanup;

    // create user with new details
    init_user(&user, userId, role, session_active, account_active,
              name, age, address, phone, username, password);

    clear_terminal(fd);
    send_message(fd, "\nEmployee account created successfully!\n");

    // save user to db
    if (record__save(&user, sizeof(User), USER_DB, RECORD_USE_LOCK) != 0)
        send_message(fd, "Unable to save employee details in database.\n");
    else
        send_message(fd, "Employee details saved to database.\n");

    waitTillEnter(fd);

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
void user_create_customer()
{
    int fd = clientfd;
    User user;
    char *name = NULL;
    char *address = NULL;
    char *phone = NULL;
    char *username = NULL;
    char *password = NULL;
    char *age = NULL;
    char *principalBalance = NULL;

    int userId = generateUniqueUserId(); // auto-generate
    int role = CUSTOMER_ROLE;
    int session_active = SESSION_INACTIVE;
    int account_active = ACCOUNT_ACTIVE;

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "     Customer Creation Portal        \n");
    send_message(fd, "=====================================\n");

    send_message(fd, "\n Use -1 to cancel \n\n");

    // get updated details
    if (prompt_user_input(fd, "\nEnter Customer name: ", &name) != 0 ||
        prompt_user_input(fd, "\nEnter Customer age: ", &age) != 0 ||
        prompt_user_input(fd, "\nEnter Customer address: ", &address) != 0 ||
        prompt_user_input(fd, "\nEnter phone number: ", &phone) != 0 ||
        prompt_user_input(fd, "\nEnter username: ", &username) != 0 ||
        prompt_user_input(fd, "\nEnter password: ", &password) != 0 ||
        prompt_user_input(fd, "\nEnter Account Balance: ", &principalBalance) != 0)
        goto cleanup;

    // create user with new details
    init_user(&user, userId, role, session_active, account_active,
              name, age, address, phone, username, password);

    account_create_account(userId, atoi(principalBalance));

    clear_terminal(fd);

    send_message(fd, "\nCustomer account created successfully!\n");

    // save user to db
    if (record__save(&user, sizeof(User), USER_DB, RECORD_USE_LOCK) != 0)
        send_message(fd, "Unable to save Customer details in database.\n");
    else
        send_message(fd, "Customer details saved to database.\n");

    waitTillEnter(fd);

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

// Change User details

//============================================================================

// user login compare function for searching user record
static int __compare_user_username(void *rec, void *ctx)
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
static int __find_user_based_on_username(char *username, User *tempUser, int lock)
{
    int index = record__search(tempUser, sizeof(User), USER_DB, __compare_user_username, username, lock);
    return index;
}
// prompt user input
static int __prompt_user_input_update(int fd, const char *message, char **out, char *original)
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
void user_change_user_details(int customerOnly)
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
    if (prompt_user_input(fd, "\nEnter username to update details for: ", &username) != 0)
        goto cleanup;

    int index = __find_user_based_on_username(username, &user, RECORD_USE_LOCK);
    if (index == -1)
    {
        send_message(fd, "\nUser not found.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    if (customerOnly == 1 && user.role != CUSTOMER_ROLE)
    {
        send_message(fd, "\nEnter valid customer username");
        waitTillEnter(fd);
        goto cleanup;
    }

    clear_terminal(fd);
    send_message(fd, "\nEnter 0 to skip a field, or -1 to cancel update entirely.\n");

    // Name
    send_message(fd, "\nCurrent name is: ");
    send_message(fd, user.name);
    if (__prompt_user_input_update(fd, "\nEnter new name: ", &name, user.name) == -1)
        goto cleanup;

    // Age
    clear_terminal(fd);
    send_message(fd, "\nCurrent age is: ");
    send_message(fd, user.age);
    if (__prompt_user_input_update(fd, "\nEnter new age: ", &age, user.age) == -1)
        goto cleanup;

    // Address
    clear_terminal(fd);
    send_message(fd, "\nCurrent Address is: ");
    send_message(fd, user.address);
    if (__prompt_user_input_update(fd, "\nEnter new address: ", &address, user.address) == -1)
        goto cleanup;

    // Phone
    clear_terminal(fd);
    send_message(fd, "\nCurrent phone number is: ");
    send_message(fd, user.phone);
    if (__prompt_user_input_update(fd, "\nEnter new phone number: ", &phone, user.phone) == -1)
        goto cleanup;

    // Username
    clear_terminal(fd);
    send_message(fd, "\nCurrent username is: ");
    send_message(fd, user.username);
    if (__prompt_user_input_update(fd, "\nEnter new username: ", &username, user.username) == -1)
        goto cleanup;

    // Password
    clear_terminal(fd);
    send_message(fd, "\nCurrent password is: ");
    send_message(fd, user.password);
    if (__prompt_user_input_update(fd, "\nEnter new password: ", &password, user.password) == -1)
        goto cleanup;

    // create user with new details
    init_user(&user, user.userId, user.role, user.session_active, user.account_active,
              name, age, address, phone, username, password);

    // Save updated record to the database
    clear_terminal(fd);
    if (record__update(&user, sizeof(User), USER_DB, index, RECORD_USE_LOCK) == 0)
        send_message(fd, "\nUser details updated successfully!\n");
    else
        send_message(fd, "\nError updating user details in database.\n");

    waitTillEnter(fd);

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
void user_change_password()
{
    int fd = clientfd;
    User *user = &logged_in_user;
    char *password = NULL;

    clear_terminal(fd);
    send_message(fd, "=====================================\n");
    send_message(fd, "            Update Password          \n");
    send_message(fd, "=====================================\n");

    // Password
    clear_terminal(fd);
    send_message(fd, makeString("\nCurrent password is: %s", user->password));
    if (prompt_user_input(fd, "\nEnter new password: ", &password) == -1)
        goto cleanup;

    safe_strncpy(user->password, password, 20);

    if (record__update(user, sizeof(User), USER_DB, logged_in_user_index, RECORD_USE_LOCK) != 0)
        send_message(fd, "\nUnable to save new password details in database.\n");
    else
        send_message(fd, "\nNew password details saved to database.\n");

    waitTillEnter(fd);

cleanup:
    free(password);
    return showStartScreen();
}

//============================================================================

// Activate user

//============================================================================

// compare by userId
static int __compare_user_userId(void *rec, void *ctx)
{
    User *user = (User *)rec;
    int *id = (int *)ctx;
    return user->userId == *id;
}

int find_user_based_on_userId(User *user, int userId, int lock)
{
    return record__search(user, sizeof(User), USER_DB, __compare_user_userId, &userId, lock);
}

// UI - Activate user
void user_activate_user()
{

    int fd = clientfd;
    User user;
    char *userId_str = NULL;

    clear_terminal(fd);
    send_message(fd, "=====================================\n");
    send_message(fd, "          Activate User Account      \n");
    send_message(fd, "=====================================\n");

    // Get userId input
    if (prompt_user_input(fd, "\nEnter User ID to activate (-1 to cancel): ", &userId_str) != 0)
        goto cleanup;

    int userId = atoi(userId_str);
    int index = find_user_based_on_userId(&user, userId, RECORD_USE_LOCK);

    if (index == -1)
    {
        send_message(fd, "\nUser not found.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // TODO: new if condition
    // check if already active
    if (user.account_active == 1 || user.role == ADMIN_ROLE)
    {
        send_message(fd, "\nThis user account is already active.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    user.account_active = 1; // activate user

    if (record__update(&user, sizeof(User), USER_DB, index, RECORD_USE_LOCK) == 0)
        send_message(fd, "\nUser account activated successfully!\n");
    else
        send_message(fd, "\nError activating user account.\n");

    waitTillEnter(fd);

cleanup:
    free(userId_str);
    return showStartScreen();
}

//============================================================================

// Deactivate user

//============================================================================

// UI - Deactivate user
void user_deactivate_user()
{

    int fd = clientfd;
    User user;
    char *userId_str = NULL;

    clear_terminal(fd);
    send_message(fd, "=====================================\n");
    send_message(fd, "         Deactivate User Account     \n");
    send_message(fd, "=====================================\n");

    // Get userId input
    if (prompt_user_input(fd, "\nEnter User ID to deactivate (-1 to cancel): ", &userId_str) != 0)
        goto cleanup;

    int userId = atoi(userId_str);
    int index = find_user_based_on_userId(&user, userId, RECORD_USE_LOCK);

    if (index == -1)
    {
        send_message(fd, "\nUser not found.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // check if already inactive
    if (user.account_active == 0 || user.role == ADMIN_ROLE)
    {
        send_message(fd, "\nThis user account is already deactivated.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    user.account_active = 0; // deactivate

    if (record__update(&user, sizeof(User), USER_DB, index, RECORD_USE_LOCK) == 0)
        send_message(fd, "\nUser account deactivated successfully!\n");
    else
        send_message(fd, "\nError deactivating user account.\n");

    waitTillEnter(fd);

cleanup:
    free(userId_str);
    return showStartScreen();
}

//============================================================================

// Change user role

//============================================================================

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
    if (prompt_user_input(fd, "Enter username to update details for (-1 to cancel): ", &username) != 0)
        goto cleanup;

    int index = __find_user_based_on_username(username, &user, RECORD_USE_LOCK);
    if (index == -1)
    {
        send_message(fd, "User not found.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    send_message(fd, "\nEnter -1 to cancel the update entirely.\n");
    send_message(fd, "\nChoose the new role for the user:");
    send_message(fd, "1- Admin\n2- Employee\n3- Manager\n");

    // ROLE
    send_message(fd, makeString("Current role is: %d", user.role));

    int res = prompt_user_input(fd, "\nEnter your choice: ", &role);
    if (res == -1)
        goto cleanup;
    // TODO res == 0
    if (res == 0)
        user.role = atoi(role);

    clear_terminal(fd);
    // Save updated record to the database
    if (record__update(&user, sizeof(User), USER_DB, index, RECORD_USE_LOCK) == 0)
        send_message(fd, "\nUser role updated successfully!\n");
    else
        send_message(fd, "\nError updating role in database.\n");

    waitTillEnter(fd);

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
    record__update(&logged_in_user, sizeof(logged_in_user), USER_DB, logged_in_user_index, RECORD_USE_LOCK);

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
    record__search_and_update_cont(&user, sizeof(user), USER_DB, &__ule_true_func, NULL, &__ule_update_session, RECORD_USE_LOCK);
    return;
}
