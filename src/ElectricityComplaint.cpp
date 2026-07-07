#include "../include/ElectricityComplaint.h"

// ══════════════════════════════════════════════════════════
//  Constructor Behavior in Inheritance:
//  Calls both Complaint and Trackable base constructors
// ══════════════════════════════════════════════════════════

// Default Constructor
ElectricityComplaint::ElectricityComplaint()
    : Complaint(), Trackable(), transformerId(""), affectedHomes(0) {
    addEvent("Electricity complaint created (default)");
}

// Parameterized Constructor
ElectricityComplaint::ElectricityComplaint(const string& desc, const string& photo,
                                           const string& sev,
                                           const string& citName, const string& citContact,
                                           const string& citEmail,
                                           const string& tid, int homes)
    : Complaint(desc, photo, sev, citName, citContact, citEmail),
      Trackable(), transformerId(tid), affectedHomes(homes) {
    addEvent("Electricity complaint #" + to_string(complaintId) + " registered | Transformer: " + tid + " | Homes affected: " + to_string(homes));
}

// Copy Constructor
ElectricityComplaint::ElectricityComplaint(const ElectricityComplaint& other)
    : Complaint(other), Trackable(other),
      transformerId(other.transformerId), affectedHomes(other.affectedHomes) {
    addEvent("Electricity complaint #" + to_string(complaintId) + " copied");
}

// Destructor
ElectricityComplaint::~ElectricityComplaint() {}

// ══════════════════════════════════════════════════════════
//  FUNCTION OVERRIDING
// ══════════════════════════════════════════════════════════

string ElectricityComplaint::getDepartment() const {
    return "Electricity Board";
}

void ElectricityComplaint::displayTypeSpecificInfo() const {
    cout << "    Complaint Type   : Electricity / Power" << endl;
    cout << "    Transformer ID   : " << transformerId << endl;
    cout << "    Affected Homes   : " << affectedHomes << endl;
}

void ElectricityComplaint::forwardToOfficer() {
    Complaint::forwardToOfficer();
    if (getIsAssigned()) {
        addEvent("Complaint forwarded to Electricity Board officer: " + getAssignedOfficer().getEmail());
    }
}
