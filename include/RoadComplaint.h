#pragma once
#include "Complaint.h"
#include "Trackable.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  RoadComplaint — Derived from Complaint + Trackable
//  Demonstrates:
//    - Single Inheritance (from Complaint)
//    - Multiple Inheritance (from Complaint + Trackable)
//    - Hierarchical Inheritance (one of 3 types from Complaint)
//    - Function Overriding (getDepartment, displayTypeSpecificInfo)
//    - Constructor Behavior in Inheritance (calls base ctors)
//    - Runtime Polymorphism (virtual dispatch)
// ══════════════════════════════════════════════════════════

class RoadComplaint : public Complaint, public Trackable {
private:
    string location;        // road name or area
    int wardNumber;         // municipal ward number

public:
    // ── Constructors (call base class constructors) ──
    RoadComplaint();
    RoadComplaint(const string& desc, const string& photo, const string& sev,
                  const string& citName, const string& citContact, const string& citEmail,
                  const string& location, int wardNumber);
    RoadComplaint(const RoadComplaint& other);

    // ── Destructor ──
    ~RoadComplaint();

    // ── Override Pure Virtual Functions ──
    string getDepartment() const override;
    void displayTypeSpecificInfo() const override;

    // ── Override Virtual Function ──
    void forwardToOfficer() override;

    // ── Getters (Inline) ──
    inline string getLocation() const { return location; }
    inline int getWardNumber() const { return wardNumber; }

    // ── Setters (this pointer) ──
    void setLocation(const string& l) { this->location = l; }
    void setWardNumber(int w) { this->wardNumber = w; }
};
