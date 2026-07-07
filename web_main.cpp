// ══════════════════════════════════════════════════════════
//  SPCMS Web Server — Entry Point
//  Smart Public Complaint Management System
//  Full-Stack C++ OOP Application
// ══════════════════════════════════════════════════════════

#include <iostream>
#include <cstdlib>
#include "include/Database.h"
#include "include/WebServer.h"

#ifdef _WIN32
#include <direct.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define MKDIR(d) mkdir(d, 0755)
#endif

int main() {
    std::cout << R"(
  ╔══════════════════════════════════════════════════╗
  ║   SPCMS — Smart Public Complaint Management     ║
  ║   Full-Stack Web Application (C++ OOP)           ║
  ║   Version 2.0                                    ║
  ╚══════════════════════════════════════════════════╝
    )" << std::endl;

    try {
        // Ensure data directory exists
        MKDIR("data");
        MKDIR("uploads");

        // Initialize SQLite database (Singleton)
        std::cout << "  Initializing database..." << std::endl;
        Database& db = Database::getInstance();
        db.initialize("data/spcms.db");
        std::cout << "  Database ready. (" << db.getComplaintCount() << " complaints)" << std::endl;

        // Start web server (Composition: WebServer uses Database + AuthManager)
        WebServer webServer(8080);
        webServer.start();

    } catch (const std::exception& e) {
        std::cerr << "\n  [ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
