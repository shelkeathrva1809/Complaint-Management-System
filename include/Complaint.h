#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include "Officer.h"
using namespace std;

// Forward declaration (friend class — defined in Phase 3)
class EscalationManager;

// ══════════════════════════════════════════════════════════
//  Complaint — Abstract Base Class
//  Demonstrates:
//    - Abstraction (pure virtual getDepartment, displayTypeSpecificInfo)
//    - Encapsulation (protected + private data, public interface)
//    - Static Data Members (nextId for auto-increment)
//    - Constructor Overloading (Default, Param, Copy)
//    - Virtual Destructor (for polymorphic deletion)
//    - Virtual Functions (forwardToOfficer — overridable)
//    - Operator Overloading: Unary ++, Binary ==, Stream <<
//    - Friend Function: displayFullReport()
//    - Friend Class: EscalationManager
//    - Inline Functions (getters)
//    - this Pointer (setters)
//    - Scope Resolution Operator (all definitions in .cpp)
//  Extended with:
//    - Proof of Resolution (officer provides proof text/path)
//    - Citizen Acceptance (accept/reject resolution)
//    - Rating System (citizen rates 1-5 after resolution)
//    - Date of Submission (formatted string)
// ══════════════════════════════════════════════════════════

class Complaint {
protected:
    int complaintId;
    string description;
    string photoPath;           // path to photo evidence
    time_t createdTime;
    string status;              // "Pending", "In Progress", "Resolved", "Escalated"
    string severity;            // "Low", "Normal", "Emergency"
    string citizenName;
    string citizenContact;
    string citizenEmail;
    Officer assignedOfficer;
    bool isAssigned;
    int escalationLevel;        // how many times escalated (0 = never)

    // ── NEW: Resolution & Rating Fields ──
    string resolutionProof;     // proof text or file path provided by officer
    bool citizenAccepted;       // true if citizen accepted the resolution
    bool resolutionSubmitted;   // true if officer has submitted proof
    int citizenRating;          // rating given by citizen (1-5), 0 = not rated
    string dateOfSubmission;    // formatted date string
    vector<string> statusTimeline;  // timestamped history of status changes

    static int nextId;          // auto-increment complaint ID

public:
    // ── Constructors ──
    Complaint();                                                                        // Default
    Complaint(const string& desc, const string& photo, const string& sev,
              const string& citName, const string& citContact,
              const string& citEmail);                                                  // Parameterized
    Complaint(const Complaint& other);                                                  // Copy

    // ── Virtual Destructor (REQUIRED for polymorphic base class) ──
    virtual ~Complaint();

    // ══════════════════════════════════════════════════════
    //  PURE VIRTUAL FUNCTIONS — Makes Complaint ABSTRACT
    //  Every derived class MUST override these
    // ══════════════════════════════════════════════════════
    virtual string getDepartment() const = 0;
    virtual void displayTypeSpecificInfo() const = 0;

    // ══════════════════════════════════════════════════════
    //  VIRTUAL FUNCTIONS — Can be overridden by derived
    // ══════════════════════════════════════════════════════
    virtual void forwardToOfficer();

    // ── Escalation ──
    void escalateComplaint();
    bool checkEscalation() const;
    double getSecondsPassed() const;
    int getEscalationThreshold() const;     // returns seconds based on severity

    // ══════════════════════════════════════════════════════
    //  PROOF OF RESOLUTION
    // ══════════════════════════════════════════════════════
    void submitResolutionProof(const string& proof);    // officer submits proof
    void acceptResolution();                             // citizen accepts
    void rejectResolution();                             // citizen rejects → back to In Progress

    // ══════════════════════════════════════════════════════
    //  RATING SYSTEM
    // ══════════════════════════════════════════════════════
    void setRating(int rating);                          // citizen rates 1-5

    // ══════════════════════════════════════════════════════
    //  STATUS TIMELINE
    // ══════════════════════════════════════════════════════
    void addStatusChange(const string& oldStatus, const string& newStatus);
    const vector<string>& getStatusTimeline() const { return statusTimeline; }

    // ══════════════════════════════════════════════════════
    //  OPERATOR OVERLOADING
    // ══════════════════════════════════════════════════════
    Complaint& operator++();                                        // Unary prefix: increase severity
    bool operator==(const Complaint& other) const;                  // Binary: compare by ID
    friend ostream& operator<<(ostream& os, const Complaint& c);   // Stream insertion

    // ══════════════════════════════════════════════════════
    //  FRIEND FUNCTION — accesses private members directly
    // ══════════════════════════════════════════════════════
    friend void displayFullReport(const Complaint& c);

    // ══════════════════════════════════════════════════════
    //  FRIEND CLASS — EscalationManager can access private
    //  time/status fields for escalation decisions
    // ══════════════════════════════════════════════════════
    friend class EscalationManager;

    // ── Getters (Inline Functions) ──
    inline int getId() const { return complaintId; }
    inline string getStatus() const { return status; }
    inline string getSeverity() const { return severity; }
    inline string getDescription() const { return description; }
    inline string getPhotoPath() const { return photoPath; }
    inline time_t getCreatedTime() const { return createdTime; }
    inline string getCitizenName() const { return citizenName; }
    inline string getCitizenContact() const { return citizenContact; }
    inline string getCitizenEmail() const { return citizenEmail; }
    inline Officer getAssignedOfficer() const { return assignedOfficer; }
    inline bool getIsAssigned() const { return isAssigned; }
    inline int getEscalationLevel() const { return escalationLevel; }

    // ── NEW Getters ──
    inline string getResolutionProof() const { return resolutionProof; }
    inline bool getCitizenAccepted() const { return citizenAccepted; }
    inline bool getResolutionSubmitted() const { return resolutionSubmitted; }
    inline int getCitizenRating() const { return citizenRating; }
    inline string getDateOfSubmission() const { return dateOfSubmission; }

    // ── Setters (using this pointer explicitly) ──
    void setStatus(const string& s) { this->status = s; }
    void setSeverity(const string& s) { this->severity = s; }
    void setDescription(const string& d) { this->description = d; }
    void setAssignedOfficer(const Officer& o) {
        this->assignedOfficer = o;
        this->isAssigned = true;
    }

    // ── Static Members ──
    static int getTotalComplaints() { return nextId - 1; }
    static int getNextId() { return nextId; }

    // ── Utility ──
    string getFormattedTime() const;        // human-readable creation time
    string getDeadlineTime() const;         // human-readable deadline time
};

// Friend function declaration (non-member)
void displayFullReport(const Complaint& c);
