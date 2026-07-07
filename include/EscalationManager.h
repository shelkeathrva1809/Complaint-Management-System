#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "Complaint.h"
#include "Officer.h"
#include "ConsoleUI.h"
using namespace std;

// Forward declaration — full definition in ComplaintSystem.h
class ComplaintSystem;

// ══════════════════════════════════════════════════════════
//  EscalationManager — Friend Class of Complaint
//  Demonstrates:
//    - Friend Class (accesses private members of Complaint)
//    - Encapsulation (private logic, public interface)
//    - Pointer to Objects (operates on Complaint*)
//    - Dynamic Binding (runtime polymorphism via Complaint*)
//    - Exception Handling (throws EscalationException)
//    - Static Member Functions (utility operations)
//    - this Pointer
//    - Inline Functions
// ══════════════════════════════════════════════════════════

class EscalationManager {
private:
    // ── Escalation configuration ──
    int maxEscalationLevel;     // max level before no further escalation
    int totalEscalations;       // running count of escalations performed

public:
    // ── Constructors ──
    EscalationManager();
    EscalationManager(int maxLevel);
    EscalationManager(const EscalationManager& other);

    // ── Destructor ──
    ~EscalationManager();

    // ══════════════════════════════════════════════════════
    //  CORE ESCALATION LOGIC
    //  These methods ACCESS PRIVATE MEMBERS of Complaint
    //  because EscalationManager is a friend class
    // ══════════════════════════════════════════════════════

    // Check a single complaint for escalation eligibility
    // Returns true if complaint was escalated
    bool checkAndEscalate(Complaint* complaint, ComplaintSystem& system);

    // Run escalation check on ALL complaints in the system
    // Returns count of complaints escalated in this run
    int runFullCheck(ComplaintSystem& system);

    // Manually escalate a complaint to a specific level
    void manualEscalate(Complaint* complaint, int targetLevel, ComplaintSystem& system);

    // ══════════════════════════════════════════════════════
    //  ESCALATION INFO (uses friend access for private members)
    // ══════════════════════════════════════════════════════

    // Get detailed escalation report for a complaint
    // Accesses private: createdTime, escalationLevel, status
    void printEscalationReport(const Complaint* complaint) const;

    // Check remaining time before escalation (in seconds)
    // Accesses private: createdTime, severity
    double getRemainingTime(const Complaint* complaint) const;

    // Check if complaint is overdue
    // Accesses private: createdTime, status
    bool isOverdue(const Complaint* complaint) const;

    // ── Getters (Inline) ──
    inline int getMaxLevel() const { return maxEscalationLevel; }
    inline int getTotalEscalations() const { return totalEscalations; }

    // ── Setters (this pointer) ──
    void setMaxLevel(int level) { this->maxEscalationLevel = level; }

    // ── Static Utility ──
    static string getEscalationSeverityLabel(int level);
};
