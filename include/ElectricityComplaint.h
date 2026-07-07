#pragma once
#include "Complaint.h"
#include "Trackable.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  ElectricityComplaint — Derived from Complaint + Trackable
//  Demonstrates: Hierarchical + Multiple Inheritance,
//                Function Overriding, Runtime Polymorphism
// ══════════════════════════════════════════════════════════

class ElectricityComplaint : public Complaint, public Trackable {
private:
    string transformerId;       // transformer/substation identifier
    int affectedHomes;          // number of homes affected

public:
    // ── Constructors ──
    ElectricityComplaint();
    ElectricityComplaint(const string& desc, const string& photo, const string& sev,
                         const string& citName, const string& citContact, const string& citEmail,
                         const string& transformerId, int affectedHomes);
    ElectricityComplaint(const ElectricityComplaint& other);

    // ── Destructor ──
    ~ElectricityComplaint();

    // ── Override Pure Virtual Functions ──
    string getDepartment() const override;
    void displayTypeSpecificInfo() const override;

    // ── Override Virtual Function ──
    void forwardToOfficer() override;

    // ── Getters (Inline) ──
    inline string getTransformerId() const { return transformerId; }
    inline int getAffectedHomes() const { return affectedHomes; }

    // ── Setters (this pointer) ──
    void setTransformerId(const string& t) { this->transformerId = t; }
    void setAffectedHomes(int h) { this->affectedHomes = h; }
};
