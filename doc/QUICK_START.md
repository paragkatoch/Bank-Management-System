# Quick Start Guide

[Main README](../README.md) | [Flowcharts](flowchart.md) | [Class Diagram](class_diagram.puml)

## Setup

```bash
chmod +x make && ./make
./make server                # Terminal 1
./make client                # Terminal 2
```

Login: `admin` / `admin123`

## Common Operations

**Administrator:** Add employees/managers, modify users, change roles

**Employee:** Add customers, process loans, view transactions

**Manager:** Assign loans, activate/deactivate accounts, review feedback

**Customer:** Deposit, withdraw, transfer, apply for loans, submit feedback

## Default Credentials

| Username | Password   | Role          |
| -------- | ---------- | ------------- |
| admin    | admin123   | Administrator |
| manager  | manager123 | Manager       |
| devanshi | 12345      | Employee      |
| diksha   | 12345      | Employee      |
| parag    | 12345      | Employee      |

## Quick Reference

**Roles:** 1=Admin, 2=Employee, 3=Manager, 4=Customer

**Loan Status:** 0=Processing, 1=Approved, 2=Rejected

**Commands:**

```bash
./make                          # Compile
./make server                        # Start server
./make client                        # Connect client
```
