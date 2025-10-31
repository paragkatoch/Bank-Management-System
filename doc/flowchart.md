# Bank Management System - Flowchart & System Flow

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    BANK MANAGEMENT SYSTEM                    │
│                   (Client-Server Architecture)               │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │         TCP/IP Socket (Port 8080)        │
        └─────────────────────────────────────────┘
                              │
                ┌─────────────┴─────────────┐
                │                           │
                ▼                           ▼
        ┌──────────────┐          ┌──────────────┐
        │    Server    │          │   Client(s)  │
        │  (Fork-based │          │  (Multiple)  │
        │  Concurrent) │          │              │
        └──────────────┘          └──────────────┘
                │
                ▼
        ┌──────────────┐
        │  File-based  │
        │   Database   │
        │  (w/ Locks)  │
        └──────────────┘
```

## 1. Main System Flow

```
START
  │
  ├─► Server Initialization
  │     ├─► Create Socket (AF_INET, SOCK_STREAM)
  │     ├─► Bind to Port 8080
  │     ├─► Listen for connections
  │     └─► Initialize Database Files
  │
  ├─► Client Connection
  │     ├─► Accept Connection
  │     ├─► Fork New Process
  │     └─► Handle Client in Child Process
  │
  ├─► User Authentication
  │     ├─► Display Login Screen
  │     ├─► Validate Credentials
  │     ├─► Check Session Status (One session per user)
  │     └─► Determine User Role
  │
  ├─► Role-Based Menu
  │     ├─► Customer Menu
  │     ├─► Employee Menu
  │     ├─► Manager Menu
  │     └─► Administrator Menu
  │
  ├─► Execute Operations
  │     ├─► File Locking (fcntl)
  │     ├─► Read/Write Data
  │     ├─► Update Records
  │     └─► Release Locks
  │
  └─► Logout & Cleanup
        ├─► Mark Session Inactive
        ├─► Close Connection
        └─► Exit Child Process
```

## 2. Authentication Flow

```
Client Connects → Display Login → Enter Credentials
                                         │
                                         ▼
                                  Validate User
                                         │
                    ┌────────────────────┼────────────────────┐
                    │                    │                    │
                 INVALID              VALID               INACTIVE
                    │                    │                    │
                    ▼                    ▼                    ▼
              Retry Login      Check Session Status    Account Disabled
                                         │
                    ┌────────────────────┼────────────────────┐
                    │                                         │
              SESSION_ACTIVE                           SESSION_INACTIVE
                    │                                         │
                    ▼                                         ▼
          Already Logged In                         Mark Session Active
              (Deny)                                          │
                                                              ▼
                                                    Load Role-Based Menu
```

## 3. Customer Operations

### Customer Menu
1. View Account Balance
2. Deposit Money
3. Withdraw Money
4. Transfer Funds
5. Apply for Loan
6. View Loan Status
7. Change Password
8. Add Feedback
9. View Transaction History
10. Logout
11. Exit

### Deposit/Withdraw Flow
```
Request Operation
      │
      ▼
Lock Account File (Write Lock)
      │
      ▼
Read Current Balance
      │
      ▼
Validate Operation (Sufficient funds for withdraw)
      │
      ▼
Update Balance
      │
      ▼
Create Transaction Record
      │
      ▼
Write to Files
      │
      ▼
Release Lock
      │
      ▼
Confirm to User
```

### Fund Transfer Flow
```
Request Transfer (From A to B)
      │
      ▼
Lock Account A (Write Lock)
      │
      ▼
Lock Account B (Write Lock)
      │
      ▼
Validate Accounts & Balance
      │
      ▼
Deduct from Account A
      │
      ▼
Add to Account B
      │
      ▼
Create Transaction Record
      │
      ▼
Write All Changes
      │
      ▼
Release Both Locks
      │
      ▼
Confirm Success
```

## 4. Employee Operations

### Employee Menu
1. Add New Customer
2. Modify Customer Details
3. View Assigned Loan Applications
4. Approve/Reject Loans
5. View Customer Transactions
6. Change Password
7. Logout
8. Exit

### Add Customer Flow
```
Get Customer Details
      │
      ▼
Lock User File (Write Lock)
      │
      ▼
Lock Account File (Write Lock)
      │
      ▼
Create User Record
  ├─ userId (auto-increment)
  ├─ role = CUSTOMER_ROLE
  ├─ session_active = INACTIVE
  ├─ account_active = ACTIVE
  └─ Personal Details
      │
      ▼
Create Account Record
  ├─ accountId = userId
  └─ accountBalance = 0
      │
      ▼
Write Records
      │
      ▼
Release Locks
      │
      ▼
Confirm Success
```

### Loan Processing Flow
```
View Assigned Loans (assignedID = employeeId)
      │
      ▼
Select Loan
      │
      ▼
Lock Loan File (Write Lock)
      │
      ▼
Choose Action: Approve/Reject
      │
      ├─► Approve
      │     ├─► Lock Account File
      │     ├─► Credit Loan Amount
      │     ├─► Update loanStatus = APPROVED
      │     └─► Release Locks
      │
      └─► Reject
            ├─► Update loanStatus = REJECTED
            └─► Release Lock
```

## 5. Manager Operations

### Manager Menu
1. Activate Customer Account
2. Deactivate Customer Account
3. Assign Loan Applications to Employees
4. Review Customer Feedback
5. Change Password
6. Logout
7. Exit

### Loan Assignment Flow
```
View Unassigned Loans (assignedID = -1)
      │
      ▼
Lock Loan File (Write Lock)
      │
      ▼
Select Loan
      │
      ▼
Enter Employee ID
      │
      ▼
Update assignedID = employeeId
      │
      ▼
Write to File
      │
      ▼
Release Lock
      │
      ▼
Confirm Assignment
```

### Feedback Review Flow
```
View Non-Reviewed Feedback
      │
      ▼
Lock Feedback File (Write Lock)
      │
      ▼
Select Feedback
      │
      ▼
Add Action/Response
      │
      ▼
Update reviewStatus = REVIEWED
      │
      ▼
Write to File
      │
      ▼
Release Lock
      │
      ▼
Confirm Review
```

## 6. Administrator Operations

### Administrator Menu
1. Add New Bank Employee
2. Modify Customer/Employee Details
3. Manage User Roles
4. Change Password
5. Logout
6. Exit

### Add Employee Flow
```
Get Employee Details
      │
      ▼
Lock User File (Write Lock)
      │
      ▼
Create User Record
  ├─ userId (auto-increment)
  ├─ role = EMPLOYEE_ROLE or MANAGER_ROLE
  ├─ session_active = INACTIVE
  ├─ account_active = ACTIVE
  └─ Personal Details
      │
      ▼
Write Record
      │
      ▼
Release Lock
      │
      ▼
Confirm Success
```

## 7. File Locking Mechanism

```
Operation Request
      │
      ▼
Open File (O_RDWR)
      │
      ▼
Determine Lock Type
      │
      ├─► Read Operation → F_RDLCK (Shared Lock)
      └─► Write Operation → F_WRLCK (Exclusive Lock)
      │
      ▼
fcntl(fd, F_SETLKW, lock) - Blocking Wait
      │
      ▼
Lock Acquired
      │
      ▼
Perform File Operation
      │
      ▼
fcntl(fd, F_SETLK, F_UNLCK) - Release Lock
      │
      ▼
Close File
```

## 8. ACID Properties Implementation

### Atomicity
- All operations within a transaction complete or none do
- Use file locking to ensure atomic updates

### Consistency
- Validate all inputs before processing
- Check account balances before withdrawals
- Verify user roles before operations

### Isolation
- File locks prevent concurrent modifications
- Read locks for queries, write locks for updates
- Multiple read locks allowed, exclusive write locks

### Durability
- All changes written to disk immediately
- Binary file format ensures data persistence
- Transaction logs maintain operation history

## 9. Database Structure

### Files
- `user.db` - User records (User struct)
- `account.db` - Account records (Account struct)
- `loan.db` - Loan records (Loan struct)
- `transaction.db` - Transaction records (Transaction struct)
- `feedback.db` - Feedback records (Feedback struct)

### User Record
```c
typedef struct {
    int userId;
    int role;                    // 1=Admin, 2=Employee, 3=Manager, 4=Customer
    int session_active;          // 1=Active, 0=Inactive
    int account_active;          // 1=Active, 0=Inactive
    char name[50];
    char age[5];
    char address[100];
    char phone[20];
    char username[20];
    char password[20];
} User;
```

### Account Record
```c
typedef struct {
    int accountId;
    int accountBalance;
} Account;
```

### Loan Record
```c
typedef struct {
    int loanId;
    int loanAmount;
    int accountID;
    int assignedID;              // -1=Not assigned, else employeeId
    int loanStatus;              // 0=Processing, 1=Approved, 2=Rejected
} Loan;
```

### Transaction Record
```c
typedef struct {
    int transactionId;
    int previousAmount;
    int transactionAmount;
    int lastAmount;
    int from_uid;
    int to_uid;
} Transaction;
```

### Feedback Record
```c
typedef struct {
    int feedbackId;
    int userId;
    char feedback[1000];
    char action[1000];
    int reviewStatus;            // 0=Not Reviewed, 1=Reviewed
} Feedback;
```

## 10. Concurrency Scenarios

### Scenario 1: Multiple Readers
- Multiple customers can view balance simultaneously
- Shared read locks (F_RDLCK) allow concurrent access
- No blocking between readers

### Scenario 2: Reader-Writer Conflict
- Customer A viewing balance (read lock)
- Customer B depositing money (write lock)
- Writer waits for reader to finish
- Exclusive access granted to writer

### Scenario 3: Writer-Writer Conflict
- Customer A withdrawing (write lock)
- Customer B depositing (write lock)
- Second writer waits for first to complete
- Sequential execution enforced

### Scenario 4: Fund Transfer (Multiple Locks)
- Lock both accounts in order
- Prevents deadlock by consistent ordering
- Atomic operation across two accounts

## 11. Session Management

```
Login Attempt
      │
      ▼
Lock User File (Read)
      │
      ▼
Check session_active
      │
      ├─► ACTIVE → Deny Login
      └─► INACTIVE → Continue
            │
            ▼
      Upgrade to Write Lock
            │
            ▼
      Set session_active = ACTIVE
            │
            ▼
      Release Lock
            │
            ▼
      Grant Access

Logout
      │
      ▼
Lock User File (Write)
      │
      ▼
Set session_active = INACTIVE
      │
      ▼
Release Lock
```

## 12. Error Handling

- **File Open Error**: Log error, notify client, return to menu
- **Lock Timeout**: Retry with timeout, notify if failed
- **Insufficient Funds**: Release locks, send error message
- **Invalid Input**: Validate, request re-entry
- **User Not Found**: Send error message
- **Connection Lost**: Cleanup resources, release locks, mark session inactive

## 13. Communication Protocol

### Message Format
- Client-Server communication via TCP sockets
- `send_message(fd, msg)` - Send string message
- `receive_message(fd, buffer)` - Receive string message
- Blocking I/O for synchronous operations

### Client-Server Interaction
```
Client                          Server
  │                               │
  ├──── Connect ─────────────────►│
  │                               │
  │◄──── Login Prompt ────────────┤
  │                               │
  ├──── Credentials ─────────────►│
  │                               │
  │◄──── Menu Options ────────────┤
  │                               │
  ├──── Operation Request ───────►│
  │                               │
  │◄──── Result/Confirmation ─────┤
  │                               │
  ├──── Logout ──────────────────►│
  │                               │
  │◄──── Disconnect ──────────────┤
```

## 14. System Initialization

```
Server Start
      │
      ▼
Check Database Files
      │
      ├─► Exist → Load Existing Data
      │
      └─► Don't Exist
            │
            ▼
      Create Database Directory
            │
            ▼
      Create Default Admin
        ├─ userId: 1
        ├─ role: ADMIN_ROLE
        ├─ username: "admin"
        └─ password: "admin123"
            │
            ▼
      Initialize Empty Files
        ├─ user.db
        ├─ account.db
        ├─ loan.db
        ├─ transaction.db
        └─ feedback.db
```
