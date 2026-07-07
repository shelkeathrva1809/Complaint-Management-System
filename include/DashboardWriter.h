#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include "ConsoleUI.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  DashboardWriter — Writes JSON data for web dashboard
//  Demonstrates: File I/O, Encapsulation, STL Vectors
//  Purpose: The C++ console app writes stats/complaints
//           to a JSON file that the HTML dashboard reads
// ══════════════════════════════════════════════════════════

struct ComplaintData {
    int id;
    string description;
    string department;
    string severity;
    string status;
    string officer;
    string time;
};

struct OfficerData {
    int id;
    string name;
    string email;
    string department;
    int level;
};

struct EmailLogEntry {
    string to;
    string subject;
    string time;
    string status;
};

struct ActivityEntry {
    string text;
    string time;
};

class DashboardWriter {
private:
    string filePath;

    // Helper: Escape JSON string (handle quotes and special chars)
    static string escapeJson(const string& s) {
        string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }

    static string getCurrentTimeStr() {
        time_t now = time(0);
        struct tm* t = localtime(&now);
        char buf[30];
        strftime(buf, 30, "%d-%b-%Y %H:%M:%S", t);
        return string(buf);
    }

public:
    DashboardWriter(const string& path = "data/dashboard.json")
        : filePath(path) {}

    void write(const DashboardStats& stats,
               const vector<ComplaintData>& complaints,
               const vector<OfficerData>& officers,
               const vector<EmailLogEntry>& emailLogs,
               const vector<ActivityEntry>& activities) {

        ofstream file(filePath);
        if (!file.is_open()) return;

        file << "{\n";

        // ── Stats Object ──
        file << "  \"stats\": {\n";
        file << "    \"total\": " << stats.totalComplaints << ",\n";
        file << "    \"resolved\": " << stats.resolved << ",\n";
        file << "    \"inProgress\": " << stats.inProgress << ",\n";
        file << "    \"pending\": " << stats.pending << ",\n";
        file << "    \"escalated\": " << stats.escalated << ",\n";
        file << "    \"road\": " << stats.roadCount << ",\n";
        file << "    \"water\": " << stats.waterCount << ",\n";
        file << "    \"electricity\": " << stats.electricityCount << ",\n";
        file << "    \"emergency\": " << stats.emergencyCount << ",\n";
        file << "    \"normal\": " << stats.normalCount << ",\n";
        file << "    \"low\": " << stats.lowCount << "\n";
        file << "  },\n";

        // ── Complaints Array ──
        file << "  \"complaints\": [\n";
        for (int i = 0; i < (int)complaints.size(); i++) {
            const auto& c = complaints[i];
            file << "    {\"id\": " << c.id
                 << ", \"description\": \"" << escapeJson(c.description) << "\""
                 << ", \"department\": \"" << escapeJson(c.department) << "\""
                 << ", \"severity\": \"" << escapeJson(c.severity) << "\""
                 << ", \"status\": \"" << escapeJson(c.status) << "\""
                 << ", \"officer\": \"" << escapeJson(c.officer) << "\""
                 << ", \"time\": \"" << escapeJson(c.time) << "\""
                 << "}";
            if (i < (int)complaints.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ],\n";

        // ── Officers Array ──
        file << "  \"officers\": [\n";
        for (int i = 0; i < (int)officers.size(); i++) {
            const auto& o = officers[i];
            file << "    {\"id\": " << o.id
                 << ", \"name\": \"" << escapeJson(o.name) << "\""
                 << ", \"email\": \"" << escapeJson(o.email) << "\""
                 << ", \"department\": \"" << escapeJson(o.department) << "\""
                 << ", \"level\": " << o.level
                 << "}";
            if (i < (int)officers.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ],\n";

        // ── Email Log Array ──
        file << "  \"emailLog\": [\n";
        for (int i = 0; i < (int)emailLogs.size(); i++) {
            const auto& e = emailLogs[i];
            file << "    {\"to\": \"" << escapeJson(e.to) << "\""
                 << ", \"subject\": \"" << escapeJson(e.subject) << "\""
                 << ", \"time\": \"" << escapeJson(e.time) << "\""
                 << ", \"status\": \"" << escapeJson(e.status) << "\""
                 << "}";
            if (i < (int)emailLogs.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ],\n";

        // ── Activity Array ──
        file << "  \"activity\": [\n";
        for (int i = 0; i < (int)activities.size(); i++) {
            const auto& a = activities[i];
            file << "    {\"text\": \"" << escapeJson(a.text) << "\""
                 << ", \"time\": \"" << escapeJson(a.time) << "\""
                 << "}";
            if (i < (int)activities.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ]\n";

        file << "}\n";
        file.close();
    }

    // Quick write with just stats (for fast updates)
    void writeStats(const DashboardStats& stats) {
        write(stats, {}, {}, {}, {});
    }

    // Open dashboard in default browser
    static void openInBrowser() {
#ifdef _WIN32
        system("start dashboard.html");
#elif __APPLE__
        system("open dashboard.html");
#else
        system("xdg-open dashboard.html");
#endif
    }
};
