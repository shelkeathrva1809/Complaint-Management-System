#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "User.h"
using namespace std;

// Forward declarations
class ComplaintSystem;

// ══════════════════════════════════════════════════════════
//  Admin — Derived from User
//  Demonstrates:
//    - Inheritance (from User)
//    - Encapsulation (analytics methods)
//    - Object Composition (works with ComplaintSystem)
//    - Static Methods (utility functions)
//    - Friend access pattern (references ComplaintSystem)
// ══════════════════════════════════════════════════════════

class Admin : public User {
private:
    string adminLevel;          // "Super Admin", "Moderator", etc.

public:
    // ── Constructors ──
    Admin();
    Admin(const string& username, const string& password,
          const string& name, const string& email,
          const string& phone, const string& adminLevel = "Super Admin");
    Admin(const Admin& other);

    // ── Destructor ──
    ~Admin();

    // ── Role Override ──
    Role getRole() const override { return Role::ADMIN; }
    string getRoleString() const override { return "Admin"; }

    // ══════════════════════════════════════════════════════
    //  ADMIN DASHBOARD ANALYTICS
    //  These methods take ComplaintSystem reference to
    //  compute and display analytics
    // ══════════════════════════════════════════════════════

    // View total number of complaints
    void viewTotalComplaints(const ComplaintSystem& system) const;

    // View department-wise complaint count
    void viewDepartmentWiseCount(const ComplaintSystem& system) const;

    // View Pending vs Resolved breakdown
    void viewPendingVsResolved(const ComplaintSystem& system) const;

    // Identify worst performing officers/areas
    void viewWorstPerformingOfficers(const ComplaintSystem& system) const;

    // View escalated complaints
    void viewEscalatedComplaints(const ComplaintSystem& system) const;

    // Full admin dashboard (combines all above)
    void showAdminDashboard(const ComplaintSystem& system) const;

    // ── Getters ──
    inline string getAdminLevel() const { return adminLevel; }

    // ── Setters ──
    void setAdminLevel(const string& l) { this->adminLevel = l; }
};
