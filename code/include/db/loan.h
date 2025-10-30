// loan.h

//============================================================================

// This file contains logic and UI related to loans

//============================================================================

#ifndef Loan_H
#define Loan_H

// =======================================
// Loan Record Structure
// =======================================

#define LOAN_PROCESSING 0
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

// View employee assigned loans
void loan_view_and_process_assigned_loans();

// View and Assign non-assigned loans
void loan_view_non_assigned_loans_and_assign();

#endif
