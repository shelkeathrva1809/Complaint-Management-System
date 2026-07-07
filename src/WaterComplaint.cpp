#include "../include/WaterComplaint.h"

// ══════════════════════════════════════════════════════════
//  Constructor Behavior in Inheritance:
//  Calls both Complaint and Trackable base constructors
// ══════════════════════════════════════════════════════════

// Default Constructor
WaterComplaint::WaterComplaint()
    : Complaint(), Trackable(), pipelineId(""), area("") {
    addEvent("Water complaint created (default)");
}

// Parameterized Constructor
WaterComplaint::WaterComplaint(const string& desc, const string& photo, const string& sev,
                               const string& citName, const string& citContact,
                               const string& citEmail,
                               const string& pid, const string& area)
    : Complaint(desc, photo, sev, citName, citContact, citEmail),
      Trackable(), pipelineId(pid), area(area) {
    addEvent("Water complaint #" + to_string(complaintId) + " registered | Pipeline: " + pid + " | Area: " + area);
}

// Copy Constructor
WaterComplaint::WaterComplaint(const WaterComplaint& other)
    : Complaint(other), Trackable(other),
      pipelineId(other.pipelineId), area(other.area) {
    addEvent("Water complaint #" + to_string(complaintId) + " copied");
}

// Destructor
WaterComplaint::~WaterComplaint() {}

// ══════════════════════════════════════════════════════════
//  FUNCTION OVERRIDING
// ══════════════════════════════════════════════════════════

string WaterComplaint::getDepartment() const {
    return "Water Supply Department";
}

void WaterComplaint::displayTypeSpecificInfo() const {
    cout << "    Complaint Type : Water Supply" << endl;
    cout << "    Pipeline ID    : " << pipelineId << endl;
    cout << "    Affected Area  : " << area << endl;
}

void WaterComplaint::forwardToOfficer() {
    Complaint::forwardToOfficer();
    if (getIsAssigned()) {
        addEvent("Complaint forwarded to Water Dept officer: " + getAssignedOfficer().getEmail());
    }
}
