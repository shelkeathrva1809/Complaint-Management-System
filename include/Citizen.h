#pragma once
#include <string>
#include <iostream>
#include "User.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  Citizen Class — Derived from User
//  Demonstrates: Single Inheritance, Encapsulation,
//                Static Data Members, Constructor Overloading,
//                Inline Functions, this Pointer,
//                Operator Overloading (== and <<)
// ══════════════════════════════════════════════════════════

class Citizen : public User {
private:
    int citizenId;
    string address;

    static int nextCitizenId;       // Auto-increment ID generator

public:
    // ── Constructors ──
    Citizen();                                                                          // Default
    Citizen(const string& name, const string& phone,
            const string& email, const string& address);                                // Parameterized (no login)
    Citizen(const string& username, const string& password,
            const string& name, const string& phone,
            const string& email, const string& address);                                // Full with login
    Citizen(const Citizen& other);                                                      // Copy

    // ── Destructor ──
    ~Citizen();

    // ── Role Override (from User) ──
    Role getRole() const override { return Role::CITIZEN; }
    string getRoleString() const override { return "Citizen"; }

    // ── Getters (Inline) ──
    inline int getId() const { return citizenId; }
    inline string getAddress() const { return address; }

    // ── Setters (using this pointer) ──
    void setAddress(const string& a) { this->address = a; }

    // ── Operator Overloading ──
    bool operator==(const Citizen& other) const;                    // Binary ==
    friend ostream& operator<<(ostream& os, const Citizen& c);     // Stream <<

    // ── Static ──
    static int getTotalCitizens() { return nextCitizenId - 1; }
};
