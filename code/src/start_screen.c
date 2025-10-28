#include <unistd.h>
#include <stdlib.h>
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
            return user_change_user_details();
        case '3':
            free(response);
            return user_change_user_role();
        default:
            free(response);
            send_message(fd, "\n Invalid choice... Try again\n\n");
            sleep(2);
        }
    }
}

void manager_start_screen() {}
void employee_start_screen() {}
void customer_start_screen() {}