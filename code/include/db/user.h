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
    int age;
    char address[100];
    char phone[20];

    char username[20];
    char password[20];

} User;

// =======================================
// User method Declarations
// =======================================

// init user
void init_user(User *user, int userId, int role, int s_a, int a_a, char *name, int age, char *address, char *phone, char *username, char *password);

// Login
void user_login();

// Create and save new user
void user_create_employee();

void user_create_customer();

// Update user details
void user_change_user_details();

// View user details
void user_view_user_details();

// Change Password
void user_change_password();

// Activate user
void user_activate_user();

// Deactivate user
void user_deactivate_user();

// Change user role
void user_change_user_role();

// Logout
void user_logout();

#endif
