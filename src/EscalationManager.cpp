#include "../include/EscalationManager.h"
#include "../include/ComplaintSystem.h"
#include "../include/Exceptions.h"
#include <cmath>

// ══════════════════════════════════════════════════════════
//  CONSTRUCTORS & DESTRUCTOR
// ══════════════════════════════════════════════════════════

EscalationManager::EscalationManager()
    : maxEscalationLevel(3), totalEscalations(0) {}

EscalationManager::EscalationManager(int maxLevel)
    : maxEscalationLevel(maxLevel), totalEscalations(0) {}

EscalationManager::EscalationManager(const EscalationManager& other)
    : maxEscalationLevel(other.maxEscalationLevel),
      totalEscalations(other.totalEscalations) {}

EscalationManager::~EscalationManager() {}

// ══════════════════════════════════════════════════════════
//  CORE: Check & Escalate a Single Complaint
//  FRIEND ACCESS: reads private createdTime, escalationLevel,
//                 status, severity, assignedOfficer, etc.
// ══════════════════════════════════════════════════════════

bool EscalationManager::checkAndEscalate(Complaint* complaint, ComplaintSystem& system) {
    // Skip resolved or already max-escalated complaints
    if (complaint->status == "Resolved") return false;

    // Check if time threshold has been exceeded
    double elapsed = difftime(time(0), complaint->createdTime);
    int threshold = complaint->getEscalationThreshold();

    if (elapsed < threshold) return false;       // not yet overdue
    if (complaint->escalationLevel >= maxEscalationLevel) {
        // Already at max — can't escalate further
        return false;
    }

    // ── PERFORM ESCALATION ──
    int oldLevel = complaint->escalationLevel;
    int newLevel = oldLevel + 1;

    // Update complaint's private members (friend access!)
    complaint->escalationLevel = newLevel;
    complaint->status = "Escalated";

    // Show escalation alert in console
    ConsoleUI::drawEscalationAlert(complaint->complaintId, newLevel);

    // Find higher-level officer from same department
    Officer* newOfficer = system.findOfficerByDepartment(complaint->getDepartment(), newLevel);
    if (!newOfficer && newLevel >= 3) {
        // Fall back to Municipal Commissioner
        newOfficer = system.findOfficerByDepartment("Municipal", 3);
    }

    if (newOfficer) {
        complaint->assignedOfficer = *newOfficer;
        complaint->isAssigned = true;

        ConsoleUI::drawSuccess("Reassigned to: " + newOfficer->getName() +
                               " (" + newOfficer->getLevelTitle() + ")");

        // Email notification
        string subject = "ESCALATION: Complaint #" + to_string(complaint->complaintId) +
                         " escalated to Level " + to_string(newLevel);
        system.logEmail(newOfficer->getEmail(), subject, "Sent");

        ConsoleUI::drawEmailAnimation(newOfficer->getEmail());

        // Log activity
        system.logActivity("[^] Complaint #" + to_string(complaint->complaintId) +
                           " escalated to Level " + to_string(newLevel) +
                           " -> " + newOfficer->getName());
    } else {
        throw EscalationException("No officer available at Level " + to_string(newLevel) +
                                  " for " + complaint->getDepartment());
    }

    totalEscalations++;
    return true;
}

// ══════════════════════════════════════════════════════════
//  Run Full Check on All Complaints
// ══════════════════════════════════════════════════════════

int EscalationManager::runFullCheck(ComplaintSystem& system) {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("ESCALATION CHECK");
    ConsoleUI::drawLoadingBar("Scanning complaints for deadline violations", 2000);

    int escalated = 0;
    int checked = 0;
    int skipped = 0;

    for (auto* complaint : system.complaints) {
        checked++;
        try {
            if (checkAndEscalate(complaint, system)) {
                escalated++;
            } else {
                skipped++;
            }
        } catch (const EscalationException& e) {
            ConsoleUI::drawWarning(string("Escalation issue: ") + e.what());
        }
    }

    cout << endl;
    if (escalated > 0) {
        ConsoleUI::drawWarning("ESCALATION SUMMARY: " + to_string(escalated) + " complaint(s) escalated!");
    } else {
        ConsoleUI::drawSuccess("No complaints require escalation at this time.");
    }
    ConsoleUI::drawInfo("Checked: " + to_string(checked) + " | Escalated: " + to_string(escalated) + " | OK: " + to_string(skipped));

    return escalated;
}

// ══════════════════════════════════════════════════════════
//  Manual Escalation
// ══════════════════════════════════════════════════════════

void EscalationManager::manualEscalate(Complaint* complaint, int targetLevel, ComplaintSystem& system) {
    if (targetLevel > maxEscalationLevel) {
        throw EscalationException("Target level " + to_string(targetLevel) +
                                  " exceeds maximum escalation level " + to_string(maxEscalationLevel));
    }
    if (targetLevel <= complaint->escalationLevel) {
        throw EscalationException("Cannot escalate backward. Current level: " +
                                  to_string(complaint->escalationLevel));
    }

    // Directly set escalation level (friend access)
    complaint->escalationLevel = targetLevel;
    complaint->status = "Escalated";

    ConsoleUI::drawEscalationAlert(complaint->complaintId, targetLevel);

    // Find officer at target level
    Officer* newOfficer = system.findOfficerByDepartment(complaint->getDepartment(), targetLevel);
    if (!newOfficer && targetLevel >= 3) {
        newOfficer = system.findOfficerByDepartment("Municipal", 3);
    }

    if (newOfficer) {
        complaint->assignedOfficer = *newOfficer;
        complaint->isAssigned = true;

        string subject = "MANUAL ESCALATION: Complaint #" + to_string(complaint->complaintId) +
                         " to Level " + to_string(targetLevel);
        system.logEmail(newOfficer->getEmail(), subject, "Sent");

        ConsoleUI::drawEmailAnimation(newOfficer->getEmail());

        system.logActivity("[^] MANUAL: Complaint #" + to_string(complaint->complaintId) +
                           " escalated to Level " + to_string(targetLevel) +
                           " -> " + newOfficer->getName());
    }

    totalEscalations++;
}

// ══════════════════════════════════════════════════════════
//  ESCALATION REPORT (uses friend access to private members)
// ══════════════════════════════════════════════════════════

void EscalationManager::printEscalationReport(const Complaint* complaint) const {
    double elapsed = difftime(time(0), complaint->createdTime);
    int threshold = complaint->getEscalationThreshold();
    double remaining = threshold - elapsed;

    cout << endl;
    cout << "  " << Color::BOLD << "ESCALATION REPORT — Complaint #" << complaint->complaintId << Color::RESET << endl;
    cout << "  " << ConsoleUI::repeat(Box::SH, 50) << endl;
    cout << "  Status           : " << complaint->status << endl;
    cout << "  Severity         : " << complaint->severity << endl;
    cout << "  Escalation Level : " << complaint->escalationLevel << " / " << maxEscalationLevel << endl;
    cout << "  Time Elapsed     : " << (int)elapsed << " seconds" << endl;
    cout << "  Threshold        : " << threshold << " seconds" << endl;

    if (remaining > 0) {
        cout << "  Time Remaining   : " << Color::GREEN << (int)remaining << " seconds" << Color::RESET << endl;
        cout << "  Status           : " << Color::GREEN << "Within deadline" << Color::RESET << endl;
    } else {
        cout << "  Overdue By       : " << Color::RED << (int)(-remaining) << " seconds" << Color::RESET << endl;
        cout << "  Status           : " << Color::RED << "OVERDUE — NEEDS ESCALATION" << Color::RESET << endl;
    }

    if (complaint->isAssigned) {
        cout << "  Current Officer  : " << complaint->assignedOfficer.getName()
             << " (Level " << complaint->assignedOfficer.getLevel() << ")" << endl;
    }
    cout << "  " << ConsoleUI::repeat(Box::SH, 50) << endl;
}

// ══════════════════════════════════════════════════════════
//  REMAINING TIME
// ══════════════════════════════════════════════════════════

double EscalationManager::getRemainingTime(const Complaint* complaint) const {
    double elapsed = difftime(time(0), complaint->createdTime);
    int threshold = complaint->getEscalationThreshold();
    return threshold - elapsed;
}

bool EscalationManager::isOverdue(const Complaint* complaint) const {
    if (complaint->status == "Resolved") return false;
    return getRemainingTime(complaint) < 0;
}

// ══════════════════════════════════════════════════════════
//  STATIC UTILITY
// ══════════════════════════════════════════════════════════

string EscalationManager::getEscalationSeverityLabel(int level) {
    switch (level) {
        case 0: return "Normal Processing";
        case 1: return "First Escalation — Supervisor Review";
        case 2: return "Second Escalation — Department Head";
        case 3: return "Critical — Municipal Commissioner";
        default: return "Maximum Escalation";
    }
}
