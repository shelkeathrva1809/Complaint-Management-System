# 🏛️ SPCMS — Complete Project Working Document

> **Smart Public Complaint Management System** — Full-Stack C++ OOP Application  
> This document explains the **complete working** of the project: every process, every role, every flow.

---

## Table of Contents

1. [What is SPCMS?](#1-what-is-spcms)
2. [System Architecture](#2-system-architecture)
3. [User Roles & Responsibilities](#3-user-roles--responsibilities)
4. [Complete Complaint Lifecycle](#4-complete-complaint-lifecycle)
5. [Authentication Flow](#5-authentication-flow)
6. [Auto-Escalation System](#6-auto-escalation-system)
7. [Rating & Feedback System](#7-rating--feedback-system)
8. [Notification System](#8-notification-system)
9. [Dashboard & Analytics](#9-dashboard--analytics)
10. [Database Layer](#10-database-layer)
11. [API Endpoints](#11-api-endpoints)
12. [OOP Concepts Used](#12-oop-concepts-used)

---

## 1. What is SPCMS?

SPCMS is a **Smart Public Complaint Management System** — a government-grade application where citizens can file complaints about public infrastructure (roads, water supply, electricity), and government officers handle, resolve, and close those complaints.

The system has **two modes**:

| Mode | Entry Point | Purpose |
|------|------------|---------|
| **Console App** (`spcms.exe`) | `main.cpp` | Terminal-based interactive menu system |
| **Web App** (`spcms_web.exe`) | `web_main.cpp` | REST API server + React.js frontend |

Both modes demonstrate the same OOP class hierarchy, but the web version uses **SQLite** for persistence while the console version uses **in-memory storage + file I/O**.

---

## 2. System Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     USER (Browser)                          │
│              React.js Frontend (Port 5173)                  │
│   ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│   │  Login/  │  │Dashboard │  │Complaint │  │ Profile  │   │
│   │  Signup  │  │  + Stats │  │  Table   │  │  Card    │   │
│   └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
└───────────────────────┬─────────────────────────────────────┘
                        │ HTTP (JSON via Vite Proxy)
                        ▼
┌─────────────────────────────────────────────────────────────┐
│               C++ Backend Server (Port 8080)                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐   │
│  │  WebServer   │  │ AuthManager  │  │    Validator      │   │
│  │  (httplib)   │  │ (JWT + Hash) │  │ (Input Checks)   │   │
│  └──────┬───────┘  └──────────────┘  └──────────────────┘   │
│         │                                                   │
│  ┌──────▼───────────────────────────────────────────────┐   │
│  │              Database (Singleton)                     │   │
│  │  ┌──────────┐ ┌────────────┐ ┌───────────────────┐   │   │
│  │  │  Users   │ │ Complaints │ │  Notifications    │   │   │
│  │  └──────────┘ └────────────┘ └───────────────────┘   │   │
│  └──────────────────────┬───────────────────────────────┘   │
└─────────────────────────┼───────────────────────────────────┘
                          │
                    ┌─────▼─────┐
                    │  SQLite   │
                    │ spcms.db  │
                    └───────────┘
```

### Component Interaction Flow

```
User Action → React Component → API Call (fetch) → Vite Proxy → C++ WebServer
                                                                      │
                                                        ┌─────────────┤
                                                        ▼             ▼
                                                   AuthManager    Database
                                                   (verify JWT)  (SQLite query)
                                                        │             │
                                                        └──────┬──────┘
                                                               ▼
                                                        JSON Response
                                                               │
                                                               ▼
                                                     React State Update
                                                               │
                                                               ▼
                                                        UI Re-renders
```

---

## 3. User Roles & Responsibilities

### 3.1 Citizen 👤

| Action | What Happens |
|--------|-------------|
| **Register** | Signs up with name, email, phone, password. Password is hashed with salt. |
| **Login** | Authenticates → receives JWT token → redirected to dashboard |
| **File Complaint** | Selects type (Road/Water/Electricity), severity (Low/Normal/Emergency), writes description |
| **View My Complaints** | Sees all complaints they submitted with current status |
| **Accept/Reject Resolution** | When officer submits proof of resolution, citizen reviews and accepts or rejects |
| **Rate Officer** | After accepting resolution, rates the officer 1-5 stars with optional feedback |

### 3.2 Officer 👮

| Action | What Happens |
|--------|-------------|
| **View Assigned Complaints** | Sees complaints auto-assigned to them by department matching |
| **Update Status** | Changes status: `Pending → Assigned → In Progress → Resolved` |
| **Submit Resolution Proof** | Provides proof text/description that the issue was fixed |
| **View Ratings** | Sees their average performance rating from citizens |
| **Run Escalation Check** | Manually triggers the escalation engine |

### 3.3 Senior Officer 👨‍💼

*Everything an Officer can do, PLUS:*

| Action | What Happens |
|--------|-------------|
| **View Escalated Complaints** | Sees complaints that exceeded the time deadline |
| **Handle Escalated Cases** | Can take over complaints escalated from regular officers |
| **Update ANY Complaint** | Not restricted to only their assigned complaints |

### 3.4 Admin 🛡️

| Action | What Happens |
|--------|-------------|
| **View All Complaints** | Full table of every complaint in the system |
| **Analytics Dashboard** | Total counts, department-wise breakdown, pending vs resolved |
| **Officer Performance** | Identify worst-performing officers by rating |
| **Manage Users** | View all users, delete accounts |
| **Export CSV** | Export complaint data for reporting |
| **Run Escalation** | Trigger system-wide escalation check |

---

## 4. Complete Complaint Lifecycle

This is the **full flow** of a complaint from registration to closure:

### Step-by-Step Flow

```
STEP 1: CITIZEN REGISTERS COMPLAINT
├── Citizen selects complaint type: Road / Water / Electricity
├── Fills in: description, location, severity (Low/Normal/Emergency)
├── System validates all inputs (Validator class)
├── System creates complaint in database with status = "Pending"
├── System generates unique Tracking ID (e.g., SPCMS-2026-0001)
└── System auto-assigns to an officer by department matching

STEP 2: AUTO-ASSIGNMENT
├── System looks up officers whose department matches:
│   ├── Road complaint → PWD department officer
│   ├── Water complaint → Water Supply department officer
│   └── Electricity complaint → Electricity Board officer
├── Officer is selected (round-robin or first available)
├── Complaint status changes: "Pending" → "Assigned"
├── Notification sent to the assigned officer
└── Citizen receives tracking ID for reference

STEP 3: OFFICER WORKS ON COMPLAINT
├── Officer sees complaint in their "Assigned Complaints" list
├── Officer changes status: "Assigned" → "In Progress"
├── Officer investigates the issue in the field
├── [STATUS TIMELINE records every change with timestamp]
└── When fixed, officer moves to Step 4

STEP 4: OFFICER SUBMITS RESOLUTION PROOF
├── Officer changes status to "Resolved"
├── Officer writes proof of resolution (description of fix)
├── resolutionSubmitted = true
├── Notification sent to citizen: "Your complaint has been resolved"
└── Citizen is asked to review

STEP 5: CITIZEN REVIEWS RESOLUTION
├── Citizen sees the resolution proof
├── TWO CHOICES:
│
│   ✅ ACCEPT RESOLUTION:
│   ├── citizenAccepted = true
│   ├── Complaint status → "Closed"
│   ├── Citizen is prompted to rate the officer (1-5 stars)
│   └── Officer's average rating is updated
│
│   ❌ REJECT RESOLUTION:
│   ├── citizenAccepted remains false
│   ├── Complaint status → "In Progress" (reopened)
│   ├── resolutionSubmitted = false (officer must resubmit)
│   └── Officer is notified to re-investigate

STEP 6: RATING (after acceptance)
├── Citizen rates 1 to 5 stars
├── Optional written feedback
├── Rating is linked to the officer's profile
├── Officer's average rating recalculated
└── Complaint is fully closed
```

### Complaint Status State Machine

```
                    ┌──────────┐
                    │ PENDING  │ ◄── Citizen submits complaint
                    └────┬─────┘
                         │ Officer assigned
                         ▼
                    ┌──────────┐
                    │ ASSIGNED │
                    └────┬─────┘
                         │ Officer starts work
                         ▼
                 ┌───────────────┐
                 │  IN PROGRESS  │ ◄── Also: citizen rejects resolution
                 └───────┬───────┘
                         │ Officer submits proof
                         ▼
                    ┌──────────┐         ┌───────────┐
                    │ RESOLVED │────────►│  CLOSED   │  (citizen accepts + rates)
                    └──────────┘         └───────────┘
                         ▲
                         │
                 ┌───────┴───────┐
                 │  ESCALATED    │ ◄── Auto-escalation if deadline exceeded
                 └───────────────┘
```

### What Happens at Each Status?

| Status | Who Sets It | Meaning |
|--------|-------------|---------|
| **Pending** | System | Complaint just submitted, waiting for assignment |
| **Assigned** | System | Officer has been matched by department |
| **In Progress** | Officer | Officer is actively working on it |
| **Resolved** | Officer | Officer submitted resolution proof, awaiting citizen review |
| **Closed** | System | Citizen accepted the resolution |
| **Escalated** | EscalationManager | Deadline exceeded → forwarded to Senior Officer |

---

## 5. Authentication Flow

### Signup Process

```
Client → POST /api/auth/signup
         { name, email, phone, password, role }
                    │
                    ▼
         ┌──────────────────┐
         │   Validator       │  ← Validates name (letters only, 2-50 chars)
         │   Class           │  ← Validates phone (10 digits, starts 6-9)
         │                   │  ← Validates email (gmail/yahoo/outlook only)
         │                   │  ← Validates password (8+ chars, 1 upper, 1 number)
         └────────┬─────────┘
                  │ All valid?
                  ▼
         ┌──────────────────┐
         │   AuthManager     │  ← Creates random salt
         │                   │  ← Hashes: DJB2(password + salt)
         └────────┬─────────┘
                  │
                  ▼
         ┌──────────────────┐
         │   Database        │  ← Checks if email already exists
         │                   │  ← INSERT INTO users (name, email, hash, salt, role)
         │                   │  ← Returns new user ID
         └────────┬─────────┘
                  │
                  ▼
         ┌──────────────────┐
         │   AuthManager     │  ← Generates JWT token
         │   .generateToken  │     {userId, role, name} + HMAC signature
         └────────┬─────────┘
                  │
                  ▼
         JSON Response: { success: true, token: "xxx", user: {...} }
```

### Login Process

```
Client → POST /api/auth/login
         { email, password }
                    │
                    ▼
         Database.getUserByEmail(email)
                    │
                    ▼
         AuthManager.verifyPassword(password, storedHash, storedSalt)
                    │
              ┌─────┴─────┐
              │            │
           MATCH        NO MATCH
              │            │
              ▼            ▼
         Generate JWT   Return Error
         Return token   "Invalid credentials"
```

### Token Usage (Every API Call)

```
React Component → fetch('/api/complaints', {
    headers: { 'Authorization': 'Bearer <JWT_TOKEN>' }
})
         │
         ▼
WebServer.authenticate(req)
  → Extract token from "Authorization" header
  → AuthManager.verifyToken(token)
  → Returns { userId, role, name, valid }
         │
    ┌────┴────┐
    │         │
  VALID    INVALID
    │         │
    ▼         ▼
  Process   401 Unauthorized
  Request   (frontend redirects to login)
```

---

## 6. Auto-Escalation System

The system has an **automatic escalation engine** that escalates overdue complaints.

### How It Works

```
EscalationManager runs periodically (or manually triggered):

FOR EACH complaint WHERE status != "Resolved" AND status != "Closed":
│
├── Calculate time elapsed since complaint was created
├── Compare with severity-based deadline:
│   ├── Emergency  → 6 hours (21,600 seconds)
│   ├── Normal     → 24 hours (86,400 seconds)
│   └── Low        → 48 hours (172,800 seconds)
│
├── IF elapsed > deadline:
│   ├── Status → "Escalated"
│   ├── escalationLevel++ (tracks how many times escalated)
│   ├── Re-assign to a higher-level officer or Senior Officer
│   ├── Add entry to status timeline
│   ├── Notify both citizen and new officer
│   └── Log the escalation in activity feed
│
└── IF elapsed < deadline:
    └── Skip (still within SLA)
```

### Escalation Chain

```
Level 1: Field Officer (regular officer)
    │ (deadline exceeded)
    ▼
Level 2: Supervisor (senior officer)
    │ (deadline exceeded again)
    ▼
Level 3: Commissioner (highest authority)
    │ (no further escalation possible)
    ▼
EscalationException thrown: "No higher authority available"
```

### Key OOP Concept Here
- `EscalationManager` is a **friend class** of `Complaint`, which means it can directly access `Complaint`'s private fields (`createdTime`, `escalationLevel`, `status`) without going through getters. This demonstrates the **friend class** concept in C++.

---

## 7. Rating & Feedback System

### Flow

```
1. Officer resolves complaint and submits proof
2. Citizen reviews and ACCEPTS resolution
3. Complaint status → "Closed"
4. System prompts: "Rate the officer (1-5 stars)"
5. Citizen submits rating + optional text feedback
6. Rating stored in complaint record (citizenRating field)
7. Officer's aggregate rating updated:
   ├── totalRating += newRating
   ├── ratingCount++
   └── averageRating = totalRating / ratingCount
8. Admin can view "worst performing officers" sorted by avg rating
```

### What if Citizen Rejects?

```
1. Citizen sees resolution proof and clicks "Reject"
2. resolutionSubmitted → false
3. citizenAccepted remains false
4. Complaint status → "In Progress" (reopened)
5. Officer receives notification to re-investigate
6. Officer must submit NEW resolution proof
7. Cycle repeats until citizen accepts
```

---

## 8. Notification System

Notifications are generated at every major event:

| Event | Who Receives | Message |
|-------|-------------|---------|
| Complaint created | Officer | "New complaint #ID assigned to you" |
| Status updated | Citizen | "Complaint #ID status changed to X" |
| Resolution submitted | Citizen | "Officer submitted resolution for #ID" |
| Resolution accepted | Officer | "Citizen accepted resolution for #ID" |
| Resolution rejected | Officer | "Citizen rejected resolution for #ID" |
| Escalation | Sr. Officer | "Complaint #ID has been escalated" |
| Rating received | Officer | "You received a X-star rating for #ID" |

### API

- `GET /api/notifications` → Get all notifications for current user
- `POST /api/notifications/read-all` → Mark all as read
- Frontend polls every 30 seconds for new notifications

---

## 9. Dashboard & Analytics

### Stats Cards

| Card | Calculation |
|------|-------------|
| **Total Complaints** | `COUNT(*)` from complaints table |
| **Pending** | `COUNT(*) WHERE status IN ('Pending', 'Assigned')` |
| **Resolved** | `COUNT(*) WHERE status IN ('Resolved', 'Closed')` |
| **Emergency** | `COUNT(*) WHERE severity = 'Emergency'` |

### Charts

| Chart Type | Data Source |
|-----------|------------|
| **Bar Chart** | Complaints grouped by type (Road, Water, Electricity) |
| **Doughnut Chart** | Complaints grouped by status |
| **Line Chart** | Complaints filed over last 30 days (date vs count) |

### API Endpoints

```
GET /api/dashboard/stats
Response: { stats: { total, pending, resolved, emergency } }

GET /api/dashboard/charts
Response: {
    categories: { labels: ["Road","Water","Electricity"], values: [5,3,2] },
    statusDist: { labels: ["Pending","Resolved",...], values: [3,4,...] },
    timeline: { labels: ["2026-04-01",...], values: [2,1,...] }
}
```

---

## 10. Database Layer

### Tables

```sql
users (
    id INTEGER PRIMARY KEY,
    name TEXT, email TEXT UNIQUE, phone TEXT,
    password_hash TEXT, salt TEXT,
    role TEXT, department TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
)

complaints (
    id INTEGER PRIMARY KEY,
    tracking_id TEXT UNIQUE,
    user_id INTEGER REFERENCES users(id),
    citizen_name TEXT, citizen_email TEXT, citizen_phone TEXT,
    type TEXT, description TEXT, location TEXT,
    severity TEXT, status TEXT DEFAULT 'Pending',
    assigned_officer INTEGER REFERENCES users(id),
    officer_name TEXT, officer_email TEXT,
    attachment TEXT, resolution_proof TEXT,
    rating INTEGER DEFAULT 0, feedback TEXT,
    created_at DATETIME, updated_at DATETIME
)

notifications (
    id INTEGER PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    complaint_id INTEGER,
    message TEXT, is_read BOOLEAN DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
)
```

### Singleton Pattern

The `Database` class uses the **Singleton design pattern** — only ONE instance of the database connection exists in the entire application:

```cpp
class Database {
private:
    static Database* instance;  // single instance pointer
    Database();                 // private constructor
public:
    static Database& getInstance() {
        if (!instance) instance = new Database();
        return *instance;
    }
};
```

---

## 11. API Endpoints

### Authentication

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/auth/signup` | Register new user |
| POST | `/api/auth/login` | Login, returns JWT token |

### Complaints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/complaints` | Get all complaints (filtered by role) |
| POST | `/api/complaints` | Create new complaint (citizen only) |
| GET | `/api/complaints/:id` | Get single complaint detail |
| PATCH | `/api/complaints/:id/status` | Update complaint status (officer) |
| PATCH | `/api/complaints/:id/resolve` | Submit resolution proof (officer) |
| PATCH | `/api/complaints/:id/accept` | Accept resolution (citizen) |
| PATCH | `/api/complaints/:id/reject` | Reject resolution (citizen) |
| POST | `/api/complaints/:id/rate` | Rate officer (citizen) |

### Dashboard

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/dashboard/stats` | Get stat card numbers |
| GET | `/api/dashboard/charts` | Get chart data (categories, status, timeline) |

### Users & Notifications

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/users` | List all users (admin) |
| DELETE | `/api/users/:id` | Delete user (admin) |
| GET | `/api/notifications` | Get notifications for current user |
| POST | `/api/notifications/read-all` | Mark all notifications as read |

---

## 12. OOP Concepts Used

This project demonstrates **every major OOP concept** in C++:

### Core Four Pillars

| Concept | Where Used | Example |
|---------|-----------|---------|
| **Encapsulation** | Every class | Private data + public getters/setters in `User`, `Complaint`, `Officer` |
| **Inheritance** | `Officer : User`, `Citizen : User`, `Admin : User` | Derived classes inherit `authenticate()`, `getName()`, etc. |
| **Polymorphism** | `Complaint*` pointer array | `RoadComplaint`, `WaterComplaint`, `ElectricityComplaint` called through base pointer |
| **Abstraction** | `User` and `Complaint` are abstract | Pure virtual functions: `getRole()`, `getDepartment()` |

### Inheritance Types Demonstrated

| Type | Example |
|------|---------|
| **Single Inheritance** | `Citizen : User`, `Admin : User` |
| **Multilevel Inheritance** | `User → Officer → SeniorOfficer` |
| **Hierarchical Inheritance** | `Complaint → RoadComplaint`, `Complaint → WaterComplaint`, `Complaint → ElectricityComplaint` |
| **Multiple Inheritance** | `RoadComplaint : Complaint, Trackable` (each derived complaint type inherits from both) |

### Advanced OOP Features

| Feature | Where | Code Example |
|---------|-------|-------------|
| **Pure Virtual Functions** | `Complaint.h` | `virtual string getDepartment() const = 0;` |
| **Virtual Functions** | `Complaint.h` | `virtual void forwardToOfficer();` |
| **Virtual Destructor** | `User`, `Complaint` | `virtual ~Complaint();` |
| **Function Overriding** | `RoadComplaint` | `getDepartment()` returns `"Road"` |
| **Operator Overloading** | `Complaint` | `++` (increase severity), `==` (compare by ID), `<<` (stream output) |
| **Friend Functions** | `Complaint` | `friend void displayFullReport(const Complaint& c);` |
| **Friend Classes** | `EscalationManager` | `friend class EscalationManager;` in `Complaint` |
| **Static Members** | `Complaint::nextId`, `User::nextUserId` | Auto-increment IDs |
| **Static Methods** | `Validator::validateEmail()`, `Database::getInstance()` | Utility and Singleton |
| **this Pointer** | All setter methods | `this->name = n;` |
| **Inline Functions** | All getters | `inline int getId() const { return complaintId; }` |
| **Constructor Overloading** | Every class | Default, Parameterized, Copy constructors |
| **Copy Constructor** | `Complaint(const Complaint& other)` | Deep copy of complaint data |
| **Scope Resolution Operator** | All `.cpp` files | `void Complaint::setStatus(const string& s)` |
| **Access Specifiers** | Every class | `private`, `protected`, `public` sections |
| **Nested Structs** | `AuthManager` | `struct TokenPayload { ... };` |
| **Enum Class** | `User.h` | `enum class Role { CITIZEN, OFFICER, SENIOR_OFFICER, ADMIN };` |

### Design Patterns

| Pattern | Where | Purpose |
|---------|-------|---------|
| **Singleton** | `Database` | Single database connection for entire app |
| **Composition** | `WebServer` contains `Database&` + `AuthManager` | Server composed of subsystems |
| **Observer-like** | Notification system | Events trigger notifications to relevant users |
| **Factory-like** | `ComplaintSystem::registerComplaint()` | Creates `RoadComplaint`, `WaterComplaint`, or `ElectricityComplaint` based on type input |
| **RAII** | `Database` destructor | Closes SQLite connection when object destroyed |

### Exception Handling

| Exception Class | When Thrown |
|----------------|-----------|
| `InvalidInputException` | User provides invalid form data |
| `ComplaintNotFoundException` | Searching for a non-existent complaint ID |
| `EscalationException` | No higher officer available for escalation |
| `EmailException` | Email notification fails |
| `FileIOException` | File read/write error |

All custom exceptions inherit from `std::exception` and override `what()`.

### STL Usage

| STL Feature | Where |
|-------------|-------|
| `vector<Complaint*>` | Polymorphic complaint storage |
| `vector<Officer>` | Officer directory |
| `map<string, int>` | Analytics (category → count) |
| `pair<string, int>` | Timeline data |
| `string` | Used everywhere |
| Iterators | `Trackable::showHistory()` |

---

## Summary: Complete Process Flow (Big Picture)

```
CITIZEN                    SYSTEM                     OFFICER
───────                    ──────                     ───────
  │                          │                          │
  ├── Signs Up ──────────►  Creates User + JWT          │
  │                          │                          │
  ├── Logs In ───────────►  Verifies Password           │
  │                         Returns JWT Token           │
  │                          │                          │
  ├── Files Complaint ───►  Validates Input             │
  │   (Road/Water/Elec)     Creates in DB               │
  │                         Auto-assigns officer ────►  Gets notification
  │                          │                          │
  │                          │                     ◄──  Views assigned list
  │                          │                     ◄──  Changes → "In Progress"
  │                          │                     ◄──  Investigates issue
  │                          │                     ◄──  Submits resolution proof
  │                          │                          │
  │ ◄── Notification ───    "Complaint resolved"        │
  │                          │                          │
  ├── Reviews Proof          │                          │
  │                          │                          │
  ├── ACCEPTS ───────────►  Status → "Closed"           │
  │                          │                          │
  ├── Rates Officer (⭐) ─►  Updates officer rating ──► Sees new rating
  │                          │                          │
  └── DONE                   └──── COMPLAINT CLOSED ────┘

     ─── ALTERNATE: If citizen REJECTS ───
  │                          │                          │
  ├── REJECTS ───────────►  Status → "In Progress"      │
  │                         Notifies officer ────────►  Must resubmit proof
  │                          │                          │
  └── Cycle repeats          │                          │

     ─── ALTERNATE: If deadline exceeded ───
  │                          │                          │
  │                     EscalationManager runs           │
  │                     Status → "Escalated"             │
  │                     Reassigns to Senior Officer ──► Senior Officer handles
  │                          │                          │
```

---

*This document was auto-generated for the SPCMS project. Last updated: April 2026.*
