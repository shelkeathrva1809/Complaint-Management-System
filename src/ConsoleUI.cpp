#include "../include/ConsoleUI.h"
#include <algorithm>

// ══════════════════════════════════════════════════════════
//  Static Member Initialization
// ══════════════════════════════════════════════════════════
int ConsoleUI::consoleWidth = 82;
bool ConsoleUI::ansiEnabled = false;

// ══════════════════════════════════════════════════════════
//  INITIALIZATION
// ══════════════════════════════════════════════════════════

void ConsoleUI::init() {
    enableANSI();
    setConsoleSize(90, 42);
    // Set console code page to UTF-8 for box drawing chars
#ifdef _WIN32
    SetConsoleOutputCP(437);    // OEM charset for box-drawing
    SetConsoleCP(437);
#endif
}

void ConsoleUI::enableANSI() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
            ansiEnabled = true;
        }
    }
#else
    ansiEnabled = true;
#endif
}

void ConsoleUI::setConsoleSize(int width, int height) {
#ifdef _WIN32
    string cmd = "mode con: cols=" + to_string(width) + " lines=" + to_string(height);
    system(cmd.c_str());
    consoleWidth = width - 8;
#endif
}

// ══════════════════════════════════════════════════════════
//  SCREEN MANAGEMENT
// ══════════════════════════════════════════════════════════

void ConsoleUI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleUI::pause(const string& msg) {
    cout << "\n  " << Color::DIM << msg << Color::RESET;
    cin.ignore(10000, '\n');
    cin.get();
}

// ══════════════════════════════════════════════════════════
//  TEXT UTILITIES
// ══════════════════════════════════════════════════════════

string ConsoleUI::centerText(const string& text, int width) {
    if (width == 0) width = consoleWidth;
    int textLen = (int)text.length();
    if (textLen >= width) return text;
    int pad = (width - textLen) / 2;
    return string(pad, ' ') + text;
}

string ConsoleUI::padRight(const string& text, int width) {
    if ((int)text.length() >= width) return text.substr(0, width);
    return text + string(width - text.length(), ' ');
}

string ConsoleUI::padLeft(const string& text, int width) {
    if ((int)text.length() >= width) return text.substr(0, width);
    return string(width - text.length(), ' ') + text;
}

string ConsoleUI::repeat(const string& ch, int count) {
    string result = "";
    for (int i = 0; i < count; i++) result += ch;
    return result;
}

string ConsoleUI::truncate(const string& text, int maxLen) {
    if ((int)text.length() <= maxLen) return text;
    return text.substr(0, maxLen - 3) + "...";
}

// ══════════════════════════════════════════════════════════
//  TIME UTILITIES
// ══════════════════════════════════════════════════════════

string ConsoleUI::getCurrentTime() {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[20];
    strftime(buf, 20, "%H:%M:%S", t);
    return string(buf);
}

string ConsoleUI::getCurrentDate() {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[20];
    strftime(buf, 20, "%d-%b-%Y", t);
    return string(buf);
}

// ══════════════════════════════════════════════════════════
//  HEADERS & BORDERS
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawDoubleBar() {
    cout << "  " << Color::CYAN << Box::LT << repeat(Box::H, consoleWidth) << Box::RT << Color::RESET << endl;
}

void ConsoleUI::drawSingleBar() {
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::DIM << repeat(Box::SH, consoleWidth - 4) << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;
}

void ConsoleUI::drawHeader() {
    cout << endl;
    cout << "  " << Color::CYAN << Box::TL << repeat(Box::H, consoleWidth) << Box::TR << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << Color::WHITE
         << centerText("SMART PUBLIC COMPLAINT MANAGEMENT SYSTEM", consoleWidth)
         << Color::RESET
         << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << Color::DIM
         << centerText("Government of India  --  Municipal Corporation", consoleWidth)
         << Color::RESET
         << Color::CYAN << Box::V << Color::RESET << endl;
    drawDoubleBar();
}

void ConsoleUI::drawSectionTitle(const string& title) {
    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::BOLD << Color::YELLOW << title << Color::RESET
         << string(consoleWidth - (int)title.length() - 2, ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::DIM << repeat(Box::SH, (int)title.length() + 2) << Color::RESET
         << string(consoleWidth - (int)title.length() - 4, ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;
}

// ══════════════════════════════════════════════════════════
//  SPLASH SCREEN
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawSplashScreen() {
    clearScreen();
    cout << endl << endl << endl;
    cout << Color::CYAN << Color::BOLD;
    cout << centerText("====================================================") << endl;
    cout << centerText("||                                                ||") << endl;
    cout << centerText("||     SMART PUBLIC COMPLAINT MANAGEMENT SYSTEM   ||") << endl;
    cout << centerText("||                                                ||") << endl;
    cout << centerText("||          Government of India                   ||") << endl;
    cout << centerText("||          Municipal Corporation                 ||") << endl;
    cout << centerText("||                                                ||") << endl;
    cout << centerText("====================================================") << endl;
    cout << Color::RESET << endl;
    cout << Color::DIM << centerText("Version 1.0  |  OOP Course Project  |  C++17") << Color::RESET << endl;
    cout << endl;

    // Animated loading bar
    cout << endl;
    string loadMsg = "  Initializing system";
    cout << Color::GREEN;
    for (int i = 0; i < 3; i++) {
        cout << "\r" << centerText(loadMsg);
        cout.flush();
        loadMsg += ".";
        Sleep(400);
    }

    // Progress bar animation
    cout << endl;
    int barW = 40;
    for (int i = 0; i <= barW; i++) {
        cout << "\r" << string((consoleWidth - barW) / 2 + 2, ' ') << "[";
        for (int j = 0; j < barW; j++) {
            if (j < i) cout << Box::FULL;
            else cout << Box::SHADE;
        }
        int pct = (i * 100) / barW;
        cout << "] " << pct << "%";
        cout.flush();
        Sleep(25);
    }
    cout << Color::RESET << endl;
    cout << endl << Color::GREEN << centerText("[+] System Ready!") << Color::RESET << endl;
    Sleep(600);
}

// ══════════════════════════════════════════════════════════
//  MAIN MENU
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawMainMenu() {
    clearScreen();
    drawHeader();

    // Date & time row
    string dateTime = getCurrentDate() + "  |  " + getCurrentTime();
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << string(consoleWidth - (int)dateTime.length() - 2, ' ')
         << Color::DIM << dateTime << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    drawSingleBar();

    // Menu options
    string menuItems[] = {
        "  [1]  Register New Complaint",
        "  [2]  View All Complaints",
        "  [3]  Search Complaint by ID",
        "  [4]  Update Complaint Status",
        "  [5]  Run Escalation Check",
        "  [6]  View Email Logs",
        "  [7]  View Officer Directory",
        "  [8]  Give Citizen Feedback",
        "  [9]  Open Dashboard (Browser)",
        "  [0]  Exit System"
    };

    string colors[] = {
        Color::GREEN,    // 1 - Register
        Color::BLUE,     // 2 - View
        Color::CYAN,     // 3 - Search
        Color::YELLOW,   // 4 - Update
        Color::RED,      // 5 - Escalation
        Color::MAGENTA,  // 6 - Email
        Color::BLUE,     // 7 - Officers
        Color::GREEN,    // 8 - Feedback
        Color::CYAN,     // 9 - Dashboard
        Color::DIM       // 0 - Exit
    };

    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    for (int i = 0; i < 10; i++) {
        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << colors[i] << menuItems[i] << Color::RESET
             << string(consoleWidth - (int)menuItems[i].length(), ' ')
             << Color::CYAN << Box::V << Color::RESET << endl;
    }
    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    // Footer
    cout << "  " << Color::CYAN << Box::BL << repeat(Box::H, consoleWidth) << Box::BR << Color::RESET << endl;
    cout << endl;
}

// ══════════════════════════════════════════════════════════
//  LOGIN SCREEN
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawLoginScreen() {
    clearScreen();
    cout << endl << endl;
    cout << Color::CYAN << Color::BOLD;
    cout << centerText("====================================================") << endl;
    cout << centerText("||                                                ||") << endl;
    cout << centerText("||     SMART PUBLIC COMPLAINT MANAGEMENT SYSTEM   ||") << endl;
    cout << centerText("||                   LOGIN PORTAL                 ||") << endl;
    cout << centerText("||                                                ||") << endl;
    cout << centerText("====================================================") << endl;
    cout << Color::RESET << endl;
    cout << Color::DIM << centerText("Select your role to continue") << Color::RESET << endl;
    cout << endl;

    cout << "  " << Color::GREEN  << "  [1]  Citizen Login" << Color::RESET << endl;
    cout << "  " << Color::YELLOW << "  [2]  Officer Login" << Color::RESET << endl;
    cout << "  " << Color::MAGENTA << "  [3]  Senior Officer Login" << Color::RESET << endl;
    cout << "  " << Color::RED    << "  [4]  Admin Login" << Color::RESET << endl;
    cout << "  " << Color::DIM   << "  [0]  Exit System" << Color::RESET << endl;
    cout << endl;
}

// ══════════════════════════════════════════════════════════
//  CITIZEN MENU
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawCitizenMenu(const string& citizenName) {
    clearScreen();
    drawHeader();

    // Welcome message
    string dateTime = getCurrentDate() + "  |  " + getCurrentTime();
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::GREEN << Icon::PERSON << " Welcome, " << Color::BOLD << citizenName << Color::RESET
         << string(max(0, consoleWidth - (int)citizenName.length() - 16), ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << string(consoleWidth - (int)dateTime.length() - 2, ' ')
         << Color::DIM << dateTime << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    drawSingleBar();

    string menuItems[] = {
        "  [1]  Register New Complaint",
        "  [2]  View My Complaints",
        "  [3]  Search Complaint by ID",
        "  [4]  Filter Complaints",
        "  [5]  Accept/Reject Resolution",
        "  [6]  Rate Officer",
        "  [0]  Logout"
    };

    string colors[] = {
        Color::GREEN,    // 1 - Register
        Color::BLUE,     // 2 - View My
        Color::CYAN,     // 3 - Search
        Color::YELLOW,   // 4 - Filter
        Color::MAGENTA,  // 5 - Accept/Reject
        Color::GREEN,    // 6 - Rate
        Color::DIM       // 0 - Logout
    };

    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    for (int i = 0; i < 7; i++) {
        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << colors[i] << menuItems[i] << Color::RESET
             << string(consoleWidth - (int)menuItems[i].length(), ' ')
             << Color::CYAN << Box::V << Color::RESET << endl;
    }
    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::BL << repeat(Box::H, consoleWidth) << Box::BR << Color::RESET << endl;
    cout << endl;
}

// ══════════════════════════════════════════════════════════
//  OFFICER MENU
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawOfficerMenu(const string& officerName) {
    clearScreen();
    drawHeader();

    string dateTime = getCurrentDate() + "  |  " + getCurrentTime();
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::YELLOW << Icon::SHIELD << " Officer: " << Color::BOLD << officerName << Color::RESET
         << string(max(0, consoleWidth - (int)officerName.length() - 16), ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << string(consoleWidth - (int)dateTime.length() - 2, ' ')
         << Color::DIM << dateTime << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    drawSingleBar();

    string menuItems[] = {
        "  [1]  View Assigned Complaints",
        "  [2]  Update Complaint Status",
        "  [3]  Submit Resolution Proof",
        "  [4]  View All Complaints",
        "  [5]  Search Complaint by ID",
        "  [6]  View My Ratings",
        "  [7]  Run Escalation Check",
        "  [0]  Logout"
    };

    string colors[] = {
        Color::GREEN,    // 1
        Color::YELLOW,   // 2
        Color::MAGENTA,  // 3
        Color::BLUE,     // 4
        Color::CYAN,     // 5
        Color::GREEN,    // 6
        Color::RED,      // 7
        Color::DIM       // 0
    };

    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    for (int i = 0; i < 8; i++) {
        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << colors[i] << menuItems[i] << Color::RESET
             << string(consoleWidth - (int)menuItems[i].length(), ' ')
             << Color::CYAN << Box::V << Color::RESET << endl;
    }
    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::BL << repeat(Box::H, consoleWidth) << Box::BR << Color::RESET << endl;
    cout << endl;
}

// ══════════════════════════════════════════════════════════
//  SENIOR OFFICER MENU
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawSeniorOfficerMenu(const string& officerName) {
    clearScreen();
    drawHeader();

    string dateTime = getCurrentDate() + "  |  " + getCurrentTime();
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::MAGENTA << Icon::SHIELD << " Sr. Officer: " << Color::BOLD << officerName << Color::RESET
         << string(max(0, consoleWidth - (int)officerName.length() - 20), ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << string(consoleWidth - (int)dateTime.length() - 2, ' ')
         << Color::DIM << dateTime << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    drawSingleBar();

    string menuItems[] = {
        "  [1]  View Assigned Complaints",
        "  [2]  View Escalated Complaints",
        "  [3]  Update Complaint Status",
        "  [4]  Submit Resolution Proof",
        "  [5]  View All Complaints",
        "  [6]  Search Complaint by ID",
        "  [7]  View My Ratings",
        "  [8]  Run Escalation Check",
        "  [0]  Logout"
    };

    string colors[] = {
        Color::GREEN,    // 1
        Color::RED,      // 2
        Color::YELLOW,   // 3
        Color::MAGENTA,  // 4
        Color::BLUE,     // 5
        Color::CYAN,     // 6
        Color::GREEN,    // 7
        Color::RED,      // 8
        Color::DIM       // 0
    };

    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    for (int i = 0; i < 9; i++) {
        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << colors[i] << menuItems[i] << Color::RESET
             << string(consoleWidth - (int)menuItems[i].length(), ' ')
             << Color::CYAN << Box::V << Color::RESET << endl;
    }
    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::BL << repeat(Box::H, consoleWidth) << Box::BR << Color::RESET << endl;
    cout << endl;
}

// ══════════════════════════════════════════════════════════
//  ADMIN MENU
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawAdminMenu(const string& adminName) {
    clearScreen();
    drawHeader();

    string dateTime = getCurrentDate() + "  |  " + getCurrentTime();
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::RED << Icon::SHIELD << " Admin: " << Color::BOLD << adminName << Color::RESET
         << string(max(0, consoleWidth - (int)adminName.length() - 14), ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << string(consoleWidth - (int)dateTime.length() - 2, ' ')
         << Color::DIM << dateTime << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    drawSingleBar();

    string menuItems[] = {
        "  [1]  View Total Complaints",
        "  [2]  Department-wise Count",
        "  [3]  Pending vs Resolved Analysis",
        "  [4]  Officer Performance (Worst Performers)",
        "  [5]  View All Complaints",
        "  [6]  View Officer Directory",
        "  [7]  View Email Logs",
        "  [8]  Run Escalation Check",
        "  [9]  Open Web Dashboard",
        "  [0]  Logout"
    };

    string colors[] = {
        Color::GREEN,    // 1
        Color::BLUE,     // 2
        Color::YELLOW,   // 3
        Color::RED,      // 4
        Color::CYAN,     // 5
        Color::BLUE,     // 6
        Color::MAGENTA,  // 7
        Color::RED,      // 8
        Color::CYAN,     // 9
        Color::DIM       // 0
    };

    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    for (int i = 0; i < 10; i++) {
        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << colors[i] << menuItems[i] << Color::RESET
             << string(consoleWidth - (int)menuItems[i].length(), ' ')
             << Color::CYAN << Box::V << Color::RESET << endl;
    }
    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::BL << repeat(Box::H, consoleWidth) << Box::BR << Color::RESET << endl;
    cout << endl;
}

// ══════════════════════════════════════════════════════════
//  RATING STARS DISPLAY
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawRatingStars(int rating) {
    cout << "  " << Color::YELLOW;
    for (int i = 0; i < 5; i++) {
        if (i < rating) cout << Icon::STAR;
        else cout << Color::DIM << "." << Color::YELLOW;
    }
    cout << Color::RESET << " (" << rating << "/5)" << endl;
}

// ══════════════════════════════════════════════════════════
//  DASHBOARD
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawDashboard(const DashboardStats& stats) {
    clearScreen();
    drawHeader();

    // ── Title & Time ──
    string dateTime = getCurrentDate() + "  |  " + getCurrentTime();
    drawSectionTitle("LIVE DASHBOARD");

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << string(consoleWidth - (int)dateTime.length() - 2, ' ')
         << Color::DIM << dateTime << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;

    // ── OVERVIEW BOX ──
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::BOLD << "  OVERVIEW" << Color::RESET << "                          "
         << Color::BOLD << "  STATUS BREAKDOWN" << Color::RESET
         << string(consoleWidth - 56, ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    // Overview stats
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Total Complaints: " << Color::BOLD << Color::WHITE << padLeft(to_string(stats.totalComplaints), 4) << Color::RESET
         << "              "
         << Color::GREEN << Icon::CHECK << " Resolved    " << Color::RESET;
    drawProgressBar(stats.resolved, max(stats.totalComplaints, 1), 16);

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Resolved:         " << Color::GREEN << padLeft(to_string(stats.resolved), 4) << Color::RESET
         << "              "
         << Color::YELLOW << Icon::CLOCK << " In Progress " << Color::RESET;
    drawProgressBar(stats.inProgress, max(stats.totalComplaints, 1), 16);

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Pending:          " << Color::RED << padLeft(to_string(stats.pending), 4) << Color::RESET
         << "              "
         << Color::RED << Icon::CROSS << " Pending     " << Color::RESET;
    drawProgressBar(stats.pending, max(stats.totalComplaints, 1), 16);

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Escalated:        " << Color::MAGENTA << padLeft(to_string(stats.escalated), 4) << Color::RESET
         << "              "
         << Color::MAGENTA << Icon::WARN << " Escalated   " << Color::RESET;
    drawProgressBar(stats.escalated, max(stats.totalComplaints, 1), 16);

    drawSingleBar();

    // ── BY DEPARTMENT ──
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::BOLD << "  BY DEPARTMENT" << Color::RESET
         << string(consoleWidth - 17, ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    int maxDept = max({stats.roadCount, stats.waterCount, stats.electricityCount, 1});
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Road (PWD)   : ";
    drawBarChart("", stats.roadCount, maxDept, Color::YELLOW, 25);

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Water Supply : ";
    drawBarChart("", stats.waterCount, maxDept, Color::BLUE, 25);

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Electricity  : ";
    drawBarChart("", stats.electricityCount, maxDept, Color::MAGENTA, 25);

    drawSingleBar();

    // ── BY SEVERITY ──
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::BOLD << "  BY SEVERITY" << Color::RESET
         << string(consoleWidth - 15, ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    int maxSev = max({stats.emergencyCount, stats.normalCount, stats.lowCount, 1});
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Emergency : ";
    drawBarChart("", stats.emergencyCount, maxSev, Color::RED, 28);

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Normal    : ";
    drawBarChart("", stats.normalCount, maxSev, Color::YELLOW, 28);

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "    Low       : ";
    drawBarChart("", stats.lowCount, maxSev, Color::GREEN, 28);

    drawSingleBar();

    // ── RECENT ACTIVITY ──
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::BOLD << "  RECENT ACTIVITY" << Color::RESET
         << string(consoleWidth - 19, ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;

    if (stats.recentActivity.empty()) {
        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << "    " << Color::DIM << "No recent activity." << Color::RESET
             << string(consoleWidth - 23, ' ')
             << Color::CYAN << Box::V << Color::RESET << endl;
    } else {
        for (int i = 0; i < (int)stats.recentActivity.size() && i < 5; i++) {
            string entry = truncate(stats.recentActivity[i], consoleWidth - 8);
            cout << "  " << Color::CYAN << Box::V << Color::RESET
                 << "    " << Color::DIM << entry << Color::RESET
                 << string(max(0, consoleWidth - (int)entry.length() - 4), ' ')
                 << Color::CYAN << Box::V << Color::RESET << endl;
        }
    }

    cout << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::BL << repeat(Box::H, consoleWidth) << Box::BR << Color::RESET << endl;
}

// ══════════════════════════════════════════════════════════
//  COMPLAINT TABLE
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawComplaintTableHeader() {
    cout << endl;
    cout << "  " << Color::CYAN
         << Box::TL << repeat(Box::H, 5) << Box::TT
         << repeat(Box::H, 24) << Box::TT
         << repeat(Box::H, 14) << Box::TT
         << repeat(Box::H, 11) << Box::TT
         << repeat(Box::H, 13) << Box::TT
         << repeat(Box::H, 12) << Box::TR
         << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " ID  " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Description            " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Department   " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Severity  " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Status      " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Officer    " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN
         << Box::LT << repeat(Box::H, 5) << Box::CR
         << repeat(Box::H, 24) << Box::CR
         << repeat(Box::H, 14) << Box::CR
         << repeat(Box::H, 11) << Box::CR
         << repeat(Box::H, 13) << Box::CR
         << repeat(Box::H, 12) << Box::RT
         << Color::RESET << endl;
}

void ConsoleUI::drawComplaintTableRow(int id, const string& desc, const string& dept,
                                       const string& severity, const string& status,
                                       const string& officer) {
    // Color based on status
    string statusColor = Color::WHITE;
    string statusIcon = "";
    if (status == "Resolved") { statusColor = Color::GREEN; statusIcon = Icon::CHECK + " "; }
    else if (status == "In Progress") { statusColor = Color::YELLOW; statusIcon = Icon::CLOCK + " "; }
    else if (status == "Pending") { statusColor = Color::RED; statusIcon = Icon::CROSS + " "; }
    else if (status == "Escalated") { statusColor = Color::MAGENTA; statusIcon = Icon::WARN + " "; }

    // Color based on severity
    string sevColor = Color::WHITE;
    if (severity == "Emergency") sevColor = Color::RED;
    else if (severity == "Normal") sevColor = Color::YELLOW;
    else if (severity == "Low") sevColor = Color::GREEN;

    // Short department name
    string shortDept = dept;
    if (dept.find("PWD") != string::npos || dept.find("Road") != string::npos || dept.find("Public") != string::npos) shortDept = "Road/PWD";
    else if (dept.find("Water") != string::npos) shortDept = "Water";
    else if (dept.find("Elec") != string::npos) shortDept = "Electricity";

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << " " << padRight(to_string(id), 4)
         << Color::CYAN << Box::V << Color::RESET
         << " " << padRight(truncate(desc, 22), 23)
         << Color::CYAN << Box::V << Color::RESET
         << " " << padRight(truncate(shortDept, 12), 13)
         << Color::CYAN << Box::V << Color::RESET
         << " " << sevColor << padRight(severity, 10) << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << " " << statusColor << padRight(statusIcon + status, 12) << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << " " << padRight(truncate(officer, 10), 11)
         << Color::CYAN << Box::V << Color::RESET << endl;
}

void ConsoleUI::drawComplaintTableFooter() {
    cout << "  " << Color::CYAN
         << Box::BL << repeat(Box::H, 5) << Box::BT
         << repeat(Box::H, 24) << Box::BT
         << repeat(Box::H, 14) << Box::BT
         << repeat(Box::H, 11) << Box::BT
         << repeat(Box::H, 13) << Box::BT
         << repeat(Box::H, 12) << Box::BR
         << Color::RESET << endl;
}

// ══════════════════════════════════════════════════════════
//  DETAIL VIEW
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawDetailHeader(int complaintId) {
    cout << endl;
    cout << "  " << Color::CYAN << Box::TL << repeat(Box::H, 60) << Box::TR << Color::RESET << endl;
    string title = "COMPLAINT DETAILS -- #" + to_string(complaintId);
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << Color::WHITE << centerText(title, 60) << Color::RESET
         << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::LT << repeat(Box::H, 60) << Box::RT << Color::RESET << endl;
}

void ConsoleUI::drawDetailField(const string& label, const string& value) {
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::DIM << padRight(label, 16) << Color::RESET
         << ": " << value
         << string(max(0, 60 - (int)label.length() - (int)value.length() - 6), ' ')
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;
}

void ConsoleUI::drawDetailSection(const string& title) {
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::DIM << repeat(Box::SH, 56) << Color::RESET
         << "  " << Color::CYAN << Box::V << Color::RESET << endl;
    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << "  " << Color::BOLD << Color::YELLOW << title << Color::RESET
         << string(60 - (int)title.length() - 2, ' ')
         << Color::CYAN << Box::V << Color::RESET << endl;
}

void ConsoleUI::drawDetailFooter() {
    cout << "  " << Color::CYAN << Box::BL << repeat(Box::H, 60) << Box::BR << Color::RESET << endl;
}

// ══════════════════════════════════════════════════════════
//  STATUS MESSAGES
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawSuccess(const string& msg) {
    cout << "\n  " << Color::GREEN << Color::BOLD
         << Icon::CHECK << " " << msg
         << Color::RESET << endl;
}

void ConsoleUI::drawError(const string& msg) {
    cout << "\n  " << Color::RED << Color::BOLD
         << Icon::CROSS << " " << msg
         << Color::RESET << endl;
}

void ConsoleUI::drawWarning(const string& msg) {
    cout << "\n  " << Color::YELLOW << Color::BOLD
         << Icon::WARN << " " << msg
         << Color::RESET << endl;
}

void ConsoleUI::drawInfo(const string& msg) {
    cout << "\n  " << Color::CYAN
         << Icon::INFO << " " << msg
         << Color::RESET << endl;
}

// ══════════════════════════════════════════════════════════
//  PROGRESS & ANIMATION
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawProgressBar(int current, int total, int barWidth) {
    int filled = (total > 0) ? (current * barWidth / total) : 0;
    cout << "[";
    for (int i = 0; i < barWidth; i++) {
        if (i < filled) cout << Box::FULL;
        else cout << Box::SHADE;
    }
    cout << "] " << padLeft(to_string(current), 3) << endl;
}

void ConsoleUI::drawLoadingBar(const string& task, int durationMs) {
    int barW = 30;
    int steps = 40;
    int stepDelay = durationMs / steps;

    for (int i = 0; i <= steps; i++) {
        int filled = (i * barW) / steps;
        int pct = (i * 100) / steps;
        cout << "\r  " << task << " [";
        for (int j = 0; j < barW; j++) {
            if (j < filled) cout << Color::GREEN << Box::FULL << Color::RESET;
            else cout << Color::DIM << Box::SHADE << Color::RESET;
        }
        cout << "] " << pct << "%  ";
        cout.flush();
        Sleep(stepDelay);
    }
    cout << Color::GREEN << " Done!" << Color::RESET << endl;
}

void ConsoleUI::typeWriter(const string& text, int delayMs) {
    for (char c : text) {
        cout << c;
        cout.flush();
        Sleep(delayMs);
    }
}

void ConsoleUI::drawEmailAnimation(const string& recipient) {
    cout << endl;
    cout << "  " << Color::MAGENTA << Icon::MAIL << " Sending email to: " << recipient << Color::RESET << endl;
    drawLoadingBar("  " + string(Icon::MAIL) + " Delivering", 1200);
    cout << "  " << Color::GREEN << Icon::CHECK << " Email delivered successfully!" << Color::RESET << endl;
}

void ConsoleUI::drawEscalationAlert(int complaintId, int newLevel) {
    cout << endl;
    cout << "  " << Color::RED << Color::BOLD
         << "  =======================================" << endl;
    cout << "  " << Icon::WARN << " ESCALATION ALERT" << endl;
    cout << "  =======================================" << Color::RESET << endl;
    cout << "  " << Color::YELLOW
         << "  Complaint #" << complaintId
         << " escalated to Level " << newLevel << endl;
    string levelName = "";
    if (newLevel == 2) levelName = "Supervisor / Department Head";
    else if (newLevel == 3) levelName = "Municipal Commissioner";
    else levelName = "Level " + to_string(newLevel);
    cout << "  " << "  Now assigned to: " << levelName << Color::RESET << endl;
    cout << "  " << Color::RED << Color::BOLD
         << "  =======================================" << Color::RESET << endl;
}

// ══════════════════════════════════════════════════════════
//  BAR CHART
// ══════════════════════════════════════════════════════════

void ConsoleUI::drawBarChart(const string& label, int value, int maxValue,
                              const string& color, int barWidth) {
    if (!label.empty()) cout << padRight(label, 14) << " ";
    int filled = (maxValue > 0) ? (value * barWidth / maxValue) : 0;
    cout << color;
    for (int i = 0; i < barWidth; i++) {
        if (i < filled) cout << Box::FULL;
        else cout << Box::SHADE;
    }
    cout << Color::RESET << "  " << value << endl;
}

// ══════════════════════════════════════════════════════════
//  INPUT HELPERS
// ══════════════════════════════════════════════════════════

int ConsoleUI::getMenuChoice(int min, int max) {
    cout << "  " << Color::CYAN << ">> Enter choice: " << Color::RESET;
    int choice;
    cin >> choice;
    if (cin.fail() || choice < min || choice > max) {
        cin.clear();
        cin.ignore(10000, '\n');
        return -1;  // invalid
    }
    cin.ignore(10000, '\n');
    return choice;
}

string ConsoleUI::getInput(const string& prompt) {
    cout << "  " << Color::CYAN << prompt << ": " << Color::RESET;
    string input;
    getline(cin, input);
    return input;
}

int ConsoleUI::getIntInput(const string& prompt) {
    cout << "  " << Color::CYAN << prompt << ": " << Color::RESET;
    int val;
    cin >> val;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
        return -1;
    }
    cin.ignore(10000, '\n');
    return val;
}

string ConsoleUI::getChoiceInput(const string& prompt, const vector<string>& options) {
    cout << "\n  " << Color::CYAN << prompt << Color::RESET << endl;
    for (int i = 0; i < (int)options.size(); i++) {
        cout << "    [" << (i + 1) << "] " << options[i] << endl;
    }
    cout << "  " << Color::CYAN << ">> Choice: " << Color::RESET;
    int choice;
    cin >> choice;
    cin.ignore(10000, '\n');
    if (choice >= 1 && choice <= (int)options.size()) {
        return options[choice - 1];
    }
    return "";
}

// ══════════════════════════════════════════════════════════
//  INPUT VALIDATION FUNCTIONS
//  Demonstrates: Regex, Static Methods, Functional Programming
// ══════════════════════════════════════════════════════════

// 1. Name: Only alphabets (A-Z, a-z) and spaces, 2-50 chars
bool ConsoleUI::validateName(const string& name) {
    if (name.length() < 2 || name.length() > 50) return false;
    regex namePattern("^[A-Za-z][A-Za-z ]*[A-Za-z]$");
    return regex_match(name, namePattern);
}

// 2. Phone: Exactly 10 digits, starts with 6-9 (Indian standard)
bool ConsoleUI::validatePhone(const string& phone) {
    regex phonePattern("^[6-9][0-9]{9}$");
    return regex_match(phone, phonePattern);
}

// 3. Email: Valid format, only gmail.com / yahoo.com / outlook.com
bool ConsoleUI::validateEmail(const string& email) {
    regex emailPattern("^[a-zA-Z0-9][a-zA-Z0-9._%+-]*@(gmail\\.com|yahoo\\.com|outlook\\.com)$",
                        regex::icase);
    return regex_match(email, emailPattern);
}

// Generic validated input loop — keeps asking until validator returns true
string ConsoleUI::getValidatedInput(const string& prompt, const string& errorMsg,
                                     function<bool(const string&)> validator) {
    string input;
    while (true) {
        cout << "  " << Color::CYAN << prompt << ": " << Color::RESET;
        getline(cin, input);

        // Trim leading and trailing spaces
        size_t start = input.find_first_not_of(" \t");
        size_t end = input.find_last_not_of(" \t");
        if (start == string::npos) {
            input = "";
        } else {
            input = input.substr(start, end - start + 1);
        }

        // Collapse multiple internal spaces to single space
        string cleaned;
        bool lastWasSpace = false;
        for (char c : input) {
            if (c == ' ') {
                if (!lastWasSpace) { cleaned += c; lastWasSpace = true; }
            } else {
                cleaned += c;
                lastWasSpace = false;
            }
        }
        input = cleaned;

        if (validator(input)) {
            return input;
        }

        // Show error in red, matching CLI style
        cout << "  " << Color::RED << Color::BOLD
             << Icon::CROSS << " " << errorMsg
             << Color::RESET << endl;
    }
}

