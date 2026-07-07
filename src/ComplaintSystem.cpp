#include "../include/ComplaintSystem.h"
#include "../include/EscalationManager.h"
#include "../include/ConsoleUI.h"
#include "../include/DashboardWriter.h"
#include <algorithm>

// ══════════════════════════════════════════════════════════
//  Static Member Initialization
// ══════════════════════════════════════════════════════════
int ComplaintSystem::instanceCount = 0;

// ══════════════════════════════════════════════════════════
//  CONSTRUCTORS & DESTRUCTOR
// ══════════════════════════════════════════════════════════

// Default Constructor
ComplaintSystem::ComplaintSystem() {
    instanceCount++;
    logActivity("System initialized");
}

// Copy Constructor — Deep copy of complaints (dynamic memory)
ComplaintSystem::ComplaintSystem(const ComplaintSystem& other)
    : officers(other.officers), seniorOfficers(other.seniorOfficers),
      citizens(other.citizens), admins(other.admins),
      emailLog(other.emailLog), activityLog(other.activityLog) {
    // Deep copy each complaint (polymorphic cloning via type detection)
    for (const auto& c : other.complaints) {
        string dept = c->getDepartment();
        if (dept.find("PWD") != string::npos || dept.find("Public") != string::npos) {
            complaints.push_back(new RoadComplaint(dynamic_cast<const RoadComplaint&>(*c)));
        } else if (dept.find("Water") != string::npos) {
            complaints.push_back(new WaterComplaint(dynamic_cast<const WaterComplaint&>(*c)));
        } else if (dept.find("Elec") != string::npos) {
            complaints.push_back(new ElectricityComplaint(dynamic_cast<const ElectricityComplaint&>(*c)));
        }
    }
    instanceCount++;
}

// Destructor — Frees dynamically allocated Complaint objects
ComplaintSystem::~ComplaintSystem() {
    for (auto* c : complaints) {
        delete c;
    }
    complaints.clear();
    instanceCount--;
}

// Assignment Operator — Deep copy (Rule of Three)
ComplaintSystem& ComplaintSystem::operator=(const ComplaintSystem& other) {
    if (this != &other) {
        // Free existing complaints
        for (auto* c : complaints) {
            delete c;
        }
        complaints.clear();

        // Deep copy complaints
        for (const auto& c : other.complaints) {
            string dept = c->getDepartment();
            if (dept.find("PWD") != string::npos || dept.find("Public") != string::npos) {
                complaints.push_back(new RoadComplaint(dynamic_cast<const RoadComplaint&>(*c)));
            } else if (dept.find("Water") != string::npos) {
                complaints.push_back(new WaterComplaint(dynamic_cast<const WaterComplaint&>(*c)));
            } else if (dept.find("Elec") != string::npos) {
                complaints.push_back(new ElectricityComplaint(dynamic_cast<const ElectricityComplaint&>(*c)));
            }
        }

        officers = other.officers;
        seniorOfficers = other.seniorOfficers;
        citizens = other.citizens;
        admins = other.admins;
        emailLog = other.emailLog;
        activityLog = other.activityLog;
    }
    return *this;
}

// ══════════════════════════════════════════════════════════
//  PRIVATE HELPERS
// ══════════════════════════════════════════════════════════

Officer* ComplaintSystem::findOfficerByDepartment(const string& dept, int level) {
    for (auto& o : officers) {
        bool deptMatch = false;
        if (dept.find("PWD") != string::npos || dept.find("Public") != string::npos || dept.find("Road") != string::npos) {
            deptMatch = (o.getDepartment().find("PWD") != string::npos ||
                         o.getDepartment().find("Public") != string::npos ||
                         o.getDepartment() == "PWD");
        } else if (dept.find("Water") != string::npos) {
            deptMatch = (o.getDepartment().find("Water") != string::npos);
        } else if (dept.find("Elec") != string::npos) {
            deptMatch = (o.getDepartment().find("Elec") != string::npos);
        }
        // Municipal Commissioner matches any department at level 3
        if (o.getDepartment().find("Municipal") != string::npos && level >= 3) {
            deptMatch = true;
        }

        if (deptMatch && o.getLevel() == level) {
            return &o;
        }
    }
    return nullptr;
}

Officer* ComplaintSystem::findOfficerById(int id) {
    for (auto& o : officers) {
        if (o.getId() == id) return &o;
    }
    return nullptr;
}

void ComplaintSystem::logActivity(const string& text) {
    string timeStr = "[" + ConsoleUI::getCurrentTime() + "]";
    ActivityEntry entry = {timeStr + " " + text, ConsoleUI::getCurrentTime()};
    activityLog.insert(activityLog.begin(), entry);  // newest first
    if (activityLog.size() > 50) activityLog.pop_back();  // keep last 50
}

void ComplaintSystem::logEmail(const string& to, const string& subject, const string& status) {
    EmailLogEntry entry = {to, subject, ConsoleUI::getCurrentDate() + " " + ConsoleUI::getCurrentTime(), status};
    emailLog.push_back(entry);
}

// ══════════════════════════════════════════════════════════
//  AUTHENTICATION
// ══════════════════════════════════════════════════════════

Citizen* ComplaintSystem::authenticateCitizen(const string& username, const string& password) {
    for (auto& c : citizens) {
        if (c.authenticate(username, password)) {
            return &c;
        }
    }
    return nullptr;
}

Officer* ComplaintSystem::authenticateOfficer(const string& username, const string& password) {
    for (auto& o : officers) {
        if (o.authenticate(username, password)) {
            return &o;
        }
    }
    return nullptr;
}

SeniorOfficer* ComplaintSystem::authenticateSeniorOfficer(const string& username, const string& password) {
    for (auto& so : seniorOfficers) {
        if (so.authenticate(username, password)) {
            return &so;
        }
    }
    return nullptr;
}

Admin* ComplaintSystem::authenticateAdmin(const string& username, const string& password) {
    for (auto& a : admins) {
        if (a.authenticate(username, password)) {
            return &a;
        }
    }
    return nullptr;
}

// ══════════════════════════════════════════════════════════
//  OFFICER MANAGEMENT
// ══════════════════════════════════════════════════════════

void ComplaintSystem::addOfficer(const Officer& officer) {
    officers.push_back(officer);
}

void ComplaintSystem::addOfficers(const vector<Officer>& officerList) {
    for (const auto& o : officerList) {
        officers.push_back(o);
    }
}

void ComplaintSystem::addSeniorOfficer(const SeniorOfficer& officer) {
    seniorOfficers.push_back(officer);
    // Also add to officers vector for complaint assignment compatibility
    officers.push_back(officer);
}

void ComplaintSystem::viewOfficers() const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("OFFICER DIRECTORY");

    if (officers.empty()) {
        ConsoleUI::drawWarning("No officers registered in the system.");
        return;
    }

    // Table header
    cout << endl;
    cout << "  " << Color::CYAN
         << Box::TL << ConsoleUI::repeat(Box::H, 6) << Box::TT
         << ConsoleUI::repeat(Box::H, 22) << Box::TT
         << ConsoleUI::repeat(Box::H, 28) << Box::TT
         << ConsoleUI::repeat(Box::H, 18) << Box::TT
         << ConsoleUI::repeat(Box::H, 8) << Box::TR
         << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " ID   " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Name                 " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Email                      " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Department       " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Level  " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN
         << Box::LT << ConsoleUI::repeat(Box::H, 6) << Box::CR
         << ConsoleUI::repeat(Box::H, 22) << Box::CR
         << ConsoleUI::repeat(Box::H, 28) << Box::CR
         << ConsoleUI::repeat(Box::H, 18) << Box::CR
         << ConsoleUI::repeat(Box::H, 8) << Box::RT
         << Color::RESET << endl;

    for (const auto& o : officers) {
        string levelColor = Color::WHITE;
        if (o.getLevel() == 3) levelColor = Color::RED;
        else if (o.getLevel() == 2) levelColor = Color::YELLOW;
        else levelColor = Color::GREEN;

        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(to_string(o.getId()), 5)
             << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ConsoleUI::truncate(o.getName(), 20), 21)
             << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ConsoleUI::truncate(o.getEmail(), 26), 27)
             << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ConsoleUI::truncate(o.getDepartment(), 16), 17)
             << Color::CYAN << Box::V << Color::RESET
             << " " << levelColor << ConsoleUI::padRight("L" + to_string(o.getLevel()), 7) << Color::RESET
             << Color::CYAN << Box::V << Color::RESET << endl;
    }

    cout << "  " << Color::CYAN
         << Box::BL << ConsoleUI::repeat(Box::H, 6) << Box::BT
         << ConsoleUI::repeat(Box::H, 22) << Box::BT
         << ConsoleUI::repeat(Box::H, 28) << Box::BT
         << ConsoleUI::repeat(Box::H, 18) << Box::BT
         << ConsoleUI::repeat(Box::H, 8) << Box::BR
         << Color::RESET << endl;

    ConsoleUI::drawInfo("Total Officers: " + to_string(officers.size()));
}

// ══════════════════════════════════════════════════════════
//  CITIZEN MANAGEMENT
// ══════════════════════════════════════════════════════════

void ComplaintSystem::addCitizen(const Citizen& citizen) {
    citizens.push_back(citizen);
}

void ComplaintSystem::addAdmin(const Admin& admin) {
    admins.push_back(admin);
}

// ══════════════════════════════════════════════════════════
//  COMPLAINT REGISTRATION — Interactive Console
// ══════════════════════════════════════════════════════════

int ComplaintSystem::registerComplaint() {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("REGISTER NEW COMPLAINT");

    // 1. Choose complaint type
    string type = ConsoleUI::getChoiceInput("Select complaint type:", {"Road / Public Works", "Water Supply", "Electricity"});

    if (type.empty()) {
        throw InvalidInputException("Invalid complaint type selected");
    }

    // 2. Collect citizen info with strict validation
    ConsoleUI::drawSectionTitle("CITIZEN DETAILS");

    // Name: only letters and spaces, 2-50 chars
    string name = ConsoleUI::getValidatedInput(
        "Full Name",
        "Invalid name. Please enter only letters.",
        ConsoleUI::validateName
    );

    // Phone: exactly 10 digits, starts with 6-9
    string phone = ConsoleUI::getValidatedInput(
        "Phone Number",
        "Invalid phone number. Enter a 10-digit valid number.",
        ConsoleUI::validatePhone
    );

    // Email: valid format, allowed domains only, auto-lowercase
    string email = ConsoleUI::getValidatedInput(
        "Email Address",
        "Invalid email. Use gmail/yahoo/outlook only.",
        [](const string& val) -> bool {
            // Convert to lowercase for validation
            string lower = val;
            transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            return ConsoleUI::validateEmail(lower);
        }
    );
    // Store email as lowercase
    transform(email.begin(), email.end(), email.begin(), ::tolower);

    // Success confirmation
    ConsoleUI::drawSuccess("Citizen details verified successfully " + string(Icon::CHECK));

    return registerComplaintForCitizen(name, phone, email);
}

int ComplaintSystem::registerComplaintForCitizen(const string& citName, const string& citPhone, const string& citEmail) {
    // 3. Collect complaint info
    ConsoleUI::drawSectionTitle("COMPLAINT DETAILS");
    string desc = ConsoleUI::getInput("Description");
    string photo = ConsoleUI::getInput("Photo path (or press Enter to skip)");
    if (photo.empty()) photo = "No photo attached";

    string severity = ConsoleUI::getChoiceInput("Severity level:", {"Low", "Normal", "Emergency"});
    if (severity.empty()) severity = "Normal";

    // Choose complaint type
    string type = ConsoleUI::getChoiceInput("Select complaint type:", {"Road / Public Works", "Water Supply", "Electricity"});
    if (type.empty()) {
        throw InvalidInputException("Invalid complaint type selected");
    }

    // 4. Type-specific fields
    Complaint* newComplaint = nullptr;

    if (type.find("Road") != string::npos) {
        string location = ConsoleUI::getInput("Road/Location Name");
        int ward = ConsoleUI::getIntInput("Ward Number");
        if (ward < 0) ward = 1;
        newComplaint = new RoadComplaint(desc, photo, severity, citName, citPhone, citEmail, location, ward);
    } else if (type.find("Water") != string::npos) {
        string pipelineId = ConsoleUI::getInput("Pipeline ID (e.g., PL-1234)");
        string area = ConsoleUI::getInput("Affected Area/Sector");
        newComplaint = new WaterComplaint(desc, photo, severity, citName, citPhone, citEmail, pipelineId, area);
    } else if (type.find("Elec") != string::npos) {
        string transId = ConsoleUI::getInput("Transformer ID (e.g., TF-5678)");
        int homes = ConsoleUI::getIntInput("Number of affected homes");
        if (homes < 0) homes = 1;
        newComplaint = new ElectricityComplaint(desc, photo, severity, citName, citPhone, citEmail, transId, homes);
    }

    if (!newComplaint) {
        throw InvalidInputException("Failed to create complaint object");
    }

    // 5. Auto-assign officer (Level 1 from matching department)
    Officer* assignee = findOfficerByDepartment(newComplaint->getDepartment(), 1);
    if (assignee) {
        newComplaint->setAssignedOfficer(*assignee);
        ConsoleUI::drawSuccess("Auto-assigned to: " + assignee->getName() + " (" + assignee->getEmail() + ")");

        // Log email
        string subject = "Complaint #" + to_string(newComplaint->getId()) + " - " + newComplaint->getDepartment() + " " + severity;
        logEmail(assignee->getEmail(), subject, "Queued");

        // Show email animation
        ConsoleUI::drawEmailAnimation(assignee->getEmail());
    } else {
        ConsoleUI::drawWarning("No officer available for " + newComplaint->getDepartment() + ". Complaint is unassigned.");
    }

    // 6. Store complaint
    complaints.push_back(newComplaint);

    // 7. Log activity
    logActivity("[+] Complaint #" + to_string(newComplaint->getId()) + " registered - " +
                newComplaint->getDepartment() + " - " + severity);

    // 8. Register citizen if new
    Citizen newCitizen(citName, citPhone, citEmail, "");
    citizens.push_back(newCitizen);

    ConsoleUI::drawSuccess("Complaint #" + to_string(newComplaint->getId()) + " registered successfully!");
    return newComplaint->getId();
}

// ══════════════════════════════════════════════════════════
//  COMPLAINT REGISTRATION — Programmatic (for demo/test)
// ══════════════════════════════════════════════════════════

int ComplaintSystem::registerComplaintDirect(const string& type, const string& desc,
                                              const string& photo, const string& severity,
                                              const string& citName, const string& citContact,
                                              const string& citEmail,
                                              const string& extra1, const string& extra2) {
    Complaint* newComplaint = nullptr;

    if (type == "Road") {
        int ward = 1;
        try { ward = stoi(extra2); } catch (...) { ward = 1; }
        newComplaint = new RoadComplaint(desc, photo, severity, citName, citContact, citEmail, extra1, ward);
    } else if (type == "Water") {
        newComplaint = new WaterComplaint(desc, photo, severity, citName, citContact, citEmail, extra1, extra2);
    } else if (type == "Electricity") {
        int homes = 1;
        try { homes = stoi(extra2); } catch (...) { homes = 1; }
        newComplaint = new ElectricityComplaint(desc, photo, severity, citName, citContact, citEmail, extra1, homes);
    }

    if (!newComplaint) {
        throw InvalidInputException("Invalid complaint type: " + type);
    }

    // Auto-assign officer
    Officer* assignee = findOfficerByDepartment(newComplaint->getDepartment(), 1);
    if (assignee) {
        newComplaint->setAssignedOfficer(*assignee);
        string subject = "Complaint #" + to_string(newComplaint->getId()) + " - " + newComplaint->getDepartment() + " " + severity;
        logEmail(assignee->getEmail(), subject, "Sent");
    }

    complaints.push_back(newComplaint);
    logActivity("[+] Complaint #" + to_string(newComplaint->getId()) + " registered - " +
                newComplaint->getDepartment() + " - " + severity);

    return newComplaint->getId();
}

// ══════════════════════════════════════════════════════════
//  VIEW ALL COMPLAINTS
// ══════════════════════════════════════════════════════════

void ComplaintSystem::viewAllComplaints() const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("ALL COMPLAINTS");

    if (complaints.empty()) {
        ConsoleUI::drawWarning("No complaints registered in the system.");
        return;
    }

    ConsoleUI::drawComplaintTableHeader();
    for (const auto* c : complaints) {
        ConsoleUI::drawComplaintTableRow(
            c->getId(),
            c->getDescription(),
            c->getDepartment(),
            c->getSeverity(),
            c->getStatus(),
            c->getAssignedOfficer().getName()
        );
    }
    ConsoleUI::drawComplaintTableFooter();

    ConsoleUI::drawInfo("Total: " + to_string(complaints.size()) + " complaints");
}

// ══════════════════════════════════════════════════════════
//  SEARCH COMPLAINT
// ══════════════════════════════════════════════════════════

Complaint* ComplaintSystem::searchComplaint(int id) const {
    for (auto* c : complaints) {
        if (c->getId() == id) return c;
    }
    throw ComplaintNotFoundException(id);
}

// ══════════════════════════════════════════════════════════
//  UPDATE COMPLAINT STATUS
// ══════════════════════════════════════════════════════════

void ComplaintSystem::updateComplaintStatus(int id, const string& newStatus) {
    Complaint* c = searchComplaint(id);  // throws if not found

    string oldStatus = c->getStatus();
    c->addStatusChange(oldStatus, newStatus);
    c->setStatus(newStatus);

    logActivity("[*] Complaint #" + to_string(id) + " status: " + oldStatus + " -> " + newStatus);

    // If resolved, send notification
    if (newStatus == "Resolved") {
        logEmail(c->getCitizenEmail(), "Complaint #" + to_string(id) + " Resolved", "Sent");
        logActivity("[" + string(Icon::CHECK) + "] Complaint #" + to_string(id) + " resolved by " + c->getAssignedOfficer().getName());
    }
}

// ══════════════════════════════════════════════════════════
//  VIEW COMPLAINT DETAIL
// ══════════════════════════════════════════════════════════

void ComplaintSystem::viewComplaintDetail(int id) const {
    Complaint* c = searchComplaint(id);  // throws if not found

    ConsoleUI::drawDetailHeader(id);
    ConsoleUI::drawDetailField("Department", c->getDepartment());
    ConsoleUI::drawDetailField("Severity", c->getSeverity());
    ConsoleUI::drawDetailField("Status", c->getStatus());
    ConsoleUI::drawDetailField("Description", c->getDescription());
    ConsoleUI::drawDetailField("Filed By", c->getCitizenName());
    ConsoleUI::drawDetailField("Contact", c->getCitizenContact());
    ConsoleUI::drawDetailField("Email", c->getCitizenEmail());
    ConsoleUI::drawDetailField("Filed On", c->getFormattedTime());
    ConsoleUI::drawDetailField("Date", c->getDateOfSubmission());
    ConsoleUI::drawDetailField("Deadline", c->getDeadlineTime());
    ConsoleUI::drawDetailField("Photo", c->getPhotoPath());

    ConsoleUI::drawDetailSection("ASSIGNED OFFICER");
    if (c->getIsAssigned()) {
        ConsoleUI::drawDetailField("Officer", c->getAssignedOfficer().getName());
        ConsoleUI::drawDetailField("Email", c->getAssignedOfficer().getEmail());
        ConsoleUI::drawDetailField("Level", c->getAssignedOfficer().getLevelTitle());
    } else {
        ConsoleUI::drawDetailField("Officer", "Not yet assigned");
    }

    ConsoleUI::drawDetailSection("ESCALATION INFO");
    ConsoleUI::drawDetailField("Escalation Lvl", to_string(c->getEscalationLevel()));
    ConsoleUI::drawDetailField("Time Elapsed", to_string((int)c->getSecondsPassed()) + " seconds");
    ConsoleUI::drawDetailField("Threshold", to_string(c->getEscalationThreshold()) + " seconds");

    // Resolution proof section
    if (c->getResolutionSubmitted()) {
        ConsoleUI::drawDetailSection("RESOLUTION PROOF");
        ConsoleUI::drawDetailField("Proof", c->getResolutionProof());
        ConsoleUI::drawDetailField("Accepted", c->getCitizenAccepted() ? "Yes" : "Pending");
    }

    // Rating section
    if (c->getCitizenRating() > 0) {
        ConsoleUI::drawDetailSection("CITIZEN RATING");
        string stars = "";
        for (int i = 0; i < c->getCitizenRating(); i++) stars += "*";
        for (int i = c->getCitizenRating(); i < 5; i++) stars += ".";
        ConsoleUI::drawDetailField("Rating", stars + " (" + to_string(c->getCitizenRating()) + "/5)");
    }

    ConsoleUI::drawDetailFooter();
}

// ══════════════════════════════════════════════════════════
//  PROOF OF RESOLUTION
// ══════════════════════════════════════════════════════════

void ComplaintSystem::submitResolutionProof(int complaintId, const string& proof) {
    Complaint* c = searchComplaint(complaintId);
    c->submitResolutionProof(proof);
    logActivity("[+] Resolution proof submitted for Complaint #" + to_string(complaintId));
    logEmail(c->getCitizenEmail(), "Resolution proof for Complaint #" + to_string(complaintId), "Sent");
}

void ComplaintSystem::citizenAcceptResolution(int complaintId) {
    Complaint* c = searchComplaint(complaintId);
    c->acceptResolution();
    logActivity("[+] Citizen accepted resolution for Complaint #" + to_string(complaintId));
}

void ComplaintSystem::citizenRejectResolution(int complaintId) {
    Complaint* c = searchComplaint(complaintId);
    c->rejectResolution();
    logActivity("[!] Citizen rejected resolution for Complaint #" + to_string(complaintId) + " — returned to In Progress");
}

// ══════════════════════════════════════════════════════════
//  RATING SYSTEM
// ══════════════════════════════════════════════════════════

void ComplaintSystem::rateOfficer(int complaintId, int rating) {
    Complaint* c = searchComplaint(complaintId);
    c->setRating(rating);

    // Also update the officer's average rating
    if (c->getIsAssigned()) {
        Officer* officer = findOfficerById(c->getAssignedOfficer().getId());
        if (officer) {
            officer->addRating(rating);
        }
    }

    logActivity("[*] Complaint #" + to_string(complaintId) + " rated " + to_string(rating) + "/5");
}

// ══════════════════════════════════════════════════════════
//  COMPLAINT HISTORY & FILTERING
// ══════════════════════════════════════════════════════════

vector<Complaint*> ComplaintSystem::getComplaintsByEmail(const string& email) const {
    vector<Complaint*> result;
    for (auto* c : complaints) {
        if (c->getCitizenEmail() == email) {
            result.push_back(c);
        }
    }
    return result;
}

vector<Complaint*> ComplaintSystem::filterByStatus(const string& status) const {
    vector<Complaint*> result;
    for (auto* c : complaints) {
        if (c->getStatus() == status) {
            result.push_back(c);
        }
    }
    return result;
}

vector<Complaint*> ComplaintSystem::filterByDepartment(const string& dept) const {
    vector<Complaint*> result;
    for (auto* c : complaints) {
        string d = c->getDepartment();
        if (dept == "Road" && (d.find("PWD") != string::npos || d.find("Public") != string::npos || d.find("Road") != string::npos)) {
            result.push_back(c);
        } else if (dept == "Water" && d.find("Water") != string::npos) {
            result.push_back(c);
        } else if (dept == "Electricity" && d.find("Elec") != string::npos) {
            result.push_back(c);
        }
    }
    return result;
}

vector<Complaint*> ComplaintSystem::filterByDate(const string& date) const {
    vector<Complaint*> result;
    for (auto* c : complaints) {
        if (c->getDateOfSubmission() == date || c->getFormattedTime().find(date) != string::npos) {
            result.push_back(c);
        }
    }
    return result;
}

void ComplaintSystem::displayFilteredComplaints(const vector<Complaint*>& filtered, const string& filterLabel) const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("FILTERED COMPLAINTS: " + filterLabel);

    if (filtered.empty()) {
        ConsoleUI::drawWarning("No complaints match the filter: " + filterLabel);
        return;
    }

    ConsoleUI::drawComplaintTableHeader();
    for (const auto* c : filtered) {
        ConsoleUI::drawComplaintTableRow(
            c->getId(),
            c->getDescription(),
            c->getDepartment(),
            c->getSeverity(),
            c->getStatus(),
            c->getAssignedOfficer().getName()
        );
    }
    ConsoleUI::drawComplaintTableFooter();

    ConsoleUI::drawInfo("Found: " + to_string(filtered.size()) + " complaints matching '" + filterLabel + "'");
}

vector<Complaint*> ComplaintSystem::getComplaintsByOfficer(int officerId) const {
    vector<Complaint*> result;
    for (auto* c : complaints) {
        if (c->getIsAssigned() && c->getAssignedOfficer().getId() == officerId) {
            result.push_back(c);
        }
    }
    return result;
}

// ══════════════════════════════════════════════════════════
//  DASHBOARD & REPORTING
// ══════════════════════════════════════════════════════════

DashboardStats ComplaintSystem::buildDashboardStats() const {
    DashboardStats stats = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}};
    stats.totalComplaints = static_cast<int>(complaints.size());

    for (const auto* c : complaints) {
        // Status counts
        string s = c->getStatus();
        if (s == "Resolved") stats.resolved++;
        else if (s == "In Progress") stats.inProgress++;
        else if (s == "Pending") stats.pending++;
        else if (s == "Escalated") stats.escalated++;

        // Department counts
        string d = c->getDepartment();
        if (d.find("PWD") != string::npos || d.find("Public") != string::npos) stats.roadCount++;
        else if (d.find("Water") != string::npos) stats.waterCount++;
        else if (d.find("Elec") != string::npos) stats.electricityCount++;

        // Severity counts
        string sev = c->getSeverity();
        if (sev == "Emergency") stats.emergencyCount++;
        else if (sev == "Normal") stats.normalCount++;
        else if (sev == "Low") stats.lowCount++;
    }

    // Recent activity (up to 5)
    for (int i = 0; i < min(5, static_cast<int>(activityLog.size())); i++) {
        stats.recentActivity.push_back(activityLog[i].text);
    }

    return stats;
}

void ComplaintSystem::writeDashboardJSON() const {
    DashboardWriter writer;
    DashboardStats stats = buildDashboardStats();

    // Build complaint data
    vector<ComplaintData> cData;
    for (const auto* c : complaints) {
        ComplaintData cd;
        cd.id = c->getId();
        cd.description = c->getDescription();
        cd.department = c->getDepartment();
        cd.severity = c->getSeverity();
        cd.status = c->getStatus();
        cd.officer = c->getAssignedOfficer().getName();
        cd.time = c->getFormattedTime();
        cData.push_back(cd);
    }

    // Build officer data
    vector<OfficerData> oData;
    for (const auto& o : officers) {
        oData.push_back({o.getId(), o.getName(), o.getEmail(), o.getDepartment(), o.getLevel()});
    }

    writer.write(stats, cData, oData, emailLog, activityLog);
}

void ComplaintSystem::openDashboard() const {
    DashboardWriter::openInBrowser();
}

void ComplaintSystem::viewEmailLog() const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("EMAIL AUDIT LOG");

    if (emailLog.empty()) {
        ConsoleUI::drawWarning("No emails sent yet.");
        return;
    }

    cout << endl;
    cout << "  " << Color::CYAN
         << Box::TL << ConsoleUI::repeat(Box::H, 30) << Box::TT
         << ConsoleUI::repeat(Box::H, 30) << Box::TT
         << ConsoleUI::repeat(Box::H, 8) << Box::TR
         << Color::RESET << endl;

    cout << "  " << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Recipient                     " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Subject                       " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET
         << Color::BOLD << " Status " << Color::RESET
         << Color::CYAN << Box::V << Color::RESET << endl;

    cout << "  " << Color::CYAN
         << Box::LT << ConsoleUI::repeat(Box::H, 30) << Box::CR
         << ConsoleUI::repeat(Box::H, 30) << Box::CR
         << ConsoleUI::repeat(Box::H, 8) << Box::RT
         << Color::RESET << endl;

    for (const auto& e : emailLog) {
        string statusColor = (e.status == "Sent") ? Color::GREEN : Color::YELLOW;
        cout << "  " << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ConsoleUI::truncate(e.to, 28), 29)
             << Color::CYAN << Box::V << Color::RESET
             << " " << ConsoleUI::padRight(ConsoleUI::truncate(e.subject, 28), 29)
             << Color::CYAN << Box::V << Color::RESET
             << " " << statusColor << ConsoleUI::padRight(e.status, 7) << Color::RESET
             << Color::CYAN << Box::V << Color::RESET << endl;
    }

    cout << "  " << Color::CYAN
         << Box::BL << ConsoleUI::repeat(Box::H, 30) << Box::BT
         << ConsoleUI::repeat(Box::H, 30) << Box::BT
         << ConsoleUI::repeat(Box::H, 8) << Box::BR
         << Color::RESET << endl;

    ConsoleUI::drawInfo("Total emails: " + to_string(emailLog.size()));
}

void ComplaintSystem::viewActivityLog() const {
    ConsoleUI::clearScreen();
    ConsoleUI::drawHeader();
    ConsoleUI::drawSectionTitle("SYSTEM ACTIVITY LOG");

    if (activityLog.empty()) {
        ConsoleUI::drawWarning("No activity recorded yet.");
        return;
    }

    cout << endl;
    for (int i = 0; i < min(20, static_cast<int>(activityLog.size())); i++) {
        string color = Color::DIM;
        if (activityLog[i].text.find("[+]") != string::npos) color = Color::GREEN;
        else if (activityLog[i].text.find("[!]") != string::npos) color = Color::RED;
        else if (activityLog[i].text.find("[*]") != string::npos) color = Color::YELLOW;
        else if (activityLog[i].text.find("[@]") != string::npos) color = Color::MAGENTA;
        else if (activityLog[i].text.find("[^]") != string::npos) color = Color::CYAN;

        cout << "  " << color << "  " << activityLog[i].text << Color::RESET << endl;
    }

    cout << endl;
    ConsoleUI::drawInfo("Showing " + to_string(min(20, static_cast<int>(activityLog.size()))) + " of " + to_string(activityLog.size()) + " entries");
}

// ══════════════════════════════════════════════════════════
//  ESCALATION — Delegates to EscalationManager
// ══════════════════════════════════════════════════════════

int ComplaintSystem::runEscalationCheck() {
    EscalationManager manager(3);   // max escalation level = 3
    return manager.runFullCheck(*this);
}

// ══════════════════════════════════════════════════════════
//  FILE I/O — Save/Load Complaints to File
// ══════════════════════════════════════════════════════════

void ComplaintSystem::saveToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        throw FileIOException("Cannot open file for writing: " + filename);
    }

    file << "# SPCMS Complaint Data File" << endl;
    file << "# Format: TYPE|DESC|PHOTO|SEVERITY|STATUS|CIT_NAME|CIT_CONTACT|CIT_EMAIL|EXTRA1|EXTRA2|RATING|PROOF|ACCEPTED" << endl;
    file << complaints.size() << endl;

    for (const auto* c : complaints) {
        string type = "Unknown";
        string extra1 = "", extra2 = "";
        string dept = c->getDepartment();

        if (dept.find("PWD") != string::npos || dept.find("Public") != string::npos) {
            type = "Road";
            // We store basic info — type-specific fields are lost on reload
            // but core complaint data is preserved
        } else if (dept.find("Water") != string::npos) {
            type = "Water";
        } else if (dept.find("Elec") != string::npos) {
            type = "Electricity";
        }

        file << type << "|"
             << c->getDescription() << "|"
             << c->getPhotoPath() << "|"
             << c->getSeverity() << "|"
             << c->getStatus() << "|"
             << c->getCitizenName() << "|"
             << c->getCitizenContact() << "|"
             << c->getCitizenEmail() << "|"
             << c->getCitizenRating() << "|"
             << c->getResolutionProof() << "|"
             << (c->getCitizenAccepted() ? "1" : "0")
             << endl;
    }

    file.close();
    ConsoleUI::drawSuccess("Data saved to " + filename);
}

void ComplaintSystem::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        // File doesn't exist yet — not an error on first run
        return;
    }

    string line;
    // Skip header comments
    getline(file, line); // # SPCMS...
    getline(file, line); // # Format...

    int count = 0;
    getline(file, line);
    try { count = stoi(line); } catch (...) { return; }

    for (int i = 0; i < count; i++) {
        if (!getline(file, line)) break;

        // Parse pipe-delimited fields
        vector<string> fields;
        stringstream ss(line);
        string field;
        while (getline(ss, field, '|')) {
            fields.push_back(field);
        }

        if (fields.size() < 11) continue;

        string type = fields[0];
        string desc = fields[1];
        string photo = fields[2];
        string severity = fields[3];
        string status = fields[4];
        string citName = fields[5];
        string citContact = fields[6];
        string citEmail = fields[7];
        int rating = 0;
        try { rating = stoi(fields[8]); } catch (...) {}
        string proof = fields[9];
        bool accepted = (fields[10] == "1");

        try {
            int id = registerComplaintDirect(type, desc, photo, severity,
                                              citName, citContact, citEmail,
                                              "", "");
            // Restore status
            if (status != "Pending") {
                updateComplaintStatus(id, status);
            }
            // Restore proof/rating
            Complaint* c = searchComplaint(id);
            if (!proof.empty()) {
                c->submitResolutionProof(proof);
            }
            if (accepted) {
                c->acceptResolution();
            }
            if (rating > 0) {
                c->setRating(rating);
            }
            // Restore status again in case proof submission changed it
            if (c->getStatus() != status) {
                c->setStatus(status);
            }
        } catch (...) {
            // Skip malformed entries
        }
    }

    file.close();
    ConsoleUI::drawInfo("Loaded " + to_string(count) + " complaints from " + filename);
}
