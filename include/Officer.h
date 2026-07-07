#pragma once
#include <string>
#include <iostream>
#include "User.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  Officer Class — Derived from User
//  Demonstrates: Inheritance, Encapsulation, Access Specifiers,
//                Constructor Overloading, Inline Functions,
//                this Pointer, Operator Overloading,
//                Rating System (aggregation)
// ══════════════════════════════════════════════════════════

class Officer : public User {
private:
    int officerId;
    string department;
    int level;              // 1 = Field Officer, 2 = Supervisor, 3 = Commissioner

    // ── Rating System ──
    double totalRating;     // sum of all ratings received
    int ratingCount;        // number of ratings received

public:
    // ── Constructors ──
    Officer();                                                                          // Default
    Officer(int id, const string& name, const string& email,
            const string& dept, int level);                                             // Parameterized (backwards compat)
    Officer(int id, const string& username, const string& password,
            const string& name, const string& email,
            const string& dept, int level);                                             // Full with login
    Officer(const Officer& other);                                                      // Copy

    // ── Destructor ──
    ~Officer();

    // ── Role Override (from User) ──
    virtual Role getRole() const override { return Role::OFFICER; }
    virtual string getRoleString() const override { return "Officer"; }

    // ── Getters (Inline) ──
    inline int getId() const { return officerId; }
    inline string getDepartment() const { return department; }
    inline int getLevel() const { return level; }

    // ── Rating System ──
    void addRating(int rating);                          // Add a rating (1-5)
    inline double getAverageRating() const {
        return (ratingCount > 0) ? (totalRating / ratingCount) : 0.0;
    }
    inline int getRatingCount() const { return ratingCount; }
    inline double getTotalRating() const { return totalRating; }

    // Get readable level title
    string getLevelTitle() const;

    // ── Setters (using this pointer) ──
    void setId(int id) { this->officerId = id; }
    void setDepartment(const string& d) { this->department = d; }
    void setLevel(int l) { this->level = l; }

    // ── Operator Overloading ──
    bool operator==(const Officer& other) const;                    // Binary ==
    friend ostream& operator<<(ostream& os, const Officer& o);     // Stream <<
};
