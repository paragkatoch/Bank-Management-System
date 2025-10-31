// start_screen.c
//============================================================================

// This file contains UI and logic for start screen of every user

//============================================================================

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "config.h"
#include "communication.h"
#include "db/user.h"
#include "db/loan.h"
#include "helper.h"
#include "start_screen.h"
#include "db/account.h"
#include "db/feedback.h"
#include "db/transaction.h"

// UI - show appropriate start sceen to user based on role
void showStartScreen()
{
    if (!logged_in_user.session_active)
    {
        printf("User session not active");
        return;
    }

    switch (logged_in_user.role)
    {
    case ADMIN_ROLE:
        admin_start_screen();
        break;
    case EMPLOYEE_ROLE:
        employee_start_screen();
        break;
    case MANAGER_ROLE:
        manager_start_screen();
        break;
    case CUSTOMER_ROLE:
        customer_start_screen();
        break;

    default:
        break;
    }
}

// UI - start screen for admin user
void admin_start_screen()
{
    int fd = clientfd;

    while (1)
    {
        clear_terminal(fd);
        char *response = NULL;

        send_message(fd, "=====================================");
        send_message(fd, makeString("              Admin Portal (ID: %d)          ", logged_in_user.userId));
        send_message(fd, "=====================================\n\n");

        send_message(fd, "Available options - \n");
        send_message(fd, "1. Create new employee\n");
        send_message(fd, "2. Modify customer/employee details\n");
        send_message(fd, "3. Change user role\n");
        send_message(fd, "0. Exit\n\n");
        send_message(fd, "Choose your option: ");

        if (receive_message(fd, &response) < 0)
        {
            free(response);
            return server_error();
        }

        switch (response[0])
        {
        case '0':
            free(response);
            return user_logout();
        case '1':
            free(response);
            return user_create_employee();
        case '2':
            free(response);
            return user_change_user_details(0);
        case '3':
            free(response);
            return user_change_user_role();
        default:
            free(response);
            send_message(fd, "\n Invalid choice... Try again\n\n");
            sleep(1);
        }
    }
}

void manager_start_screen()
{
    int fd = clientfd;

    while (1)
    {
        clear_terminal(fd);
        char *response = NULL;

        send_message(fd, "=====================================");
        send_message(fd, makeString("            Manager Portal (ID: %d)        ", logged_in_user.userId));
        send_message(fd, "=====================================\n\n");

        send_message(fd, "Available options - \n");
        send_message(fd, "1. Activate user account\n");
        send_message(fd, "2. Deactivate user account\n");
        send_message(fd, "3. View/Assign non-assigned Loans\n");
        send_message(fd, "4. Review customer feedback\n");
        send_message(fd, "0. Exit\n\n");
        send_message(fd, "Choose your option: ");

        if (receive_message(fd, &response) < 0)
        {
            free(response);
            return server_error();
        }

        switch (response[0])
        {
        case '0':
            free(response);
            return user_logout();
        case '1':
            free(response);
            return user_activate_user();
        case '2':
            free(response);
            return user_deactivate_user();
        case '3':
            free(response);
            return loan_view_non_assigned_loans_and_assign();
        case '4':
            free(response);
            return feedback_reviewCreate_action();
        default:
            free(response);
            send_message(fd, "\n Invalid choice... Try again\n\n");
            sleep(1);
        }
    }
}

void employee_start_screen()
{
    int fd = clientfd;

    while (1)
    {
        clear_terminal(fd);
        char *response = NULL;

        send_message(fd, "=====================================");
        send_message(fd, makeString("            Employee Portal (ID: %d)        ", logged_in_user.userId));
        send_message(fd, "=====================================\n\n");

        send_message(fd, "Available options - \n");
        send_message(fd, "1. Create new customer\n");
        send_message(fd, "2. Modify customer details\n");
        send_message(fd, "3. View and process assigned loans\n");
        send_message(fd, "4. View customer transactions\n");
        send_message(fd, "0. Exit\n\n");
        send_message(fd, "Choose your option: ");

        if (receive_message(fd, &response) < 0)
        {
            free(response);
            return server_error();
        }

        switch (response[0])
        {
        case '0':
            free(response);
            return user_logout();
        case '1':
            free(response);
            return user_create_customer();
        case '2':
            free(response);
            return user_change_user_details(1);
        case '3':
            free(response);
            return loan_view_and_process_assigned_loans();
        case '4':
            free(response);
            return transaction_view_transactions(0);
        default:
            free(response);
            send_message(fd, "\n Invalid choice... Try again\n\n");
            sleep(1);
        }
    }
}

void customer_start_screen()
{
    int fd = clientfd;

    while (1)
    {
        clear_terminal(fd);
        char *response = NULL;

        send_message(fd, "=====================================");
        send_message(fd, makeString("            Customer Portal (ID: %d)        ", logged_in_user.userId));
        send_message(fd, "=====================================\n\n");

        send_message(fd, "Available options - \n");
        send_message(fd, "1. View Account Balance\n");
        send_message(fd, "2. Deposit money\n");
        send_message(fd, "3. Withdraw money\n");
        send_message(fd, "4. Transfer money\n");
        send_message(fd, "5. Apply for a loan\n");
        send_message(fd, "6. View loan status\n");
        send_message(fd, "7. Write a feedback\n");
        send_message(fd, "8. View feedback status\n");
        send_message(fd, "9. View transaction history\n");
        send_message(fd, "10. Change Password\n");
        send_message(fd, "0. Exit\n\n");
        send_message(fd, "Choose your option: ");

        if (receive_message(fd, &response) < 0)
        {
            free(response);
            return server_error();
        }

        int choice = atoi(response);

        switch (choice)
        {
        case 0:
            free(response);
            return user_logout();
        case 1:
            free(response);
            return account_view_balance();
        case 2:
            free(response);
            return account_deposit();
        case 3:
            free(response);
            return account_withdraw();
        case 4:
            free(response);
            return account_transfer_funds();
        case 5:
            free(response);
            return loan_create_loan();
        case 6:
            free(response);
            return loan_view_loan_status();
        case 7:
            free(response);
            return feedback_create_feedback();
        case 8:
            free(response);
            return feedback_view_user_feedback();
        case 9:
            free(response);
            return transaction_view_transactions(1);
        case 10:
            free(response);
            return user_change_password();
        default:
            free(response);
            send_message(fd, "\n Invalid choice... Try again\n\n");
            sleep(1);
        }
    }
}