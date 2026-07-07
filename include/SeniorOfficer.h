#pragma once
#include <string>
#include <iostream>
#include "Officer.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  SeniorOfficer — Derived from Officer (Multilevel Inheritance)
//  Demonstrates:
//    - Multilevel Inheritance (User -> Officer -> SeniorOfficer)
//    - Function Overriding (getRole, getRoleString)
//    - Constructor Behavior in Inheritance
//    - Encapsulation (additional responsibilities)
// ══════════════════════════════════════════════════════════

class SeniorOfficer : public Officer {
private:
    int escalatedHandled;       // count of escalated complaints handled
    string specialization;      // area of specialization

public:
    // ── Constructors ──
    SeniorOfficer();
    SeniorOfficer(int id, const string& name, const string& email,
                  const string& dept, int level,
                  const string& specialization = "General");
    SeniorOfficer(int id, const string& username, const string& password,
                  const string& name, const string& email,
                  const string& dept, int level,
                  const string& specialization = "General");
    SeniorOfficer(const SeniorOfficer& other);

    // ── Destructor ──
    ~SeniorOfficer();

    // ── Role Override ──
    Role getRole() const override { return Role::SENIOR_OFFICER; }
    string getRoleString() const override { return "Senior Officer"; }

    // ── Senior Officer Features ──
    void handleEscalatedComplaint();
    void incrementHandled() { escalatedHandled++; }

    // ── Getters ──
    inline int getEscalatedHandled() const { return escalatedHandled; }
    inline string getSpecialization() const { return specialization; }

    // ── Setters ──
    void setSpecialization(const string& s) { this->specialization = s; }
};
