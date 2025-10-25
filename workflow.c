/**


    User Flow

        1. User Related
            1. Change password
            2. Login
            3. Logout

        2. Flows

            1. Customer flow

                1. Login as a customer (User) ✅
                    1. If deactivated - return "Account is deactivated. Contact your bank for more information"

                2. Functions -

                    1. View Balance (Account) ✅
                        1. Search for account using userId and return the balance
                    
                    2. Deposit/Withdraw (Account) ✅
                        1. Search for the account userId and update the balance
                    
                    3. Transfer funds (Account) ✅
                        1. Search for 2 accounts using 2 userIds
                        2. Update balance of both accounts

                    4. Apply loan (Loan) ✅
                        1. Create a loan entry in loan db
                    
                    5. View loan status (Loan) ✅
                        1. Search for loans in loan db using userId

                    6. Feedback Related (Feedback) ✅ // same as loan
                        1. Add feedback
                        2. View feedback status

                    7. View transaction history (Transaction) ✅
                        1. Search for transactions in transaction using userId

            2. Employee Flow

                1. Login as a employee (User) ✅

                2. Functions -

                    1. Create new customer (Account, User) // handled by user ✅
                        1. Create new entry in account and user

                    2. Modify customer details (User) ✅
                        1. Update only name, age and address in user table for user with role "customer"

                    3. Process Load Application (Loan) ✅
                        1. Mark assigned loans as processed
                        2. If account deactivated - return "Failed. User Account is deactivated"

                    4. Approve/Reject loans (Loan) ✅
                        1. Mark assigned loans as approved or rejected
                        2. If account deactivated - return "Failed. User Account is deactivated"

                    5. View Assigned loans (Loan) ✅
                        1. Filter loans on the basis on "assignedTo = employeeId"

                    6. View Customer Transactions (Transaction) ✅
                        1. Filter transactions on the basis of accountId(userId)

            3. Manager Flow

                    1. Login as a manager (User) ✅

                    2. Functions -

                        1. Activate/deactivate customer accounts (Account) ✅
                            1. Filter account on the basis on accountId
                            2. Set active = false/true

                        2. View non_assigned loans (Loan) ✅

                        3. Assign loan application to empyloyees (Loan) ✅
                            1. changed assignedTo to employeeId (userId)

                        4. Review Customer Feedback (feedback) ✅
                            1. add action string and change reviewStatus to true
                        
            4. Admin Flow

                    1. Login as an Admin

                    2. Functions - 

                        1. Add new bank employee (User) ✅
                            1. create new entry in user

                        2. Modify customer/employee details (User) ✅
                            1. Update record with userId = userId

                        3. Manage User Roles (User) ✅
                            1. just change the role







 */