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

void init_user(User *user, int userId, int role, int s_a, int a_a, char *name, char *age, char *address, char *phone, char *username, char *password)
{
    memset(user, 0, sizeof(User));

    user->userId = userId;
    user->role = role;
    user->session_active = s_a;
    user->account_active = a_a;

    safe_strncpy(user->name, name, sizeof(user->name));
    safe_strncpy(user->age, name, sizeof(user->age));
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

void __user_login_update_session(void *rec)
{
    User *user = (User *)rec;
    user->session_active = 1;
}

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

    int index = record__search_and_update(&tempUser, sizeof(tempUser), USER_DB, &__user_login_compare_username_password, &username_password, &__user_login_update_session);

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
int prompt_user_input(int fd, const char *message, char **out)
{
    send_message(fd, message);
    if (receive_message(fd, out) < 0)
    {
        send_message(fd, "\n⚠️ Error receiving input.\n");
        return -2;
    }

    if (strcmp(*out, "-1") == 0)
    {
        send_message(fd, "\n❌ Operation cancelled by user.\n");
        return -1;
    }

    return 0;
}

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

    int userId = 2001;      // (you can auto-generate later)
    int role = 2;           // 1 = admin, 2 = employee
    int session_active = 0; // not logged in
    int account_active = 1; // active

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "     Employee Creation Portal        \n");
    send_message(fd, "=====================================\n");

    if (prompt_user_input(fd, "Enter employee name (or -1 to cancel): ", &name) != 0)
        goto cleanup;
    if (prompt_user_input(fd, "Enter employee age (or -1 to cancel): ", &age) != 0)
        goto cleanup;
    if (prompt_user_input(fd, "Enter employee address (or -1 to cancel): ", &address) != 0)
        goto cleanup;
    if (prompt_user_input(fd, "Enter phone number (or -1 to cancel): ", &phone) != 0)
        goto cleanup;
    if (prompt_user_input(fd, "Enter username (or -1 to cancel): ", &username) != 0)
        goto cleanup;
    if (prompt_user_input(fd, "Enter password (or -1 to cancel): ", &password) != 0)
        goto cleanup;

    init_user(&user, userId, role, session_active, account_active,
              name, age, address, phone, username, password);

    record__save(&user, sizeof(User), USER_DB);
    send_message(fd, "\n✅ Employee account created successfully!\n");
    send_message(fd, "Employee details saved to database.\n");

    sleep(5);

cleanup:
    free(name);
    free(age);
    free(address);
    free(phone);
    free(username);
    free(password);
    return showStartScreen();
}

void user_create_customer() {}

// user login compare function for searching user record
int __user_login_compare_username(void *rec, void *ctx)
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
int __user_login_find_user_record_with_username(char *username, User *tempUser)
{
    int index = record__search(tempUser, sizeof(User), USER_DB, __user_login_compare_username, username);
    return index;
}

// Update user details
int prompt_user_input_update(int fd, const char *message, char **out)
{
    send_message(fd, message);

    if (receive_message(fd, out) < 0)
        return -1; // communication error

    if (strcmp(*out, "-1") == 0)
        return -1; // cancel update

    if (strcmp(*out, "0") == 0)
        return 0; // skip field

    return 1; // valid input to store in structure
}

void user_change_user_details()
{
    int fd = clientfd;

    User user;

    char *name = NULL;
    char *address = NULL;
    char *phone = NULL;
    // char *age_str = NULL;
    char *username = NULL;
    char *password = NULL;
    char *age = NULL;

    clear_terminal(fd);

    send_message(fd, "=====================================\n");
    send_message(fd, "        Enter details to update       \n");
    send_message(fd, "=====================================\n");

    // Step 1: Get username to update
    if (prompt_user_input(fd, "Enter username to update details for (-1 to cancel): ", &username) != 0)
        goto cleanup;

    // if (strcmp(username, "-1") == 0)
    //     goto cancel;

    int index = __user_login_find_user_record_with_username(username, &user);
    // __user_login_find_user_record_with_username(username, &user);

    if (index == -1)
    {
        send_message(fd, "❌ User not found.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 2: Update fields one by one
    send_message(fd, "\n👉 Enter 0 to skip a field, or -1 to cancel update entirely.\n");

    // --- Name ---
    int res = prompt_user_input_update(fd, "Enter new name: ", &name);
    if (res == -1)
        goto cleanup;
    if (res == 1)
        safe_strncpy(user.name, name, sizeof(user.name));
    // res==0 auto skip ho jayega

    // --- Age ---
    res = prompt_user_input_update(fd, "Enter new Age: ", &age);
    if (res == -1)
        goto cleanup;
    if (res == 1)
        safe_strncpy(user.age, age, sizeof(user.age));
    // res==0 auto skip ho jayega

    // --- Address ---
    res = prompt_user_input_update(fd, "Enter new address: ", &address);
    if (res == -1)
        goto cleanup;
    if (res == 1)
        safe_strncpy(user.address, address, sizeof(user.address));
    // res==0 auto skip ho jayega

    // --- Phone ---
    res = prompt_user_input_update(fd, "Enter new phone: ", &phone);
    if (res == -1)
        goto cleanup;
    if (res == 1)
        safe_strncpy(user.phone, phone, sizeof(user.phone));
    // res==0 auto skip ho jayega

    // --- Username ---
    res = prompt_user_input_update(fd, "Enter new username: ", &username);
    if (res == -1)
        goto cleanup;
    if (res == 1)
        safe_strncpy(user.username, username, sizeof(user.username));
    // res==0 auto skip ho jayega

    // --- Password ---
    res = prompt_user_input_update(fd, "Enter new password: ", &password);
    if (res == -1)
        goto cleanup;
    if (res == 1)
        safe_strncpy(user.password, password, sizeof(user.password));
    // res==0 auto skip ho jayega

    // ===============================================
    // ✅ Step 3: Save updated record to the database
    // ===============================================
    int update_status = record__update(&user, sizeof(User), USER_DB, index);
    // This is where the user's updated data is written back to the USER_DB file.

    if (update_status == 0)
        send_message(fd, "\n User details updated successfully!\n");
    else
        send_message(fd, "\n Error updating user details in database.\n");

    sleep(2);
    goto cleanup;

cancel:
    send_message(fd, "\n Update cancelled.\n");
    sleep(2);

cleanup:
    free(name);
    free(address);
    free(phone);
    free(username);
    free(password);
    free(age);
    send_message(clientfd, "cleanup");
    // free user
    return showStartScreen();
}

// View user details
void user_view_user_details() {}

// Change Password
void user_change_password() {}

// Activate user
void user_activate_user() {}

// Deactivate user
void user_deactivate_user() {}

// Change user role
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

    // Step 1: Get username to update
    if (prompt_user_input(fd, "Enter username to update details for (-1 to cancel): ", &username) != 0)
        goto cleanup;

    int index = __user_login_find_user_record_with_username(username, &user);

    if (index == -1)
    {
        send_message(fd, " User not found.\n");
        sleep(2);
        goto cleanup;
    }

    send_message(fd, "\n Choose the new role for the user:\n");
    send_message(fd, "   1- Admin\n   2- Employee\n   3- Manager\n");
    send_message(fd, "   (Enter the corresponding number, or 0 to skip, -1 to cancel)\n");

    // ROLE
    int res = prompt_user_input_update(fd, "\n Enter your choice: ", &role);

    if (res == -1)
        goto cleanup;
    if (res == 1)
        user.role = atoi(role);
    // res==0 auto skip ho jayega

    // ===============================================
    // ✅ Step 3: Save updated record to the database
    // ===============================================
    int update_status = record__update(&user, sizeof(User), USER_DB, index);
    // This is where the user's updated data is written back to the USER_DB file.

    if (update_status == 0)
        send_message(fd, "\n✅ User role updated successfully!\n");
    else
        send_message(fd, "\n⚠️ Error updating role in database.\n");

    sleep(2);
    goto cleanup;

cleanup:

    free(username);

    send_message(clientfd, "cleanup");
    // free user
    return showStartScreen();
}

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
