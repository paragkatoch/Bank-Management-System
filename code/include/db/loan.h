#ifndef Loan_H
#define Loan_H

// =======================================
// Loan Record Structure
// =======================================
typedef struct {
    int loanId;
    int loanAmount;
    int accountID;
    int assignedID;
    bool loanStatus;
    int created_timestamp;
    int updated_timestamp;
} Loan;

// =======================================
// Loan method Declarations
// =======================================

// Create and save new loan
void loan_create_loan();

// Assign loan
void loan_assign_loan_to_an_employee();

// View all loans
void loan_view_all_loans();

// View user loans
void loan_view_user_loans();

// View non-assigned loans
void loan_view_non_assigned_loans();

// View employee assigned loans
void loan_view_employee_assigned_loans();

// Process loan
void loan_process_loan();

// Accept loan
void loan_accept_loan();

// Reject loan
void loan_reject_loan();

#endif
