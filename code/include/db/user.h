// user.h
//============================================================================

// This file contains logic and UI related to user

//============================================================================

#ifndef User_H
#define User_H

// =======================================
// User Record Structure
// =======================================
typedef struct
{
    int userId;
    int role;
    int session_active;
    int account_active;

    char name[50];
    char age[5];
    char address[100];
    char phone[20];

    char username[20];
    char password[20];

} User;

// =======================================
// User method Declarations
// =======================================

// init user

void user_login();
void user_create_employee();
void user_create_customer();
void user_change_user_details();
void user_view_user_details();
void user_change_password();
void user_activate_user();
void user_deactivate_user();
void user_change_user_role();

void user_logout();
void user_logout_everyone();
void init_user(User *user, int userId, int role, int s_a, int a_a, char *name, char *age, char *address, char *phone, char *username, char *password);

#endif
