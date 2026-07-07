#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include "Complaint.h"
#include "RoadComplaint.h"
#include "WaterComplaint.h"
#include "ElectricityComplaint.h"
#include "Officer.h"
#include "SeniorOfficer.h"
#include "Citizen.h"
#include "Admin.h"
#include "Exceptions.h"
#include "ConsoleUI.h"
#include "DashboardWriter.h"
using namespace std;

// Forward declaration
class EscalationManager;

// ══════════════════════════════════════════════════════════
//  ComplaintSystem — Central Controller Class
//  Demonstrates:
//    - Encapsulation (private data, public interface)
//    - Object Composition (contains vectors of Officers,
//      Complaints, Citizens, Users)
//    - Pointers to Objects (Complaint* polymorphic storage)
//    - Dynamic Memory Allocation (new/delete for complaints)
//    - Array of Objects (Officer[], Citizen[])
//    - STL Containers (vector)
//    - Exception Handling (try-catch throughout)
//    - Runtime Polymorphism (Complaint* dispatch)
//    - Friend Class (grants EscalationManager access)
//    - Static Members (instance count)
//    - Role-Based Access Control
//    - Complaint History + Filtering
//    - Proof of Resolution + Rating System
// ══════════════════════════════════════════════════════════

class ComplaintSystem {
private:
    // ── Core Data ──
    vector<Complaint*> complaints;      // Polymorphic storage (base class pointers)
    vector<Officer> officers;           // Officer directory
    vector<SeniorOfficer> seniorOfficers;  // Senior officer directory
    vector<Citizen> citizens;           // Registered citizens
    vector<Admin> admins;              // Admin users

    // ── Email Audit Log ──
    vector<EmailLogEntry> emailLog;     // History of all emails sent
    vector<ActivityEntry> activityLog;  // System activity feed

    // ── Static ──
    static int instanceCount;           // How many ComplaintSystem objects exist

    // ── Private Helpers ──
    Officer* findOfficerByDepartment(const string& dept, int level);
    Officer* findOfficerById(int id);
    void logActivity(const string& text);
    void logEmail(const string& to, const string& subject, const string& status);

public:
    // ── Constructors ──
    ComplaintSystem();                                  // Default
    ComplaintSystem(const ComplaintSystem& other);       // Copy (deep copy)
    
    // ── Destructor (cleans up dynamic Complaint* memory) ──
    ~ComplaintSystem();

    // ── Assignment Operator (deep copy) ──
    ComplaintSystem& operator=(const ComplaintSystem& other);

    // ══════════════════════════════════════════════════════
    //  AUTHENTICATION & LOGIN
    // ══════════════════════════════════════════════════════

    // Authenticate a citizen by username/password
    Citizen* authenticateCitizen(const string& username, const string& password);

    // Authenticate an officer by username/password
    Officer* authenticateOfficer(const string& username, const string& password);

    // Authenticate a senior officer by username/password
    SeniorOfficer* authenticateSeniorOfficer(const string& username, const string& password);

    // Authenticate an admin by username/password
    Admin* authenticateAdmin(const string& username, const string& password);

    // ══════════════════════════════════════════════════════
    //  CORE OPERATIONS
    // ══════════════════════════════════════════════════════

    // Register a new complaint via interactive console prompts
    // Returns the new complaint's ID
    int registerComplaint();

    // Register a complaint with citizen email pre-filled
    int registerComplaintForCitizen(const string& citName, const string& citPhone, const string& citEmail);

    // Register a complaint programmatically (for demo/testing)
    int registerComplaintDirect(const string& type, const string& desc,
                                const string& photo, const string& severity,
                                const string& citName, const string& citContact,
                                const string& citEmail,
                                const string& extra1, const string& extra2 = "");

    // View all complaints in table format
    void viewAllComplaints() const;

    // Search for a complaint by ID with exception handling
    Complaint* searchComplaint(int id) const;

    // Update complaint status
    void updateComplaintStatus(int id, const string& newStatus);

    // View complaint detail (full report)
    void viewComplaintDetail(int id) const;

    // ══════════════════════════════════════════════════════
    //  PROOF OF RESOLUTION
    // ══════════════════════════════════════════════════════

    // Officer submits resolution proof
    void submitResolutionProof(int complaintId, const string& proof);

    // Citizen accepts resolution
    void citizenAcceptResolution(int complaintId);

    // Citizen rejects resolution
    void citizenRejectResolution(int complaintId);

    // ══════════════════════════════════════════════════════
    //  RATING SYSTEM
    // ══════════════════════════════════════════════════════

    // Citizen rates an officer for a complaint (1-5)
    void rateOfficer(int complaintId, int rating);

    // ══════════════════════════════════════════════════════
    //  COMPLAINT HISTORY & FILTERING
    // ══════════════════════════════════════════════════════

    // Get complaints submitted by a specific citizen (by email)
    vector<Complaint*> getComplaintsByEmail(const string& email) const;

    // Filter complaints by status
    vector<Complaint*> filterByStatus(const string& status) const;

    // Filter complaints by department/category
    vector<Complaint*> filterByDepartment(const string& dept) const;

    // Filter complaints by date (formatted as dd-Mon-yyyy)
    vector<Complaint*> filterByDate(const string& date) const;

    // Display filtered complaints in table
    void displayFilteredComplaints(const vector<Complaint*>& filtered, const string& filterLabel) const;

    // ══════════════════════════════════════════════════════
    //  OFFICER MANAGEMENT
    // ══════════════════════════════════════════════════════

    // Add an officer to the directory
    void addOfficer(const Officer& officer);

    // Add multiple officers at once
    void addOfficers(const vector<Officer>& officerList);

    // Add a senior officer
    void addSeniorOfficer(const SeniorOfficer& officer);

    // View officer directory
    void viewOfficers() const;

    // Get all officers (read-only)
    const vector<Officer>& getOfficers() const { return officers; }
    const vector<SeniorOfficer>& getSeniorOfficers() const { return seniorOfficers; }

    // Get complaints assigned to a specific officer
    vector<Complaint*> getComplaintsByOfficer(int officerId) const;

    // ══════════════════════════════════════════════════════
    //  CITIZEN MANAGEMENT
    // ══════════════════════════════════════════════════════

    // Register a citizen
    void addCitizen(const Citizen& citizen);

    // Add an admin
    void addAdmin(const Admin& admin);

    // Get all citizens (read-only)
    const vector<Citizen>& getCitizens() const { return citizens; }

    // ══════════════════════════════════════════════════════
    //  ESCALATION — Delegates to EscalationManager
    // ══════════════════════════════════════════════════════

    // Run escalation check on all complaints
    // Returns count of escalated complaints
    int runEscalationCheck();

    // ══════════════════════════════════════════════════════
    //  DASHBOARD & REPORTING
    // ══════════════════════════════════════════════════════

    // Build dashboard stats from current data
    DashboardStats buildDashboardStats() const;

    // Write JSON for web dashboard
    void writeDashboardJSON() const;

    // Open web dashboard in browser
    void openDashboard() const;

    // View email log in console
    void viewEmailLog() const;

    // View activity log in console
    void viewActivityLog() const;

    // ══════════════════════════════════════════════════════
    //  GETTERS
    // ══════════════════════════════════════════════════════

    inline int getComplaintCount() const { return static_cast<int>(complaints.size()); }
    inline int getOfficerCount() const { return static_cast<int>(officers.size()); }
    inline int getCitizenCount() const { return static_cast<int>(citizens.size()); }
    const vector<Complaint*>& getComplaints() const { return complaints; }
    const vector<EmailLogEntry>& getEmailLog() const { return emailLog; }
    const vector<ActivityEntry>& getActivityLog() const { return activityLog; }

    // ── Static ──
    static int getInstanceCount() { return instanceCount; }

    // ══════════════════════════════════════════════════════════
    //  FILE I/O — Save/Load System State
    // ══════════════════════════════════════════════════════════
    void saveToFile(const string& filename = "data/complaints.txt") const;
    void loadFromFile(const string& filename = "data/complaints.txt");

    // ── Friend Class ──
    friend class EscalationManager;
};
