# Banking Management System - Class Diagram

## Overview
This class diagram represents the structure of the Banking Management System, a client-server application built in C.

## How to View the Diagram

### Option 1: Online PlantUML Viewer
1. Copy the contents of `class_diagram.puml`
2. Visit: https://www.plantuml.com/plantuml/uml/
3. Paste the content and view the diagram

### Option 2: VS Code Extension
1. Install the "PlantUML" extension in VS Code
2. Open `class_diagram.puml`
3. Press `Alt+D` (or `Option+D` on Mac) to preview

### Option 3: Command Line (requires PlantUML installed)
```bash
plantuml class_diagram.puml
```

## System Architecture

### Entity Classes (Data Models)
These represent the core business entities stored in the database:

1. **User**
   - Represents system users (Admin, Manager, Employee, Customer)
   - Manages authentication, authorization, and user profile
   - Roles: Admin (1), Employee (2), Manager (3), Customer (4)

2. **Account**
   - Represents bank accounts
   - Manages balance, deposits, withdrawals, and transfers
   - One-to-one relationship with User

3. **Transaction**
   - Records all financial transactions
   - Tracks from/to users, amounts, and balance changes
   - Created by Account operations

4. **Loan**
   - Manages loan applications
   - Tracks loan amount, status, and assigned employee
   - Status: Processing (0), Approved (1), Rejected (2)

5. **Feedback**
   - Customer feedback and complaints
   - Reviewed by managers
   - Status: Not Reviewed (0), Reviewed (1)

### Utility/Service Classes

1. **FileOperation**
   - Provides CRUD operations for all entities
   - Implements file locking for concurrent access control
   - Generic record management with search, update, delete

2. **Communication**
   - Handles client-server message passing
   - Send/receive operations over sockets

3. **Helper**
   - Utility functions for ID generation
   - Terminal operations, string handling
   - User input prompting

4. **StartScreen**
   - UI boundary layer
   - Role-based menu displays
   - Different screens for Admin, Manager, Employee, Customer

5. **Init**
   - System initialization controller
   - Sets up database files and configuration

6. **Config**
   - Global configuration and constants
   - Database file paths
   - Shared state (logged-in user, socket descriptors)

## Key Relationships

- **User ↔ Account**: One-to-one relationship (each user has one account)
- **User → Transaction**: One-to-many (user creates multiple transactions)
- **User → Loan**: One-to-many (user can apply for multiple loans)
- **User → Feedback**: One-to-many (user can submit multiple feedbacks)
- **Loan → User**: Many-to-one (loans assigned to employees)
- **All Entities → FileOperation**: Dependency for data persistence
- **System → Communication**: Used for client-server messaging
- **System → Helper**: Used for utility functions

## Design Patterns

1. **Data Access Layer**: FileOperation provides a generic interface for all CRUD operations
2. **Client-Server Architecture**: Communication layer abstracts network operations
3. **Role-Based Access Control**: User roles determine available operations
4. **File-Based Database**: Each entity has a corresponding .dat file

## File Structure
```
db/
  ├── user.dat          # User records
  ├── account.dat       # Account records
  ├── loan.dat          # Loan records
  ├── transaction.dat   # Transaction records
  └── feedback.dat      # Feedback records
```

## Notes
- This is a C-based system, not object-oriented, so "classes" represent logical groupings of data structures and functions
- The system uses file locking to handle concurrent access in a multi-client environment
- Each entity module contains both data structures (structs) and related functions
