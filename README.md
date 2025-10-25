# CS-513 System Software – Mini Project

## Title: Design and Development of Banking Management System

The Banking Management System aims to simulate the core functionalities of a bank, including customer account management, transactions, loan processing, and administrative oversight. The system features secure login mechanisms and role-based access for customers, employees, managers, and administrators. The focus is on ensuring data consistency, security, and correct handling of race conditions using file management, locking mechanisms, and process synchronization.

## Roles:

### 1. Customer:

Customers log in using their credentials and have access to banking operations such as viewing account balances, depositing or withdrawing money, transferring funds, and applying for loans. The system ensures proper locking mechanisms to avoid race conditions when performing concurrent operations.

- Login System (one session per user)
- View Account Balance
- Deposit Money
- Withdraw Money
- Transfer Funds
- Apply for a Loan
- Change Password
- Adding Feedback
- View Transaction History
- Logout
- Exit

### 2. Bank Employee:

Bank employees manage customer accounts, including opening, modifying, or closing accounts, as well as processing loans.

- Login System (one session per user)
- Add New Customer
- Modify Customer Details
- Process Loan Applications
- Approve/Reject Loans
- View Assigned Loan Applications
- View Customer Transactions( Passbook Kind of feature)
- Change Password
- Logout
- Exit

### 3. Manager:

- Login System (one session per user)
- Activate/Deactivate Customer Accounts
- Assign Loan Application Processes to Employees
- Review Customer Feedback
- Change Password
- Logout
- Exit

### 4. Administrator: The Administrator has the highest level of access and manages both customer and employee accounts.

- Login System (one session per user)
- Add New Bank Employee
- Modify Customer/Employee Details
- Manage User Roles
- Change Password
- Logout
- Exit

## Technical Requirements:

#### Socket Programming:

The server maintains the database and serves multiple clients concurrently. Clients can connect to the server and access their specific account details.

#### System Calls:

Use system calls instead of library functions wherever possible, including process management, file management, file locking, semaphores, multithreading, and inter-process communication mechanisms.

## Evaluation Criteria:

1. Blueprint/Class diagram
2. Working code for each module and its functionalities
3. Concurrency, synchronization, and ACID properties
