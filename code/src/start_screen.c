#include <unistd.h>
#include <stdio.h>
#include "string.h"
#include "config.h"
#include "communication.h"
#include "db/user.h"
#include "helper.h"
#include "start_screen.h"

/*
 *		This file contains UI and logic for start screen of every user
 */

/**
 *
 */
void showStartScreen()
{
    if (!logged_in_user.session_active)
    {
        printf("User session not active");
        return;
    }

    switch (logged_in_user.role)
    {
    case 0:
        customer_start_screen();
        break;
    case 1:
        admin_start_screen();
        break;
    case 2:
        manager_start_screen();
        break;
    case 3:
        employee_start_screen();
        break;

    default:
        break;
    }
}

/**
 *
 */
void admin_start_screen()
{
    // TODO: add try catch in whole code
    int fd = clientfd;

    while (1)
    {
        clear_terminal(fd);
        char *response = NULL;

        send_message(fd, "=====================================");
        send_message(fd, "              Admin Portal           ");
        send_message(fd, "=====================================\n\n");

        send_message(fd, "Available options - \n");
        send_message(fd, "1. Create new employee\n");
        send_message(fd, "2. Modify employee details\n");
        send_message(fd, "3. Change user role\n");
        send_message(fd, "0. Exit\n\n");
        send_message(fd, "Choose your option: ");

        if (receive_message(fd, &response) < 0)
        {
            return server_error();
        }
        else if (strcmp(response, "1") == 0)
        {
            return user_create_employee();
        }
        else if (strcmp(response, "2") == 0)
        {
            return user_change_user_details();
        }
        else if (strcmp(response, "3") == 0)
        {
            return user_change_user_role();
        }
        else if (strcmp(response, "0") == 0)
        {
            return user_logout();
        }
        else
        {
            send_message(fd, "\n Invalid choice... Try again\n\n");
            sleep(2);
        }
    }
}

void manager_start_screen() {}
void employee_start_screen() {}
void customer_start_screen() {}