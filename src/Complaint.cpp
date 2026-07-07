#include "../include/Complaint.h"
#include <iomanip>
#include <sstream>

// ══════════════════════════════════════════════════════════
//  Static Member Initialization
// ══════════════════════════════════════════════════════════
int Complaint::nextId = 1;

// ══════════════════════════════════════════════════════════
//  CONSTRUCTORS
// ══════════════════════════════════════════════════════════

// Default Constructor
Complaint::Complaint()
    : complaintId(0), description(""), photoPath(""),
      createdTime(time(0)), status("Pending"), severity("Normal"),
      citizenName(""), citizenContact(""), citizenEmail(""),
      isAssigned(false), escalationLevel(0),
      resolutionProof(""), citizenAccepted(false),
      resolutionSubmitted(false), citizenRating(0), dateOfSubmission("") {}

// Parameterized Constructor (auto-assigns ID via static nextId)
Complaint::Complaint(const string& desc, const string& photo, const string& sev,
                     const string& citName, const string& citContact, const string& citEmail)
    : complaintId(nextId++), description(desc), photoPath(photo),
      createdTime(time(0)), status("Pending"), severity(sev),
      citizenName(citName), citizenContact(citContact), citizenEmail(citEmail),
      isAssigned(false), escalationLevel(0),
      resolutionProof(""), citizenAccepted(false),
      resolutionSubmitted(false), citizenRating(0) {
    // Set formatted date of submission
    struct tm* timeInfo = localtime(&createdTime);
    char buffer[80];
    strftime(buffer, 80, "%d-%b-%Y", timeInfo);
    dateOfSubmission = string(buffer);
}

// Copy Constructor
Complaint::Complaint(const Complaint& other)
    : complaintId(other.complaintId), description(other.description),
      photoPath(other.photoPath), createdTime(other.createdTime),
      status(other.status), severity(other.severity),
      citizenName(other.citizenName), citizenContact(other.citizenContact),
      citizenEmail(other.citizenEmail), assignedOfficer(other.assignedOfficer),
      isAssigned(other.isAssigned), escalationLevel(other.escalationLevel),
      resolutionProof(other.resolutionProof), citizenAccepted(other.citizenAccepted),
      resolutionSubmitted(other.resolutionSubmitted), citizenRating(other.citizenRating),
      dateOfSubmission(other.dateOfSubmission) {}

// Virtual Destructor
Complaint::~Complaint() {}

// ══════════════════════════════════════════════════════════
//  VIRTUAL: Forward complaint to assigned officer
//  Base implementation prints notification; derived classes
//  override to add department-specific behavior + tracking
// ══════════════════════════════════════════════════════════
void Complaint::forwardToOfficer() {
    if (!isAssigned) {
        cout << "  [!] No officer assigned to this complaint." << endl;
        return;
    }
    cout << "\n  ================================================" << endl;
    cout << "  EMAIL NOTIFICATION" << endl;
    cout << "  ================================================" << endl;
    cout << "  To      : " << assignedOfficer.getEmail() << endl;
    cout << "  Subject : Complaint #" << complaintId << " - " << getDepartment() << endl;
    cout << "  Body    : " << description << endl;
    cout << "  Photo   : " << photoPath << endl;
    cout << "  Severity: " << severity << endl;
    cout << "  Filed By: " << citizenName << " (" << citizenContact << ")" << endl;
    cout << "  Status  : Forwarded Successfully" << endl;
    cout << "  ================================================\n" << endl;
}

// ══════════════════════════════════════════════════════════
//  ESCALATION LOGIC
// ══════════════════════════════════════════════════════════

// Mark complaint as escalated
void Complaint::escalateComplaint() {
    escalationLevel++;
    status = "Escalated";
    cout << "\n  [!] Complaint #" << complaintId << " has been ESCALATED (Level "
         << escalationLevel << ")" << endl;
    cout << "  [>] Requires reassignment to higher authority." << endl;
}

// Check if escalation deadline has passed
bool Complaint::checkEscalation() const {
    if (status == "Resolved") return false;
    double secondsPassed = getSecondsPassed();
    int threshold = getEscalationThreshold();
    return (secondsPassed >= threshold);
}

// Seconds elapsed since complaint was created
double Complaint::getSecondsPassed() const {
    return difftime(time(0), createdTime);
}

// Get escalation threshold in seconds
// Demo-friendly timings (for live demonstration):
//   Emergency : 30 seconds  (represents 12 hours in real system)
//   Normal    : 60 seconds  (represents 24 hours)
//   Low       : 120 seconds (represents 48 hours)
int Complaint::getEscalationThreshold() const {
    if (severity == "Emergency") return 30;
    if (severity == "Normal") return 60;
    return 120;     // Low priority
}

// ══════════════════════════════════════════════════════════
//  PROOF OF RESOLUTION
// ══════════════════════════════════════════════════════════

void Complaint::submitResolutionProof(const string& proof) {
    this->resolutionProof = proof;
    this->resolutionSubmitted = true;
    this->status = "Resolved";
}

void Complaint::acceptResolution() {
    this->citizenAccepted = true;
    this->status = "Resolved";     // stays resolved, case closed
}

void Complaint::rejectResolution() {
    this->citizenAccepted = false;
    this->resolutionSubmitted = false;
    this->resolutionProof = "";
    this->status = "In Progress";  // goes back for officer to rework
}

// ══════════════════════════════════════════════════════════
//  RATING SYSTEM
// ══════════════════════════════════════════════════════════

void Complaint::setRating(int rating) {
    if (rating >= 1 && rating <= 5) {
        this->citizenRating = rating;
    }
}

// ══════════════════════════════════════════════════════════
//  OPERATOR OVERLOADING
// ══════════════════════════════════════════════════════════

// Unary prefix ++ : Increase severity level
//   Low → Normal → Emergency (Emergency stays Emergency)
Complaint& Complaint::operator++() {
    if (this->severity == "Low") {
        this->severity = "Normal";
    } else if (this->severity == "Normal") {
        this->severity = "Emergency";
    }
    // Emergency is max — no change
    return *this;
}

// Binary == : Compare complaints by ID
bool Complaint::operator==(const Complaint& other) const {
    return this->complaintId == other.complaintId;
}

// Stream insertion operator <<
ostream& operator<<(ostream& os, const Complaint& c) {
    os << "#" << c.complaintId
       << " | " << c.getDepartment()
       << " | " << c.severity
       << " | " << c.status
       << " | " << c.description.substr(0, 40);
    if (c.description.length() > 40) os << "...";
    return os;
}

// ══════════════════════════════════════════════════════════
//  FRIEND FUNCTION — Full Report (accesses private members)
// ══════════════════════════════════════════════════════════
void displayFullReport(const Complaint& c) {
    cout << "\n  ========================================================" << endl;
    cout << "            COMPLAINT REPORT  #" << c.complaintId << endl;
    cout << "  ========================================================" << endl;
    cout << "  Department    : " << c.getDepartment() << endl;
    cout << "  Severity      : " << c.severity << endl;
    cout << "  Status        : " << c.status << endl;
    cout << "  Escalation Lvl: " << c.escalationLevel << endl;
    cout << "  Filed On      : " << c.getFormattedTime() << endl;
    cout << "  Date          : " << c.dateOfSubmission << endl;
    cout << "  Deadline      : " << c.getDeadlineTime() << endl;
    cout << "  Time Elapsed  : " << (int)c.getSecondsPassed() << " seconds" << endl;
    cout << "  --------------------------------------------------------" << endl;
    cout << "  DESCRIPTION:" << endl;
    cout << "    " << c.description << endl;
    cout << "  --------------------------------------------------------" << endl;
    cout << "  CITIZEN INFO:" << endl;
    cout << "    Name    : " << c.citizenName << endl;
    cout << "    Contact : " << c.citizenContact << endl;
    cout << "    Email   : " << c.citizenEmail << endl;
    cout << "  --------------------------------------------------------" << endl;
    cout << "  PHOTO EVIDENCE:" << endl;
    cout << "    " << (c.photoPath.empty() ? "No photo attached" : c.photoPath) << endl;
    cout << "  --------------------------------------------------------" << endl;
    if (c.isAssigned) {
        cout << "  ASSIGNED OFFICER:" << endl;
        cout << "    " << c.assignedOfficer << endl;
    } else {
        cout << "  OFFICER: Not yet assigned" << endl;
    }
    cout << "  --------------------------------------------------------" << endl;
    // Resolution proof info
    if (c.resolutionSubmitted) {
        cout << "  RESOLUTION PROOF:" << endl;
        cout << "    " << c.resolutionProof << endl;
        cout << "  Citizen Accepted: " << (c.citizenAccepted ? "Yes" : "Pending") << endl;
    }
    if (c.citizenRating > 0) {
        cout << "  CITIZEN RATING: ";
        for (int i = 0; i < c.citizenRating; i++) cout << "*";
        for (int i = c.citizenRating; i < 5; i++) cout << ".";
        cout << " (" << c.citizenRating << "/5)" << endl;
    }
    cout << "  --------------------------------------------------------" << endl;
    cout << "  TYPE-SPECIFIC INFO:" << endl;
    c.displayTypeSpecificInfo();    // virtual dispatch to derived class
    cout << "  ========================================================\n" << endl;
}

// ══════════════════════════════════════════════════════════
//  STATUS TIMELINE
// ══════════════════════════════════════════════════════════

void Complaint::addStatusChange(const string& oldStatus, const string& newStatus) {
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char buf[80];
    strftime(buf, 80, "[%d-%b-%Y %H:%M:%S]", t);
    string entry = string(buf) + " " + oldStatus + " -> " + newStatus;
    statusTimeline.push_back(entry);
}

// ══════════════════════════════════════════════════════════
//  UTILITY FUNCTIONS
// ══════════════════════════════════════════════════════════

// Format creation time as readable string
string Complaint::getFormattedTime() const {
    struct tm* timeInfo = localtime(&createdTime);
    char buffer[80];
    strftime(buffer, 80, "%d-%b-%Y %H:%M:%S", timeInfo);
    return string(buffer);
}

// Format deadline time as readable string
string Complaint::getDeadlineTime() const {
    time_t deadline = createdTime + getEscalationThreshold();
    struct tm* timeInfo = localtime(&deadline);
    char buffer[80];
    strftime(buffer, 80, "%d-%b-%Y %H:%M:%S", timeInfo);
    return string(buffer);
}
