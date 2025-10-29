#include "db/loan.h"
#include "config.h"
#include "helper.h"
#include "communication.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "file_operation.h"
#include "start_screen.h"
#include <unistd.h>

// Create and save new user
static int __lcl_prompt_user_input(int fd, const char *message, char **out)
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

// Create and save new loan
void loan_create_loan()
{
    int fd = clientfd;

    Loan newLoan;
    char *loanAmount = NULL;
    int loanId = 1001;

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "     Loan Application Portal        \n");
    send_message(fd, "=====================================\n");

    send_message(fd, "\n Use -1 to cancel \n\n");

    // get updated details
    if (__lcl_prompt_user_input(fd, "\nEnter loan amount: ", &loanAmount) != 0)
        goto cleanup;

    newLoan.loanId = generateUniqueLoanId();
    newLoan.loanAmount = atoi(loanAmount);
    newLoan.loanStatus = LOAN_PROCESSING; // 0 processed, 1 approved, 2 rejected
    newLoan.assignedID = LOAN_NOTASSIGNED;
    newLoan.accountID = logged_in_user.userId;

    clear_terminal(fd);

    // save user to db
    if (record__save(&newLoan, sizeof(Loan), LOAN_DB) != 0)
        send_message(fd, "Oops...something went wrong\n");
    else
        send_message(fd, "Applied for loan successfully.\n");

    send_message(fd, "\n\n\nPress enter to continue...");
    receive_message(fd, &loanAmount);

    // cleanup
cleanup:
    free(loanAmount);
    return showStartScreen();
}

int __lvls_cmp(void *rec, void *ctx)
{
    Loan *tempLoan = (Loan *)rec;
    int userId = *(int *)ctx;

    return tempLoan->accountID == userId ? 1 : 0;
}
// View loan status
void loan_view_loan_status()
{
    int fd = clientfd;
    int accountId = logged_in_user.userId;

    void *loansBits = NULL;
    int count = record__search_cont(&loansBits, sizeof(Loan), LOAN_DB, &__lvls_cmp, &accountId);

    if (count == -1)
        goto failure;

    Loan *loans = (Loan *)loansBits;

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "               My Loans              \n");
    send_message(fd, "=====================================\n");
    send_message(fd, "\n\nLoanId\t\t\tAmount\t\t\tStatus");

    for (int i = 0; i < count; i++)
    {
        int loanId = loans[i].loanId;
        int loanAmount = loans[i].loanAmount;
        int loanStatusInt = loans[i].loanStatus;

        char loanStatus[20];

        switch (loanStatusInt)
        {
        case LOAN_PROCESSED:
            strcpy(loanStatus, "Processed");
            break;
        case LOAN_APPROVED:
            strcpy(loanStatus, "Approved");
            break;
        case LOAN_REJECTED:
            strcpy(loanStatus, "Rejected");
            break;
        default:
            strcpy(loanStatus, "Processing");
            break;
        }

        char buffer[128];
        snprintf(buffer, sizeof(buffer),
                 "\n%d\t\t\t%d\t\t\t%s",
                 loanId, loanAmount, loanStatus);

        send_message(fd, buffer);
    }

    char *tmp = NULL;
    send_message(fd, "\n\n\nPress enter to go back...");
    receive_message(fd, &tmp);
    free(tmp);

failure:
    free(loansBits);
    return showStartScreen();
}

// Assign loan
void loan_assign_loan_to_an_employee() {}

// View all loans
void loan_view_all_loans() {}

// View user loans
void loan_view_user_loans() {}

// View non-assigned loans
void loan_view_non_assigned_loans() {}

// View employee assigned loans
void loan_view_and_process_assigned_loans() {}

// Process loan
void loan_process_loan() {}

// Accept loan
void loan_accept_loan() {}

// Reject loan
void loan_reject_loan() {}

void loan_view_process_non_assigned_loans() {}