#ifndef Loan_H
#define Loan_H

// =======================================
// Loan Record Structure
// =======================================

#define LOAN_PROCESSING -1
#define LOAN_PROCESSED 0
#define LOAN_APPROVED 1
#define LOAN_REJECTED 2

#define LOAN_NOTASSIGNED -1

typedef struct
{
    int loanId;
    int loanAmount;
    int accountID;
    int assignedID;
    int loanStatus;
    // int created_timestamp;
    // int updated_timestamp;
} Loan;

// =======================================
// Loan method Declarations
// =======================================

// Create and save new loan
void loan_create_loan();

// View loan status
void loan_view_loan_status();

// Assign loan
void loan_assign_loan_to_an_employee();

// View all loans
void loan_view_all_loans();

// View user loans
void loan_view_user_loans();

// View non-assigned loans
void loan_view_non_assigned_loans();

// View employee assigned loans
void loan_view_and_process_assigned_loans();

// Process loan
void loan_process_loan();

// Accept loan
void loan_accept_loan();

// Reject loan
void loan_reject_loan();

void loan_view_process_non_assigned_loans();

#endif
