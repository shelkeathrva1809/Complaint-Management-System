#include "../include/RoadComplaint.h"

// ══════════════════════════════════════════════════════════
//  Constructor Behavior in Inheritance:
//  Each constructor explicitly calls BOTH base class ctors
//    - Complaint(...)  → initializes complaint data
//    - Trackable()     → initializes tracking history
// ══════════════════════════════════════════════════════════

// Default Constructor
RoadComplaint::RoadComplaint()
    : Complaint(), Trackable(), location(""), wardNumber(0) {
    addEvent("Road complaint created (default)");
}

// Parameterized Constructor
RoadComplaint::RoadComplaint(const string& desc, const string& photo, const string& sev,
                             const string& citName, const string& citContact,
                             const string& citEmail,
                             const string& loc, int ward)
    : Complaint(desc, photo, sev, citName, citContact, citEmail),
      Trackable(), location(loc), wardNumber(ward) {
    addEvent("Road complaint #" + to_string(complaintId) + " registered | Location: " + loc + " | Ward: " + to_string(ward));
}

// Copy Constructor
RoadComplaint::RoadComplaint(const RoadComplaint& other)
    : Complaint(other), Trackable(other),
      location(other.location), wardNumber(other.wardNumber) {
    addEvent("Road complaint #" + to_string(complaintId) + " copied");
}

// Destructor
RoadComplaint::~RoadComplaint() {}

// ══════════════════════════════════════════════════════════
//  FUNCTION OVERRIDING — Runtime Polymorphism
// ══════════════════════════════════════════════════════════

// Override: Returns department for this complaint type
string RoadComplaint::getDepartment() const {
    return "Public Works Department (PWD)";
}

// Override: Display road-specific details
void RoadComplaint::displayTypeSpecificInfo() const {
    cout << "    Complaint Type : Road / Infrastructure" << endl;
    cout << "    Location       : " << location << endl;
    cout << "    Ward Number    : " << wardNumber << endl;
}

// Override: Forward to officer + add tracking event
void RoadComplaint::forwardToOfficer() {
    Complaint::forwardToOfficer();          // call base class version
    if (getIsAssigned()) {
        addEvent("Complaint forwarded to PWD officer: " + getAssignedOfficer().getEmail());
    }
}
