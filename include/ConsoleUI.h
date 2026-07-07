#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <sstream>
#include <ctime>
#include <regex>
#include <algorithm>
#include <functional>

#ifdef _WIN32
#define byte win_byte_override
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#include <windows.h>
#undef byte
#endif

using namespace std;

// ══════════════════════════════════════════════════════════
//  ConsoleUI — Rich Console Rendering Engine
//  Demonstrates: Encapsulation, Static Members,
//                Inline Functions, Default Arguments
//  Purpose: All visual output goes through this class
//           so the console looks professional and polished
// ══════════════════════════════════════════════════════════

namespace Color {
    // ── ANSI Escape Codes ──
    const string RESET   = "\033[0m";
    const string BOLD    = "\033[1m";
    const string DIM     = "\033[2m";
    const string ITALIC  = "\033[3m";
    const string UNDERLINE = "\033[4m";

    // Foreground
    const string BLACK   = "\033[30m";
    const string RED     = "\033[91m";
    const string GREEN   = "\033[92m";
    const string YELLOW  = "\033[93m";
    const string BLUE    = "\033[94m";
    const string MAGENTA = "\033[95m";
    const string CYAN    = "\033[96m";
    const string WHITE   = "\033[97m";
    const string GRAY    = "\033[90m";

    // Background
    const string BG_RED     = "\033[41m";
    const string BG_GREEN   = "\033[42m";
    const string BG_YELLOW  = "\033[43m";
    const string BG_BLUE    = "\033[44m";
    const string BG_MAGENTA = "\033[45m";
    const string BG_CYAN    = "\033[46m";
    const string BG_WHITE   = "\033[47m";
}

// Status icons mapping
namespace Icon {
    const string CHECK     = "[+]";
    const string CROSS     = "[X]";
    const string WARN      = "[!]";
    const string INFO      = "[i]";
    const string MAIL      = "[@]";
    const string NEW       = "[*]";
    const string UP        = "[^]";
    const string CLOCK     = "[~]";
    const string STAR      = "[*]";
    const string PERSON    = "[P]";
    const string PHONE     = "[#]";
    const string FILE      = "[F]";
    const string SHIELD    = "[S]";
}

// ── Box Drawing Characters ──
namespace Box {
    // Double border
    const string TL = "\xC9";  // ╔
    const string TR = "\xBB";  // ╗
    const string BL = "\xC8";  // ╚
    const string BR = "\xBC";  // ╝
    const string H  = "\xCD";  // ═
    const string V  = "\xBA";  // ║
    const string LT = "\xCC";  // ╠
    const string RT = "\xB9";  // ╣
    const string TT = "\xCB";  // ╦
    const string BT = "\xCA";  // ╩
    const string CR = "\xCE";  // ╬

    // Single border
    const string STL = "\xDA"; // ┌
    const string STR = "\xBF"; // ┐
    const string SBL = "\xC0"; // └
    const string SBR = "\xD9"; // ┘
    const string SH  = "\xC4"; // ─
    const string SV  = "\xB3"; // │
    const string SLT = "\xC3"; // ├
    const string SRT = "\xB4"; // ┤

    // Block characters for bars
    const string FULL  = "\xDB"; // █
    const string SHADE = "\xB0"; // ░
}

struct DashboardStats {
    int totalComplaints;
    int resolved;
    int inProgress;
    int pending;
    int escalated;

    int roadCount;
    int waterCount;
    int electricityCount;

    int emergencyCount;
    int normalCount;
    int lowCount;

    vector<string> recentActivity;  // last 5 actions
};

class ConsoleUI {
private:
    static int consoleWidth;
    static bool ansiEnabled;

public:
    // ── Initialization ──
    static void init();
    static void enableANSI();
    static void setConsoleSize(int width = 90, int height = 40);

    // ── Screen Management ──
    static void clearScreen();
    static void pause(const string& msg = "Press Enter to continue...");

    // ── Text Utilities ──
    static string centerText(const string& text, int width = 0);
    static string padRight(const string& text, int width);
    static string padLeft(const string& text, int width);
    static string repeat(const string& ch, int count);
    static string truncate(const string& text, int maxLen);

    // ══════════════════════════════════════════════════════
    //  DRAWING FUNCTIONS — The visual magic
    // ══════════════════════════════════════════════════════

    // Headers & Borders
    static void drawHeader();
    static void drawDoubleBar();
    static void drawSingleBar();
    static void drawSectionTitle(const string& title);

    // Main Menu
    static void drawMainMenu();

    // ── Login & Role-Based Menus ──
    static void drawLoginScreen();
    static void drawCitizenMenu(const string& citizenName);
    static void drawOfficerMenu(const string& officerName);
    static void drawSeniorOfficerMenu(const string& officerName);
    static void drawAdminMenu(const string& adminName);
    static void drawRatingStars(int rating);

    // Dashboard
    static void drawDashboard(const DashboardStats& stats);

    // Tables
    static void drawComplaintTableHeader();
    static void drawComplaintTableRow(int id, const string& desc, const string& dept,
                                       const string& severity, const string& status,
                                       const string& officer);
    static void drawComplaintTableFooter();

    // Detail View
    static void drawDetailHeader(int complaintId);
    static void drawDetailField(const string& label, const string& value);
    static void drawDetailSection(const string& title);
    static void drawDetailFooter();

    // Status Messages
    static void drawSuccess(const string& msg);
    static void drawError(const string& msg);
    static void drawWarning(const string& msg);
    static void drawInfo(const string& msg);

    // Progress & Animation
    static void drawProgressBar(int current, int total, int barWidth = 30);
    static void drawLoadingBar(const string& task, int durationMs = 1500);
    static void typeWriter(const string& text, int delayMs = 30);
    static void drawEmailAnimation(const string& recipient);
    static void drawEscalationAlert(int complaintId, int newLevel);

    // Bar Chart
    static void drawBarChart(const string& label, int value, int maxValue,
                              const string& color, int barWidth = 20);

    // Input Helpers
    static int getMenuChoice(int min, int max);
    static string getInput(const string& prompt);
    static int getIntInput(const string& prompt);
    static string getChoiceInput(const string& prompt, const vector<string>& options);

    // ── Input Validation Helpers ──
    static bool validateName(const string& name);
    static bool validatePhone(const string& phone);
    static bool validateEmail(const string& email);
    static string getValidatedInput(const string& prompt, const string& errorMsg,
                                     function<bool(const string&)> validator);

    // Splash Screen
    static void drawSplashScreen();

    // Getters
    static inline int getConsoleWidth() { return consoleWidth; }
    static inline bool isANSIEnabled() { return ansiEnabled; }

    // Get current time as formatted string
    static string getCurrentTime();
    static string getCurrentDate();
};
