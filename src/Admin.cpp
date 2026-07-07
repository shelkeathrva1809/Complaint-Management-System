#include "../include/Admin.h"
#include "../include/ComplaintSystem.h"
#include "../include/ConsoleUI.h"
#include <algorithm>
#include <iomanip>

// ══════════════════════════════════════════════════════════
//  CONSTRUCTORS & DESTRUCTOR
// ══════════════════════════════════════════════════════════

Admin::Admin()
    : User(), adminLevel("Super Admin") {}

Admin::Admin(const string& username, const string& password,
             const string& name, const string& email,
             const string& phone, const string& adminLevel)
    : User(username, password, name, email, phone),
      adminLevel(adminLevel) {}

Admin::Admin(const Admin& other)
    : User(other), adminLevel(other.adminLevel) {}

Admin::~Admin() {}

// ══════════════════════════════════════════════════════════
//  ADMIN ANALYTICS FUNCTIONS
// ══════════════════════════════════════════════════════════

void Admin::viewTotalComplaints(const ComplaintSystem& system) const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("TOTAL COMPLAINTS OVERVIEW");

    int total = system.getComplaintCount();
    cout << endl;
    cout << "  " << Color::BOLD << Color::WHITE
         << "  Total Complaints in System: " << total
         << Color::RESET << endl;

    // Count by status
    int pending = 0, inProgress = 0, resolved = 0, escalated = 0;
    for (const auto* c : system.getComplaints()) {
        string s = c->getStatus();
        if (s == "Pending") pending++;
        else if (s == "In Progress") inProgress++;
        else if (s == "Resolved") resolved++;
        else if (s == "Escalated") escalated++;
    }

    cout << endl;
    cout << "  " << Color::GREEN << "  Resolved    : " << resolved << Color::RESET << endl;
    cout << "  " << Color::YELLOW << "  In Progress : " << inProgress << Color::RESET << endl;
    cout << "  " << Color::RED << "  Pending     : " << pending << Color::RESET << endl;
    cout << "  " << Color::MAGENTA << "  Escalated   : " << escalated << Color::RESET << endl;

    int maxVal = max({pending, inProgress, resolved, escalated, 1});
    cout << endl;
    cout << "  " << Color::GREEN << "  Resolved    "; ConsoleUI::drawBarChart("", resolved, maxVal, Color::GREEN, 25);
    cout << "  " << Color::YELLOW << "  In Progress "; ConsoleUI::drawBarChart("", inProgress, maxVal, Color::YELLOW, 25);
    cout << "  " << Color::RED << "  Pending     "; ConsoleUI::drawBarChart("", pending, maxVal, Color::RED, 25);
    cout << "  " << Color::MAGENTA << "  Escalated   "; ConsoleUI::drawBarChart("", escalated, maxVal, Color::MAGENTA, 25);
}

void Admin::viewDepartmentWiseCount(const ComplaintSystem& system) const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("DEPARTMENT-WISE COMPLAINT COUNT");

    int road = 0, water = 0, electricity = 0;
    for (const auto* c : system.getComplaints()) {
        string d = c->getDepartment();
        if (d.find("PWD") != string::npos || d.find("Public") != string::npos || d.find("Road") != string::npos) road++;
        else if (d.find("Water") != string::npos) water++;
        else if (d.find("Elec") != string::npos) electricity++;
    }

    int maxDept = max({road, water, electricity, 1});
    cout << endl;
    cout << "  " << Color::BOLD << "  Department Breakdown:" << Color::RESET << endl;
    cout << endl;
    cout << "    Road / PWD        : ";
    ConsoleUI::drawBarChart("", road, maxDept, Color::YELLOW, 30);
    cout << "    Water Supply      : ";
    ConsoleUI::drawBarChart("", water, maxDept, Color::BLUE, 30);
    cout << "    Electricity Board : ";
    ConsoleUI::drawBarChart("", electricity, maxDept, Color::MAGENTA, 30);

    cout << endl;
    ConsoleUI::drawInfo("Total: " + to_string(road + water + electricity) + " complaints across 3 departments");
}

void Admin::viewPendingVsResolved(const ComplaintSystem& system) const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("PENDING vs RESOLVED ANALYSIS");

    int pending = 0, resolved = 0;
    for (const auto* c : system.getComplaints()) {
        if (c->getStatus() == "Pending") pending++;
        else if (c->getStatus() == "Resolved") resolved++;
    }

    int total = system.getComplaintCount();
    double resolvedPct = (total > 0) ? (resolved * 100.0 / total) : 0;
    double pendingPct = (total > 0) ? (pending * 100.0 / total) : 0;

    cout << endl;
    cout << "  " << Color::BOLD << "  Resolution Statistics:" << Color::RESET << endl;
    cout << endl;
    cout << "    " << Color::GREEN << "Resolved : " << resolved << " (" 
         << fixed << setprecision(1) << resolvedPct << "%)" << Color::RESET << endl;
    cout << "    " << Color::RED << "Pending  : " << pending << " ("
         << fixed << setprecision(1) << pendingPct << "%)" << Color::RESET << endl;
    cout << endl;

    cout << "    Resolved ";
    ConsoleUI::drawBarChart("", resolved, max(total, 1), Color::GREEN, 30);
    cout << "    Pending  ";
    ConsoleUI::drawBarChart("", pending, max(total, 1), Color::RED, 30);

    if (resolvedPct >= 80) {
        ConsoleUI::drawSuccess("Excellent! Resolution rate is above 80%");
    } else if (resolvedPct >= 50) {
        ConsoleUI::drawWarning("Resolution rate is moderate. Needs improvement.");
    } else {
        ConsoleUI::drawError("Critical! Too many unresolved complaints.");
    }
}

void Admin::viewWorstPerformingOfficers(const ComplaintSystem& system) const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("OFFICER PERFORMANCE ANALYSIS");

    const auto& officers = system.getOfficers();
    const auto& complaints = system.getComplaints();

    if (officers.empty()) {
        ConsoleUI::drawWarning("No officers in the system.");
        return;
    }

    // Build per-officer stats
    struct OfficerStat {
        int id;
        string name;
        string department;
        int totalAssigned;
        int pendingCount;
        int resolvedCount;
        double avgRating;
        int ratingCount;
    };

    vector<OfficerStat> stats;
    for (const auto& o : officers) {
        OfficerStat stat;
        stat.id = o.getId();
        stat.name = o.getName();
        stat.department = o.getDepartment();
        stat.totalAssigned = 0;
        stat.pendingCount = 0;
        stat.resolvedCount = 0;
        stat.avgRating = o.getAverageRating();
        stat.ratingCount = o.getRatingCount();

        for (const auto* c : complaints) {
            if (c->getIsAssigned() && c->getAssignedOfficer().getId() == o.getId()) {
                stat.totalAssigned++;
                if (c->getStatus() == "Pending" || c->getStatus() == "Escalated") {
                    stat.pendingCount++;
                } else if (c->getStatus() == "Resolved") {
                    stat.resolvedCount++;
                }
            }
        }
        stats.push_back(stat);
    }

    // Sort by pending count (desc) then by rating (asc) — worst first
    sort(stats.begin(), stats.end(), [](const OfficerStat& a, const OfficerStat& b) {
        if (a.pendingCount != b.pendingCount) return a.pendingCount > b.pendingCount;
        return a.avgRating < b.avgRating;
    });

    // Display table
    cout << endl;
    cout << "  " << Color::CYAN
         << Box::TL << ConsoleUI::repeat(Box::H, 6) << Box::TT
         << ConsoleUI::repeat(Box::H, 20) << Box::TT
         << ConsoleUI::repeat(Box::H, 16) << Box::TT
         << ConsoleUI::repeat(Box::H, 10) << Box::TT
         << ConsoleUI::repeat(Box::H, 10) << Box::TT
         << ConsoleUI::repeat(Box::H, 10) << Box::TR
         << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " ID   " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Name               " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Department     " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Pending  " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Resolved " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Rating   " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN
         << Box::LT << ConsoleUI::repeat(Box::H, 6) << Box::CR
         << ConsoleUI::repeat(Box::H, 20) << Box::CR
         << ConsoleUI::repeat(Box::H, 16) << Box::CR
         << ConsoleUI::repeat(Box::H, 10) << Box::CR
         << ConsoleUI::repeat(Box::H, 10) << Box::CR
         << ConsoleUI::repeat(Box::H, 10) << Box::RT
         << Color::RESET << endl;

    for (const auto& s : stats) {
        string pendColor = (s.pendingCount > 0) ? Color::RED : Color::GREEN;
        string ratingStr = (s.ratingCount > 0) ? 
            (to_string((int)(s.avgRating * 10) / 10) + "." + to_string((int)(s.avgRating * 10) % 10) + "/5") : "N/A";

        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(to_string(s.id), 5)
             << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ConsoleUI::truncate(s.name, 18), 19)
             << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ConsoleUI::truncate(s.department, 14), 15)
             << Color::CYAN << Box::V << Color::RESET
             << " " << pendColor << ConsoleUI::padRight(to_string(s.pendingCount), 9) << Color::RESET
             << Color::CYAN << Box::V << Color::RESET
             << " " << Color::GREEN << ConsoleUI::padRight(to_string(s.resolvedCount), 9) << Color::RESET
             << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ratingStr, 9)
             << Color::CYAN << Box::V << Color::RESET << endl;
    }

    cout << "  " << Color::CYAN
         << Box::BL << ConsoleUI::repeat(Box::H, 6) << Box::BT
         << ConsoleUI::repeat(Box::H, 20) << Box::BT
         << ConsoleUI::repeat(Box::H, 16) << Box::BT
         << ConsoleUI::repeat(Box::H, 10) << Box::BT
         << ConsoleUI::repeat(Box::H, 10) << Box::BT
         << ConsoleUI::repeat(Box::H, 10) << Box::BR
         << Color::RESET << endl;

    // Highlight worst performers
    if (!stats.empty() && stats[0].pendingCount > 0) {
        ConsoleUI::drawWarning("Worst performer: " + stats[0].name + 
                               " (" + to_string(stats[0].pendingCount) + " pending complaints)");
    }
}

void Admin::viewEscalatedComplaints(const ComplaintSystem& system) const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("ESCALATED COMPLAINTS");

    int count = 0;
    ConsoleUI::drawComplaintTableHeader();
    for (const auto* c : system.getComplaints()) {
        if (c->getStatus() == "Escalated") {
            ConsoleUI::drawComplaintTableRow(
                c->getId(), c->getDescription(), c->getDepartment(),
                c->getSeverity(), c->getStatus(),
                c->getAssignedOfficer().getName());
            count++;
        }
    }
    ConsoleUI::drawComplaintTableFooter();

    if (count == 0) {
        ConsoleUI::drawSuccess("No escalated complaints at this time.");
    } else {
        ConsoleUI::drawWarning(to_string(count) + " complaint(s) currently escalated!");
    }
}

void Admin::showAdminDashboard(const ComplaintSystem& system) const {
    viewTotalComplaints(system);
}
