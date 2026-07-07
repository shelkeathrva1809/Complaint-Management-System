# 🏛️ Smart Public Complaint Management System
## Product Requirements Document (PRD)

---

## 1. Product Overview

**Product Name:** Smart Public Complaint Management System (SPCMS)  
**Type:** C++ Console Application with Rich UI + Real Email Integration  
**Target:** OOP Course Project (C++)  
**Team:** Solo  

### 1.1 One-Line Pitch
> A government-grade complaint management system with real email forwarding, photo evidence, automatic escalation, and a rich color-coded console dashboard.

### 1.2 What Makes This Stand Out
1. **Actually sends real emails** via Gmail SMTP (not just simulation)
2. **Rich console dashboard** with colors, box-drawing, progress bars, live stats
3. **Covers every single OOP syllabus topic** (see mapping below)
4. **Professional UX** — teachers see a polished product, not raw `cout` statements

---

## 2. System Architecture

```
┌──────────────────────────────────────────────────────────┐
│                    main.cpp (Entry Point)                 │
│              Rich Console UI / Dashboard                 │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────┐  │
│  │ Complaint    │  │ Officer      │  │ Citizen        │  │
│  │ System       │  │ Management   │  │ Portal         │  │
│  │ (Controller) │  │              │  │                │  │
│  └──────┬───────┘  └──────┬───────┘  └───────┬────────┘  │
│         │                 │                   │          │
│  ┌──────┴─────────────────┴───────────────────┴────────┐ │
│  │              Core Business Logic                     │ │
│  │  Complaint* (abstract) → Road/Water/Electricity      │ │
│  │  EscalationManager (friend class)                    │ │
│  │  Logger<T> (class template)                          │ │
│  └──────────────────────┬──────────────────────────────┘ │
│                         │                                │
│  ┌──────────────────────┴──────────────────────────────┐ │
│  │              Infrastructure Layer                    │ │
│  │  EmailService (REAL Gmail SMTP via PowerShell)       │ │
│  │  ConsoleUI (colors, boxes, animations)               │ │
│  │  FileManager (data persistence)                      │ │
│  └─────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────┘
```

---

## 3. Functional Requirements

### FR-01: Citizen Complaint Registration
- Citizen enters: Name, Contact, Email, Complaint Type, Description, Severity, Photo Path
- System auto-generates unique Complaint ID (static counter)
- System auto-assigns officer based on department mapping
- System sends **real email** to assigned officer with complaint details
- Confirmation displayed with complaint ID and assigned officer info

### FR-02: Complaint Types (Polymorphism)
- **Road Complaint** → Public Works Department (PWD)
- **Water Complaint** → Water Supply Department
- **Electricity Complaint** → Electricity Board
- Each type overrides `getDepartment()` (pure virtual / runtime polymorphism)
- Each type has department-specific fields (e.g., Road: location/ward, Water: pipeline ID)

### FR-03: Officer Management
- Pre-loaded officer hierarchy per department:
  ```
  Level 1: Field Officer
  Level 2: Supervisor / Department Head
  Level 3: Municipal Commissioner
  ```
- Each officer has: ID, Name, Official Email (real), Department, Level
- Officers can view complaints assigned to them
- Officers can update complaint status

### FR-04: Real Email Forwarding ⚡
- **Method:** Use `system()` call to PowerShell `Send-MailMessage` OR direct SMTP via curl
- **Trigger:** On complaint registration, status update, and escalation
- Email contains: Complaint ID, Citizen info, Description, Severity, Photo path, Timestamp
- **Fallback:** If email fails (no internet), log it and mark as "Email Pending"
- Email log maintained with timestamp, recipient, subject, status

### FR-05: Auto-Escalation Engine
- Background check (triggered from menu or auto on each action):
  - Emergency complaints: escalate after simulated **30 seconds** (represents 12 hours)
  - Normal complaints: escalate after simulated **60 seconds** (represents 24 hours)
  - Low complaints: escalate after simulated **120 seconds** (represents 48 hours)
- On escalation:
  - Reassign to next-level officer (`level + 1`)
  - Send email to higher authority
  - Log escalation event
  - Notify citizen (simulated SMS + shown on screen)
- If Commissioner (Level 3) reached and still unresolved → mark as "Critical - Commissioner Notified"

### FR-06: Dashboard (THE SHOWSTOPPER 🎯)
- **Live Statistics Panel:**
  - Total complaints registered
  - Complaints by status (Pending / In Progress / Resolved / Escalated)
  - Complaints by department (Road / Water / Electricity)
  - Complaints by severity (Emergency / Normal / Low)
  - Average resolution time
- **Visual Elements:**
  - Color-coded status bars (🟢 Green = Resolved, 🟡 Yellow = In Progress, 🔴 Red = Pending/Escalated)
  - ASCII bar charts for department-wise distribution
  - Recent activity feed (last 5 actions)
  - Officer performance summary
- **This is what teachers see first — it must look stunning**

### FR-07: Search & Filter
- Search by Complaint ID (with exception handling for not-found)
- Filter by status, department, severity
- Display results in formatted colored tables

### FR-08: Citizen Feedback
- After complaint is resolved, citizen can rate (1-5 stars)
- Feedback stored and shown in officer performance section
- Uses function template for rating display

### FR-09: Data Persistence (File I/O)
- Save complaints to `complaints.dat` (text/CSV format)
- Save officers to `officers.dat`
- Save email logs to `email_logs.dat`
- Load data on startup
- Auto-save after each operation

### FR-10: Email Logs (Audit Trail)
- Every email sent/attempted is logged:
  ```
  [2026-03-03 14:30:22] TO: officer@pwd.gov.in | SUBJECT: Complaint #105 - Road | STATUS: Sent
  ```
- Viewable from dashboard
- Uses `Logger<T>` class template

---

## 4. Non-Functional Requirements

### NFR-01: Console UI Quality
- Must work on Windows 10/11 terminal
- Must support colored output (ANSI escape codes / Windows Console API)
- Must have consistent visual theme throughout
- Screen clears between views for clean navigation
- Input validation on every prompt

### NFR-02: Performance
- Complaint registration < 2 seconds (excluding email send time)
- Dashboard render < 1 second
- Search by ID: O(n) scan is acceptable for < 500 complaints

### NFR-03: Error Handling
- No crashes on invalid input (all inputs validated)
- Custom exceptions for business logic errors
- Graceful handling of email failures (offline mode)
- Try-catch around all critical operations

---

## 5. Real Email Integration — Technical Design

### Option A: PowerShell SMTP (Primary — RECOMMENDED)
```cpp
// From C++, execute PowerShell to send email via Gmail SMTP
string command = "powershell -Command \"Send-MailMessage "
    "-From 'spcms.system@gmail.com' "
    "-To '" + officerEmail + "' "
    "-Subject 'Complaint #" + to_string(id) + "' "
    "-Body '" + body + "' "
    "-SmtpServer 'smtp.gmail.com' "
    "-Port 587 -UseSsl "
    "-Credential (New-Object PSCredential('spcms.system@gmail.com', "
    "(ConvertTo-String -String 'app-password' -AsSecureString)))\"";
system(command.c_str());
```

### Option B: Python Helper Script (Fallback)
```cpp
// Call a Python script that uses smtplib
system("python send_email.py recipient subject body");
```

### Setup Required:
1. Create a Gmail account for the system (e.g., `spcms.noreply@gmail.com`)
2. Enable 2-Factor Authentication
3. Generate an App Password (Settings → Security → App Passwords)
4. Store App Password in a config file (not hardcoded)

### Email Content Template:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  SMART PUBLIC COMPLAINT MANAGEMENT SYSTEM
  Government of India — Municipal Corporation
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📋 COMPLAINT NOTIFICATION

Complaint ID  : #105
Department    : Public Works Department
Severity      : 🔴 Emergency
Status        : Pending

📝 Description:
Large pothole on MG Road near Junction 4, causing accidents.

👤 Filed By:
Name    : Rajesh Kumar
Contact : +91-9876543210
Email   : rajesh@example.com

📎 Photo Evidence: C:\Evidence\pothole_mg_road.jpg

⏰ Filed On: 2026-03-03 14:30:22
⏳ Deadline: 2026-03-03 26:30:22 (12 hours - Emergency)

👮 Assigned To: You
Action Required: Please investigate and update status.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
This is an automated notification from SPCMS.
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 6. Console Dashboard UI Design

### 6.1 Main Dashboard Screen
```
╔══════════════════════════════════════════════════════════════════════════════╗
║           🏛️  SMART PUBLIC COMPLAINT MANAGEMENT SYSTEM  🏛️                ║
║              Government of India — Municipal Corporation                    ║
╠══════════════════════════════════════════════════════════════════════════════╣
║                                                                            ║
║  📊 LIVE DASHBOARD                                         03-Mar-2026    ║
║  ─────────────────                                          14:30:22      ║
║                                                                            ║
║  ┌─── OVERVIEW ──────────┐  ┌─── BY STATUS ─────────────────────────────┐ ║
║  │ Total Complaints: 47  │  │ 🟢 Resolved    ████████████████░░  28     │ ║
║  │ Resolved:         28  │  │ 🟡 In Progress ██████░░░░░░░░░░░░  10    │ ║
║  │ Pending:           5  │  │ 🔴 Pending     ███░░░░░░░░░░░░░░░   5    │ ║
║  │ Escalated:         4  │  │ ⚠️  Escalated   ██░░░░░░░░░░░░░░░░   4    │ ║
║  └───────────────────────┘  └───────────────────────────────────────────┘ ║
║                                                                            ║
║  ┌─── BY DEPARTMENT ─────────────────┐  ┌─── BY SEVERITY ──────────────┐ ║
║  │ 🛣  Road         ████████░░  18   │  │ 🔴 Emergency  ██████░░  12  │ ║
║  │ 💧 Water        ██████░░░░  15   │  │ 🟡 Normal     ████████░  20  │ ║
║  │ ⚡ Electricity  ███████░░░  14   │  │ 🟢 Low        ███████░░  15  │ ║
║  └───────────────────────────────────┘  └──────────────────────────────┘ ║
║                                                                            ║
║  ┌─── RECENT ACTIVITY ──────────────────────────────────────────────────┐ ║
║  │ [14:28] ✅ Complaint #045 resolved by Officer Sharma                 │ ║
║  │ [14:15] 📧 Email sent to supervisor@water.gov.in (Escalation #042)  │ ║
║  │ [14:02] 🆕 New complaint #047 registered - Road - Emergency         │ ║
║  │ [13:45] ⬆️  Complaint #039 escalated to Level 2                      │ ║
║  │ [13:30] 📧 Email sent to officer@pwd.gov.in (New Assignment #046)   │ ║
║  └──────────────────────────────────────────────────────────────────────┘ ║
║                                                                            ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  [1] Register  [2] View All  [3] Search  [4] Update  [5] Escalation Check ║
║  [6] Email Log [7] Officers  [8] Feedback [9] Dashboard  [0] Exit         ║
╚══════════════════════════════════════════════════════════════════════════════╝
>> Enter choice:
```

### 6.2 Complaint Table View
```
╔════╦══════════════════════╦══════════════╦═══════════╦══════════════╦═══════════╗
║ ID ║ Description          ║ Department   ║ Severity  ║ Status       ║ Assigned  ║
╠════╬══════════════════════╬══════════════╬═══════════╬══════════════╬═══════════╣
║ 45 ║ Pothole on MG Road   ║ 🛣  PWD      ║ 🔴 Emerg  ║ ✅ Resolved  ║ Sharma    ║
║ 46 ║ Water leak Sec-21    ║ 💧 Water     ║ 🟡 Normal ║ 🔄 Progress  ║ Patel     ║
║ 47 ║ Power outage Ward-7  ║ ⚡ Electric  ║ 🔴 Emerg  ║ 🔴 Pending   ║ Gupta     ║
╚════╩══════════════════════╩══════════════╩═══════════╩══════════════╩═══════════╝
```

### 6.3 Complaint Detail View
```
╔══════════════════════════════════════════════════════════════╗
║                📋 COMPLAINT DETAILS — #047                  ║
╠══════════════════════════════════════════════════════════════╣
║                                                              ║
║  Department  : ⚡ Electricity Board                          ║
║  Severity    : 🔴 EMERGENCY                                 ║
║  Status      : 🔴 Pending                                   ║
║  Filed On    : 03-Mar-2026 14:02:15                         ║
║  Deadline    : 03-Mar-2026 02:02:15 (12hr Emergency)        ║
║  Time Left   : ⏳ 11h 32m remaining                         ║
║                                                              ║
║  ── DESCRIPTION ──────────────────────────────────────────  ║
║  Complete power outage in Ward-7, affecting 200+ homes.     ║
║  Transformers sparking near the main junction.              ║
║                                                              ║
║  ── CITIZEN INFO ─────────────────────────────────────────  ║
║  👤 Name    : Priya Verma                                   ║
║  📞 Phone   : +91-9988776655                                ║
║  📧 Email   : priya.verma@email.com                         ║
║                                                              ║
║  ── ASSIGNED OFFICER ─────────────────────────────────────  ║
║  👮 Name    : R.K. Gupta                                    ║
║  📧 Email   : rk.gupta@electricity.gov.in                   ║
║  🏅 Level   : 1 (Field Officer)                             ║
║                                                              ║
║  ── PHOTO EVIDENCE ───────────────────────────────────────  ║
║  📎 C:\Evidence\power_outage_ward7.jpg                      ║
║                                                              ║
║  ── EMAIL HISTORY ────────────────────────────────────────  ║
║  [14:02] 📧 → rk.gupta@electricity.gov.in  ✅ Sent         ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝
  [B] Back  [U] Update Status  [E] Escalate Manually
```

### 6.4 Color Scheme
```
HEADER / BORDERS     : Bright Cyan
TITLE TEXT           : Bright White + Bold
SUCCESS / RESOLVED   : Bright Green
WARNING / IN PROGRESS: Bright Yellow
ERROR / PENDING      : Bright Red
ESCALATED            : Bright Magenta
DEPARTMENT LABELS    : Bright Blue
NORMAL TEXT          : White
SUBTLE / TIMESTAMPS  : Gray
INPUT PROMPTS        : Bright Cyan
```

### 6.5 Loading / Transition Animations
- On startup: Animated ASCII art logo with typing effect
- On email send: `Sending email... [████████████████] Done ✅`
- On escalation check: `Checking complaints... [██░░░░░░░░] 2/10`
- Brief pause between screen transitions for polish

---

## 7. Complete File Structure (Updated)

```
CP/
├── main.cpp                          // Entry point, main menu loop
│
├── include/                          // All header files
│   ├── Citizen.h
│   ├── Officer.h
│   ├── Complaint.h                   // Abstract base class
│   ├── RoadComplaint.h
│   ├── WaterComplaint.h
│   ├── ElectricityComplaint.h
│   ├── Trackable.h                   // Mixin for multiple inheritance
│   ├── EmailService.h                // REAL email via SMTP
│   ├── EscalationManager.h          // Friend class of Complaint
│   ├── ComplaintSystem.h             // Main controller
│   ├── ConsoleUI.h                   // ALL visual rendering
│   ├── Logger.h                      // Class template (header-only)
│   ├── Exceptions.h                  // Custom exception classes
│   ├── FileManager.h                 // Data persistence
│   └── Config.h                      // Email credentials, constants
│
├── src/                              // All implementation files
│   ├── Citizen.cpp
│   ├── Officer.cpp
│   ├── Complaint.cpp
│   ├── RoadComplaint.cpp
│   ├── WaterComplaint.cpp
│   ├── ElectricityComplaint.cpp
│   ├── Trackable.cpp
│   ├── EmailService.cpp
│   ├── EscalationManager.cpp
│   ├── ComplaintSystem.cpp
│   ├── ConsoleUI.cpp
│   └── FileManager.cpp
│
├── data/                             // Runtime data files
│   ├── complaints.txt
│   ├── officers.txt
│   ├── email_logs.txt
│   └── feedback.txt
│
├── config/
│   └── email_config.txt              // Gmail credentials (App Password)
│
├── scripts/
│   └── send_email.py                 // Python fallback email sender
│
├── evidence/                         // Sample photo evidence folder
│   └── sample_pothole.jpg
│
├── Makefile                          // Build system (or compile.bat)
├── compile.bat                       // One-click Windows build
├── PRD.md                            // This document
└── README.md                         // Setup & usage instructions
```

---

## 8. OOP Syllabus ↔ Feature Mapping (Complete)

| # | Syllabus Topic | Implementation | File(s) |
|---|---|---|---|
| 1 | Class & Object | All 12+ classes | All files |
| 2 | Access Specifiers | private data, public methods, protected in Complaint | Throughout |
| 3 | Member Functions | Every class has methods | Throughout |
| 4 | Static Data Members | `Complaint::nextId`, `ComplaintSystem::totalCount` | Complaint.h |
| 5 | Scope Resolution `::` | All .cpp files define methods with `ClassName::method()` | All .cpp |
| 6 | Constructor (Default) | `Officer()`, `Citizen()`, etc. | All classes |
| 7 | Constructor (Parameterized) | `Officer(id, name, email, dept, level)` | All classes |
| 8 | Constructor (Copy) | `Complaint(const Complaint& other)` for escalation copy | Complaint.cpp |
| 9 | Constructor Overloading | Multiple constructors per class | Officer, Complaint |
| 10 | Destructors | `~ComplaintSystem()` frees `Complaint*` array | ComplaintSystem.cpp |
| 11 | Memory Management | `new`/`delete` for dynamic complaint creation | ComplaintSystem.cpp |
| 12 | Operator Overload (Unary) | `++complaint` increases severity | Complaint.cpp |
| 13 | Operator Overload (Binary) | `complaint1 == complaint2` compares by ID | Complaint.cpp |
| 14 | Operator Overload (Stream) | `operator<<` for printing any object | Multiple classes |
| 15 | Friend Function | `displayFullReport(const Complaint&)` | Complaint.h |
| 16 | Friend Class | `EscalationManager` is friend of `Complaint` | Complaint.h + EscalationManager.h |
| 17 | Single Inheritance | `RoadComplaint : public Complaint` | RoadComplaint.h |
| 18 | Multilevel Inheritance | `Complaint → UrgentComplaint → CriticalComplaint` (if needed) | Complaint hierarchy |
| 19 | Multiple Inheritance | `RoadComplaint : public Complaint, public Trackable` | RoadComplaint.h |
| 20 | Hierarchical Inheritance | Complaint → Road / Water / Electricity | 3 derived classes |
| 21 | Hybrid Inheritance | Combination of hierarchical + multiple (Trackable mixin) | Derived classes |
| 22 | Constructor Behavior in Inheritance | Base `Complaint()` called in derived constructors | Derived .cpp files |
| 23 | Virtual Functions | `virtual string getDepartment() = 0` | Complaint.h |
| 24 | Runtime Polymorphism | `Complaint* ptr = new RoadComplaint(); ptr->getDepartment();` | ComplaintSystem.cpp |
| 25 | Function Overloading | `registerComplaint()` with different param lists | ComplaintSystem.cpp |
| 26 | Function Overriding | `getDepartment()` overridden in each derived class | Derived classes |
| 27 | Pointers Basics | `Complaint*`, `Officer*` used throughout | ComplaintSystem.cpp |
| 28 | Pointer to Object | `Complaint* c = new RoadComplaint(...)` | ComplaintSystem.cpp |
| 29 | this Pointer | `this->name = name` in setters, `return *this` | Multiple classes |
| 30 | Dynamic Memory (new/delete) | Complaints dynamically allocated | ComplaintSystem.cpp |
| 31 | Array of Objects via Pointers | `Complaint** complaints = new Complaint*[MAX]` | ComplaintSystem.h |
| 32 | try / catch / throw | Input validation, search-not-found | ComplaintSystem.cpp |
| 33 | Multiple Catch Blocks | Catch specific then general exceptions | main.cpp |
| 34 | Custom Exceptions | `InvalidInputException`, `ComplaintNotFoundException`, `EscalationException` | Exceptions.h |
| 35 | Function Templates | `template<typename T> void displayList(T*, int)` | utils section in Logger.h |
| 36 | Class Templates | `template<typename T> class Logger` | Logger.h |
| 37 | STL Vectors | `vector<Officer>`, `vector<string>` for logs | Multiple files |
| 38 | STL Iterators | Iterate vectors to search/display | Multiple files |
| 39 | Inline Functions | `inline string getStatus()` and similar getters | Multiple .h files |
| 40 | Default Arguments | `registerComplaint(desc, severity = "Normal")` | ComplaintSystem.h |
| 41 | Abstraction | Abstract `Complaint` class with pure virtual method | Complaint.h |
| 42 | Encapsulation | Private data + public getters/setters everywhere | All classes |

**42/42 syllabus topics covered ✅**

---

## 9. Email Setup Instructions (For Actual Working Emails)

### Step 1: Create a Gmail Account for the System
- Create: `spcms.noreply@gmail.com` (or similar)
- This will be the "sender" address

### Step 2: Enable 2-Factor Authentication
- Go to Google Account → Security → 2-Step Verification → Turn On

### Step 3: Generate App Password
- Go to Google Account → Security → App Passwords
- Select "Mail" and "Windows Computer"
- Copy the 16-character password (e.g., `abcd efgh ijkl mnop`)

### Step 4: Create Config File
Create `config/email_config.txt`:
```
sender_email=spcms.noreply@gmail.com
app_password=abcdefghijklmnop
smtp_server=smtp.gmail.com
smtp_port=587
```

### Step 5: Email Sending Method (Python Helper — Most Reliable)
We use a small Python script called from C++ via `system()`:

**`scripts/send_email.py`:**
```python
import smtplib
import sys
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

def send(to_email, subject, body):
    sender = "spcms.noreply@gmail.com"
    password = "your-app-password"
    
    msg = MIMEMultipart()
    msg['From'] = sender
    msg['To'] = to_email
    msg['Subject'] = subject
    msg.attach(MIMEText(body, 'plain'))
    
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.starttls()
    server.login(sender, password)
    server.sendmail(sender, to_email, msg.as_string())
    server.quit()

if __name__ == "__main__":
    send(sys.argv[1], sys.argv[2], sys.argv[3])
```

**From C++:**
```cpp
void EmailService::sendRealEmail(string to, string subject, string body) {
    string cmd = "python scripts/send_email.py \"" + to + "\" \"" + subject + "\" \"" + body + "\"";
    int result = system(cmd.c_str());
    if (result == 0) {
        logEntry("[SUCCESS] Email sent to: " + to);
    } else {
        logEntry("[FAILED] Email to: " + to + " — Will retry later");
    }
}
```

---

## 10. Console UI Implementation — Technical Approach

### 10.1 Color System (Windows ANSI)
```cpp
// ConsoleUI.h — ANSI Escape Code Constants
namespace Color {
    const string RESET   = "\033[0m";
    const string RED     = "\033[91m";
    const string GREEN   = "\033[92m";
    const string YELLOW  = "\033[93m";
    const string BLUE    = "\033[94m";
    const string MAGENTA = "\033[95m";
    const string CYAN    = "\033[96m";
    const string WHITE   = "\033[97m";
    const string BOLD    = "\033[1m";
    const string DIM     = "\033[2m";
    const string BG_RED  = "\033[41m";
    const string BG_GREEN= "\033[42m";
}
```

### 10.2 Enable Virtual Terminal (Windows 10+ Required)
```cpp
#include <windows.h>
void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
```

### 10.3 Box Drawing Characters
```
Single: ┌ ─ ┐ │ └ ┘ ├ ┤ ┬ ┴ ┼
Double: ╔ ═ ╗ ║ ╚ ╝ ╠ ╣ ╦ ╩ ╬
```

### 10.4 Key UI Functions
```
ConsoleUI::drawHeader()           — System title with double-border box
ConsoleUI::drawDashboard(stats)   — Full dashboard with bars and stats
ConsoleUI::drawTable(complaints)  — Colored table with all complaints
ConsoleUI::drawDetailView(c)      — Single complaint full detail
ConsoleUI::drawMenu()             — Bottom menu bar
ConsoleUI::drawProgressBar(%)     — [████████░░░░] 67%
ConsoleUI::drawSuccess(msg)       — Green success message
ConsoleUI::drawError(msg)         — Red error message
ConsoleUI::drawWarning(msg)       — Yellow warning message
ConsoleUI::typeWriter(text, ms)   — Typing animation effect
ConsoleUI::clearScreen()          — Clear and redraw
ConsoleUI::drawEmailAnimation()   — Animated email sending effect
ConsoleUI::drawEscalationAlert()  — Flashing escalation warning
ConsoleUI::drawLoadingBar(task)   — Animated loading with message
```

---

## 11. Updated Implementation Phases

| Phase | What to Build | Files | Est. Time |
|---|---|---|---|
| **Phase 1: Foundation** | `Officer`, `Citizen`, `Complaint` (abstract), 3 derived classes, `Trackable` mixin, `Exceptions.h` | 12 files | Day 1 |
| **Phase 2: Console UI Engine** | `ConsoleUI` class — all drawing/rendering functions, color system, animations | 2 files | Day 1-2 |
| **Phase 3: Core Logic** | `ComplaintSystem` — register, view, search, update + `EscalationManager` (friend class) | 4 files | Day 2 |
| **Phase 4: Email Integration** | `EmailService` with real SMTP, Python helper script, `Config` loader, email templates | 4 files | Day 2-3 |
| **Phase 5: Templates & STL** | `Logger<T>` class template, function templates, vector usage, iterators | 2 files | Day 3 |
| **Phase 6: Operators & Polish** | All operator overloading (`++`, `==`, `<<`), `FileManager` persistence, feedback system | 3 files | Day 3 |
| **Phase 7: Integration** | `main.cpp` — menu loop, connect everything, dashboard, testing | 1 file + compile | Day 4 |
| **Phase 8: Demo Prep** | Demo-friendly timings (seconds instead of hours), sample data, test emails, `compile.bat` | Config + scripts | Day 4 |

---

## 12. Demo Strategy (For Assessment)

### What to Show First (Impact Order):
1. **Launch** → Animated logo + dashboard loads with pre-existing data
2. **Dashboard** → Teacher sees colored stats, bars, activity feed — instant "wow"
3. **Register a complaint** → Show the full flow with email animation
4. **Check teacher's phone** → "Check your email, you just received the complaint" 🔥
5. **Wait 30 seconds** → Run escalation check → Show auto-escalation + second email
6. **Search/Filter** → Show colored table output
7. **Resolve complaint** → Show status change + citizen feedback
8. **Email logs** → Show complete audit trail

### Pre-loaded Demo Data:
- 10-15 pre-existing complaints in various states
- 3-4 already escalated (so dashboard looks rich on startup)
- Pre-filled officer directory

---

## 13. Risk Mitigation

| Risk | Mitigation |
|---|---|
| Gmail blocks email | Use App Password; test beforehand; have fallback simulation mode |
| No internet at demo | System detects and switches to simulation mode (still logs everything) |
| Teacher asks about "real" SMTP | Explain: "We use Python's smtplib via system() call — industry-standard SMTP" |
| Console colors don't work | Test on demo machine beforehand; have fallback plain-text mode |
| Compilation issues | Provide `compile.bat` with exact g++ command; test on clean machine |

---

## 14. Viva Power Lines

1. **"The system sends real emails using SMTP via a Python helper script, which is an industry-standard approach."**
2. **"The console dashboard uses ANSI escape codes for rich color output, similar to how tools like npm and git display information."**
3. **"Automatic escalation ensures government accountability and timely resolution — this mirrors real RTI complaint systems."**
4. **"We use abstract classes and runtime polymorphism so new complaint types can be added without modifying existing code — Open/Closed Principle."**
5. **"The Logger class template demonstrates generic programming — it can log any data type while maintaining type safety."**
6. **"EscalationManager is a friend class of Complaint because it needs direct access to private timestamp and status data for time-critical escalation decisions."**

---

*Document Version: 1.0 | Created: 03-Mar-2026*
