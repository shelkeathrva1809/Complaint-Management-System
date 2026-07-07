// ══════════════════════════════════════════════════════════
//  SPCMS — Smart Public Complaint Management System
//  Multi-Role Governance System with Login-Based Access
//  Demonstrates:
//    All OOP concepts PLUS:
//    - Role-Based Access Control (Citizen, Officer, SrOfficer, Admin)
//    - Login Authentication (username/password)
//    - Status Tracking with Timeline
//    - Proof of Resolution + Citizen Accept/Reject
//    - Rating System (1-5, linked to officer performance)
//    - Admin Analytics Dashboard
//    - Auto Escalation (time-based)
//    - Complaint History & Search/Filter
//    - File I/O Persistence
//    - Department Auto-Assignment
// ══════════════════════════════════════════════════════════

#include <iostream>
#include <vector>
#include <iomanip>
#include "include/Officer.h"
#include "include/SeniorOfficer.h"
#include "include/Citizen.h"
#include "include/Admin.h"
#include "include/Complaint.h"
#include "include/RoadComplaint.h"
#include "include/WaterComplaint.h"
#include "include/ElectricityComplaint.h"
#include "include/Exceptions.h"
#include "include/ConsoleUI.h"
#include "include/DashboardWriter.h"
#include "include/ComplaintSystem.h"
#include "include/EscalationManager.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  CITIZEN DASHBOARD — All citizen operations
// ══════════════════════════════════════════════════════════
void citizenDashboard(ComplaintSystem& system, Citizen* citizen) {
    bool running = true;
    while (running) {
        ConsoleUI::drawCitizenMenu(citizen->getName());
        int choice = ConsoleUI::getMenuChoice(0, 6);

        switch (choice) {
            // ──────── [1] Register New Complaint ────────
            case 1: {
                try {
                    int id = system.registerComplaintForCitizen(
                        citizen->getName(), citizen->getPhone(), citizen->getEmail());
                    ConsoleUI::drawSuccess("Complaint #" + to_string(id) + " registered and forwarded!");
                } catch (const InvalidInputException& e) {
                    ConsoleUI::drawError(e.what());
                } catch (const exception& e) {
                    ConsoleUI::drawError(string("Registration error: ") + e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [2] View My Complaints ────────
            case 2: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("MY COMPLAINTS");

                vector<Complaint*> myComplaints = system.getComplaintsByEmail(citizen->getEmail());
                if (myComplaints.empty()) {
                    ConsoleUI::drawWarning("You have no complaints in the system.");
                } else {
                    ConsoleUI::drawComplaintTableHeader();
                    for (const auto* c : myComplaints) {
                        ConsoleUI::drawComplaintTableRow(
                            c->getId(), c->getDescription(), c->getDepartment(),
                            c->getSeverity(), c->getStatus(),
                            c->getAssignedOfficer().getName());
                    }
                    ConsoleUI::drawComplaintTableFooter();
                    ConsoleUI::drawInfo("Total: " + to_string(myComplaints.size()) + " complaint(s)");

                    // Show status timeline for each complaint
                    cout << endl;
                    for (const auto* c : myComplaints) {
                        const auto& timeline = c->getStatusTimeline();
                        if (!timeline.empty()) {
                            cout << "  " << Color::BOLD << "  Complaint #" << c->getId()
                                 << " Timeline:" << Color::RESET << endl;
                            for (const auto& entry : timeline) {
                                cout << "    " << Color::DIM << entry << Color::RESET << endl;
                            }
                        }
                    }
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [3] Search Complaint by ID ────────
            case 3: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("SEARCH COMPLAINT");
                int searchId = ConsoleUI::getIntInput("Enter Complaint ID");
                try {
                    system.viewComplaintDetail(searchId);
                } catch (const ComplaintNotFoundException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [4] Filter Complaints ────────
            case 4: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("FILTER COMPLAINTS");

                string filterType = ConsoleUI::getChoiceInput(
                    "Filter by:", {"Status", "Department", "Date"});

                vector<Complaint*> filtered;
                string label = "";

                if (filterType == "Status") {
                    string status = ConsoleUI::getChoiceInput(
                        "Select status:", {"Pending", "Assigned", "In Progress", "Resolved", "Escalated"});
                    if (!status.empty()) {
                        filtered = system.filterByStatus(status);
                        label = "Status: " + status;
                    }
                } else if (filterType == "Department") {
                    string dept = ConsoleUI::getChoiceInput(
                        "Select department:", {"Road", "Water", "Electricity"});
                    if (!dept.empty()) {
                        filtered = system.filterByDepartment(dept);
                        label = "Department: " + dept;
                    }
                } else if (filterType == "Date") {
                    string date = ConsoleUI::getInput("Enter date (dd-Mon-yyyy, e.g. 02-Apr-2026)");
                    filtered = system.filterByDate(date);
                    label = "Date: " + date;
                }

                if (!label.empty()) {
                    system.displayFilteredComplaints(filtered, label);
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [5] Accept/Reject Resolution ────────
            case 5: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("ACCEPT / REJECT RESOLUTION");

                // Show resolved complaints for this citizen
                vector<Complaint*> myComplaints = system.getComplaintsByEmail(citizen->getEmail());
                bool found = false;

                ConsoleUI::drawInfo("Complaints with resolution proof:");
                cout << endl;
                for (const auto* c : myComplaints) {
                    if (c->getResolutionSubmitted() && !c->getCitizenAccepted()) {
                        found = true;
                        cout << "  " << Color::YELLOW << "  Complaint #" << c->getId()
                             << " — " << c->getDescription() << Color::RESET << endl;
                        cout << "  " << Color::DIM << "  Proof: " << c->getResolutionProof()
                             << Color::RESET << endl;
                        cout << endl;
                    }
                }

                if (!found) {
                    ConsoleUI::drawInfo("No pending resolutions to review.");
                } else {
                    int compId = ConsoleUI::getIntInput("Enter Complaint ID to review");
                    try {
                        Complaint* c = system.searchComplaint(compId);

                        // Verify this complaint belongs to the citizen
                        if (c->getCitizenEmail() != citizen->getEmail()) {
                            ConsoleUI::drawError("This complaint does not belong to you.");
                        } else if (!c->getResolutionSubmitted()) {
                            ConsoleUI::drawWarning("No resolution proof submitted yet for this complaint.");
                        } else {
                            ConsoleUI::drawInfo("Resolution Proof: " + c->getResolutionProof());
                            string action = ConsoleUI::getChoiceInput(
                                "Your decision:", {"Accept Resolution", "Reject Resolution"});

                            if (action == "Accept Resolution") {
                                system.citizenAcceptResolution(compId);
                                ConsoleUI::drawSuccess("Resolution accepted! Complaint #" +
                                                       to_string(compId) + " is now closed.");

                                // Prompt for rating
                                ConsoleUI::drawInfo("Please rate the officer's service (1-5):");
                                int rating = ConsoleUI::getIntInput("Rating (1=Poor, 5=Excellent)");
                                if (rating >= 1 && rating <= 5) {
                                    system.rateOfficer(compId, rating);
                                    ConsoleUI::drawRatingStars(rating);
                                    ConsoleUI::drawSuccess("Thank you for your feedback!");
                                } else {
                                    ConsoleUI::drawWarning("Invalid rating. Skipped.");
                                }
                            } else if (action == "Reject Resolution") {
                                system.citizenRejectResolution(compId);
                                ConsoleUI::drawWarning("Resolution rejected. Complaint #" +
                                                       to_string(compId) + " returned to In Progress.");
                            }
                        }
                    } catch (const ComplaintNotFoundException& e) {
                        ConsoleUI::drawError(e.what());
                    }
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [6] Rate Officer ────────
            case 6: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("RATE OFFICER");

                // Show resolved & accepted complaints without rating
                vector<Complaint*> myComplaints = system.getComplaintsByEmail(citizen->getEmail());
                bool found = false;

                for (const auto* c : myComplaints) {
                    if (c->getStatus() == "Resolved" && c->getCitizenAccepted()
                        && c->getCitizenRating() == 0) {
                        found = true;
                        cout << "  " << Color::GREEN << "  Complaint #" << c->getId()
                             << " — " << c->getDescription()
                             << " (Officer: " << c->getAssignedOfficer().getName() << ")"
                             << Color::RESET << endl;
                    }
                }

                if (!found) {
                    ConsoleUI::drawInfo("No resolved complaints pending rating.");
                } else {
                    int compId = ConsoleUI::getIntInput("Enter Complaint ID to rate");
                    try {
                        Complaint* c = system.searchComplaint(compId);
                        if (c->getCitizenEmail() != citizen->getEmail()) {
                            ConsoleUI::drawError("This complaint does not belong to you.");
                        } else if (c->getCitizenRating() > 0) {
                            ConsoleUI::drawWarning("You have already rated this complaint.");
                            ConsoleUI::drawRatingStars(c->getCitizenRating());
                        } else {
                            int rating = ConsoleUI::getIntInput("Rating (1=Poor, 5=Excellent)");
                            if (rating >= 1 && rating <= 5) {
                                system.rateOfficer(compId, rating);
                                ConsoleUI::drawRatingStars(rating);
                                ConsoleUI::drawSuccess("Rating submitted! Thank you.");
                            } else {
                                ConsoleUI::drawError("Invalid rating. Must be 1-5.");
                            }
                        }
                    } catch (const ComplaintNotFoundException& e) {
                        ConsoleUI::drawError(e.what());
                    }
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [0] Logout ────────
            case 0: {
                ConsoleUI::drawInfo("Logging out...");
                running = false;
                break;
            }

            default: {
                ConsoleUI::drawError("Invalid choice. Please enter 0-6.");
                ConsoleUI::pause();
                break;
            }
        }
    }
}

// ══════════════════════════════════════════════════════════
//  OFFICER DASHBOARD — All officer operations
// ══════════════════════════════════════════════════════════
void officerDashboard(ComplaintSystem& system, Officer* officer) {
    bool running = true;
    while (running) {
        ConsoleUI::drawOfficerMenu(officer->getName());
        int choice = ConsoleUI::getMenuChoice(0, 7);

        switch (choice) {
            // ──────── [1] View Assigned Complaints ────────
            case 1: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("MY ASSIGNED COMPLAINTS");

                vector<Complaint*> assigned = system.getComplaintsByOfficer(officer->getId());
                if (assigned.empty()) {
                    ConsoleUI::drawInfo("No complaints assigned to you.");
                } else {
                    ConsoleUI::drawComplaintTableHeader();
                    for (const auto* c : assigned) {
                        ConsoleUI::drawComplaintTableRow(
                            c->getId(), c->getDescription(), c->getDepartment(),
                            c->getSeverity(), c->getStatus(),
                            officer->getName());
                    }
                    ConsoleUI::drawComplaintTableFooter();
                    ConsoleUI::drawInfo("Total assigned: " + to_string(assigned.size()));
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [2] Update Complaint Status ────────
            case 2: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("UPDATE COMPLAINT STATUS");

                int updateId = ConsoleUI::getIntInput("Enter Complaint ID");
                try {
                    Complaint* c = system.searchComplaint(updateId);

                    // Verify this complaint is assigned to this officer
                    if (!c->getIsAssigned() || c->getAssignedOfficer().getId() != officer->getId()) {
                        ConsoleUI::drawError("This complaint is not assigned to you.");
                    } else {
                        ConsoleUI::drawInfo("Current status: " + c->getStatus());
                        string newStatus = ConsoleUI::getChoiceInput(
                            "Select new status:",
                            {"Pending", "Assigned", "In Progress", "Resolved", "Escalated"});

                        if (!newStatus.empty()) {
                            system.updateComplaintStatus(updateId, newStatus);
                            ConsoleUI::drawSuccess("Status updated to: " + newStatus);

                            if (newStatus == "Resolved") {
                                ConsoleUI::drawEmailAnimation(c->getCitizenEmail());
                            }
                        }
                    }
                } catch (const ComplaintNotFoundException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [3] Submit Resolution Proof ────────
            case 3: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("SUBMIT RESOLUTION PROOF");

                int proofId = ConsoleUI::getIntInput("Enter Complaint ID");
                try {
                    Complaint* c = system.searchComplaint(proofId);

                    if (!c->getIsAssigned() || c->getAssignedOfficer().getId() != officer->getId()) {
                        ConsoleUI::drawError("This complaint is not assigned to you.");
                    } else if (c->getResolutionSubmitted()) {
                        ConsoleUI::drawWarning("Resolution proof already submitted for this complaint.");
                    } else {
                        ConsoleUI::drawInfo("Provide proof of resolution (file path or description):");
                        string proof = ConsoleUI::getInput("Proof");

                        if (!proof.empty()) {
                            system.submitResolutionProof(proofId, proof);
                            ConsoleUI::drawSuccess("Resolution proof submitted for Complaint #" +
                                                   to_string(proofId));
                            ConsoleUI::drawInfo("Citizen will be notified to accept/reject.");
                            ConsoleUI::drawEmailAnimation(c->getCitizenEmail());
                        } else {
                            ConsoleUI::drawWarning("No proof provided. Submission cancelled.");
                        }
                    }
                } catch (const ComplaintNotFoundException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [4] View All Complaints ────────
            case 4: {
                system.viewAllComplaints();
                ConsoleUI::pause();
                break;
            }

            // ──────── [5] Search Complaint by ID ────────
            case 5: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("SEARCH COMPLAINT");
                int searchId = ConsoleUI::getIntInput("Enter Complaint ID");
                try {
                    system.viewComplaintDetail(searchId);
                } catch (const ComplaintNotFoundException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [6] View My Ratings ────────
            case 6: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("MY PERFORMANCE RATINGS");

                cout << endl;
                cout << "  " << Color::BOLD << "  Officer: " << officer->getName() << Color::RESET << endl;
                cout << "  " << Color::DIM << "  Department: " << officer->getDepartment() << Color::RESET << endl;
                cout << "  " << Color::DIM << "  Level: " << officer->getLevelTitle() << Color::RESET << endl;
                cout << endl;

                if (officer->getRatingCount() > 0) {
                    cout << "  " << Color::YELLOW << "  Average Rating: ";
                    cout << fixed << setprecision(1) << officer->getAverageRating() << "/5";
                    cout << " (" << officer->getRatingCount() << " reviews)" << Color::RESET << endl;
                    cout << "  ";
                    ConsoleUI::drawRatingStars(static_cast<int>(officer->getAverageRating()));
                } else {
                    ConsoleUI::drawInfo("No ratings received yet.");
                }

                // Show per-complaint ratings
                vector<Complaint*> assigned = system.getComplaintsByOfficer(officer->getId());
                bool hasRated = false;
                for (const auto* c : assigned) {
                    if (c->getCitizenRating() > 0) {
                        if (!hasRated) {
                            cout << endl;
                            cout << "  " << Color::BOLD << "  Individual Ratings:" << Color::RESET << endl;
                            hasRated = true;
                        }
                        cout << "    Complaint #" << c->getId() << ": ";
                        for (int i = 0; i < c->getCitizenRating(); i++) cout << "*";
                        for (int i = c->getCitizenRating(); i < 5; i++) cout << ".";
                        cout << " (" << c->getCitizenRating() << "/5) — "
                             << c->getCitizenName() << endl;
                    }
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [7] Run Escalation Check ────────
            case 7: {
                try {
                    int count = system.runEscalationCheck();
                    if (count > 0) {
                        ConsoleUI::drawWarning(to_string(count) + " complaint(s) were escalated!");
                    }
                } catch (const EscalationException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [0] Logout ────────
            case 0: {
                ConsoleUI::drawInfo("Logging out...");
                running = false;
                break;
            }

            default: {
                ConsoleUI::drawError("Invalid choice. Please enter 0-7.");
                ConsoleUI::pause();
                break;
            }
        }
    }
}

// ══════════════════════════════════════════════════════════
//  SENIOR OFFICER DASHBOARD — Extended officer operations
// ══════════════════════════════════════════════════════════
void seniorOfficerDashboard(ComplaintSystem& system, SeniorOfficer* officer) {
    bool running = true;
    while (running) {
        ConsoleUI::drawSeniorOfficerMenu(officer->getName());
        int choice = ConsoleUI::getMenuChoice(0, 8);

        switch (choice) {
            // ──────── [1] View Assigned Complaints ────────
            case 1: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("MY ASSIGNED COMPLAINTS");

                vector<Complaint*> assigned = system.getComplaintsByOfficer(officer->getId());
                if (assigned.empty()) {
                    ConsoleUI::drawInfo("No complaints assigned to you.");
                } else {
                    ConsoleUI::drawComplaintTableHeader();
                    for (const auto* c : assigned) {
                        ConsoleUI::drawComplaintTableRow(
                            c->getId(), c->getDescription(), c->getDepartment(),
                            c->getSeverity(), c->getStatus(),
                            officer->getName());
                    }
                    ConsoleUI::drawComplaintTableFooter();
                    ConsoleUI::drawInfo("Total assigned: " + to_string(assigned.size()));
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [2] View Escalated Complaints ────────
            case 2: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("ESCALATED COMPLAINTS");

                vector<Complaint*> escalated = system.filterByStatus("Escalated");
                if (escalated.empty()) {
                    ConsoleUI::drawSuccess("No escalated complaints at this time.");
                } else {
                    ConsoleUI::drawComplaintTableHeader();
                    for (const auto* c : escalated) {
                        ConsoleUI::drawComplaintTableRow(
                            c->getId(), c->getDescription(), c->getDepartment(),
                            c->getSeverity(), c->getStatus(),
                            c->getAssignedOfficer().getName());
                    }
                    ConsoleUI::drawComplaintTableFooter();
                    ConsoleUI::drawWarning(to_string(escalated.size()) + " complaint(s) currently escalated!");
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [3] Update Complaint Status ────────
            case 3: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("UPDATE COMPLAINT STATUS");

                int updateId = ConsoleUI::getIntInput("Enter Complaint ID");
                try {
                    Complaint* c = system.searchComplaint(updateId);
                    ConsoleUI::drawInfo("Current status: " + c->getStatus());

                    string newStatus = ConsoleUI::getChoiceInput(
                        "Select new status:",
                        {"Pending", "Assigned", "In Progress", "Resolved", "Escalated"});

                    if (!newStatus.empty()) {
                        system.updateComplaintStatus(updateId, newStatus);
                        ConsoleUI::drawSuccess("Status updated to: " + newStatus);
                        if (newStatus == "Resolved") {
                            ConsoleUI::drawEmailAnimation(c->getCitizenEmail());
                        }
                    }
                } catch (const ComplaintNotFoundException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [4] Submit Resolution Proof ────────
            case 4: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("SUBMIT RESOLUTION PROOF");

                int proofId = ConsoleUI::getIntInput("Enter Complaint ID");
                try {
                    Complaint* c = system.searchComplaint(proofId);
                    if (c->getResolutionSubmitted()) {
                        ConsoleUI::drawWarning("Resolution proof already submitted.");
                    } else {
                        string proof = ConsoleUI::getInput("Proof (file path or description)");
                        if (!proof.empty()) {
                            system.submitResolutionProof(proofId, proof);
                            ConsoleUI::drawSuccess("Resolution proof submitted!");
                            ConsoleUI::drawEmailAnimation(c->getCitizenEmail());
                        }
                    }
                } catch (const ComplaintNotFoundException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [5] View All Complaints ────────
            case 5: {
                system.viewAllComplaints();
                ConsoleUI::pause();
                break;
            }

            // ──────── [6] Search Complaint by ID ────────
            case 6: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("SEARCH COMPLAINT");
                int searchId = ConsoleUI::getIntInput("Enter Complaint ID");
                try {
                    system.viewComplaintDetail(searchId);
                } catch (const ComplaintNotFoundException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [7] View My Ratings ────────
            case 7: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("MY PERFORMANCE RATINGS");

                cout << endl;
                cout << "  " << Color::BOLD << "  Sr. Officer: " << officer->getName() << Color::RESET << endl;
                cout << "  " << Color::DIM << "  Specialization: " << officer->getSpecialization() << Color::RESET << endl;

                if (officer->getRatingCount() > 0) {
                    cout << "  " << Color::YELLOW << "  Average Rating: "
                         << fixed << setprecision(1) << officer->getAverageRating() << "/5"
                         << " (" << officer->getRatingCount() << " reviews)" << Color::RESET << endl;
                    cout << "  ";
                    ConsoleUI::drawRatingStars(static_cast<int>(officer->getAverageRating()));
                } else {
                    ConsoleUI::drawInfo("No ratings received yet.");
                }
                cout << "  " << Color::DIM << "  Escalated Complaints Handled: "
                     << officer->getEscalatedHandled() << Color::RESET << endl;
                ConsoleUI::pause();
                break;
            }

            // ──────── [8] Run Escalation Check ────────
            case 8: {
                try {
                    int count = system.runEscalationCheck();
                    if (count > 0) {
                        ConsoleUI::drawWarning(to_string(count) + " complaint(s) were escalated!");
                    }
                } catch (const EscalationException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [0] Logout ────────
            case 0: {
                ConsoleUI::drawInfo("Logging out...");
                running = false;
                break;
            }

            default: {
                ConsoleUI::drawError("Invalid choice. Please enter 0-8.");
                ConsoleUI::pause();
                break;
            }
        }
    }
}

// ══════════════════════════════════════════════════════════
//  ADMIN DASHBOARD — Analytics & System Management
// ══════════════════════════════════════════════════════════
void adminDashboard(ComplaintSystem& system, Admin* admin) {
    bool running = true;
    while (running) {
        ConsoleUI::drawAdminMenu(admin->getName());
        int choice = ConsoleUI::getMenuChoice(0, 9);

        switch (choice) {
            // ──────── [1] View Total Complaints ────────
            case 1: {
                admin->viewTotalComplaints(system);
                ConsoleUI::pause();
                break;
            }

            // ──────── [2] Department-wise Count ────────
            case 2: {
                admin->viewDepartmentWiseCount(system);
                ConsoleUI::pause();
                break;
            }

            // ──────── [3] Pending vs Resolved Analysis ────────
            case 3: {
                admin->viewPendingVsResolved(system);
                ConsoleUI::pause();
                break;
            }

            // ──────── [4] Officer Performance (Worst Performers) ────────
            case 4: {
                admin->viewWorstPerformingOfficers(system);
                ConsoleUI::pause();
                break;
            }

            // ──────── [5] View All Complaints ────────
            case 5: {
                system.viewAllComplaints();
                ConsoleUI::pause();
                break;
            }

            // ──────── [6] View Officer Directory ────────
            case 6: {
                system.viewOfficers();
                ConsoleUI::pause();
                break;
            }

            // ──────── [7] View Email Logs ────────
            case 7: {
                system.viewEmailLog();
                ConsoleUI::pause();
                break;
            }

            // ──────── [8] Run Escalation Check ────────
            case 8: {
                try {
                    int count = system.runEscalationCheck();
                    if (count > 0) {
                        ConsoleUI::drawWarning(to_string(count) + " complaint(s) were escalated!");
                    }
                } catch (const EscalationException& e) {
                    ConsoleUI::drawError(e.what());
                }
                ConsoleUI::pause();
                break;
            }

            // ──────── [9] Open Web Dashboard ────────
            case 9: {
                ConsoleUI::drawInfo("Writing dashboard data...");
                ConsoleUI::drawLoadingBar("Exporting JSON", 1000);
                system.writeDashboardJSON();
                ConsoleUI::drawSuccess("Dashboard data written to data/dashboard.json");
                ConsoleUI::drawInfo("Opening dashboard in browser...");
                system.openDashboard();
                ConsoleUI::pause();
                break;
            }

            // ──────── [0] Logout ────────
            case 0: {
                ConsoleUI::drawInfo("Logging out...");
                running = false;
                break;
            }

            default: {
                ConsoleUI::drawError("Invalid choice. Please enter 0-9.");
                ConsoleUI::pause();
                break;
            }
        }
    }
}

// ══════════════════════════════════════════════════════════
//  MAIN — Entry Point with Login Portal
// ══════════════════════════════════════════════════════════
int main() {
    // Initialize console UI engine
    ConsoleUI::init();

    // ─────────────────────────────────────────
    // SPLASH SCREEN
    // ─────────────────────────────────────────
    ConsoleUI::drawSplashScreen();

    // ─────────────────────────────────────────
    // CREATE COMPLAINT SYSTEM (central controller)
    // ─────────────────────────────────────────
    ComplaintSystem system;

    // ═══════════════════════════════════════════
    //  PRE-SEED USERS WITH LOGIN CREDENTIALS
    // ═══════════════════════════════════════════

    // ── Register Officers (with login) ──
    // Department: PWD (Road)
    system.addOfficer(Officer(101, "officer1", "pass123",
        "R.K. Sharma", "sharma@pwd.gov.in", "PWD", 1));
    system.addOfficer(Officer(102, "officer2", "pass123",
        "S.P. Verma", "verma@pwd.gov.in", "PWD", 2));

    // Department: Water Supply
    system.addOfficer(Officer(201, "officer3", "pass123",
        "A.K. Patel", "patel@water.gov.in", "Water Supply", 1));
    system.addOfficer(Officer(202, "officer4", "pass123",
        "M.S. Iyer", "iyer@water.gov.in", "Water Supply", 2));

    // Department: Electricity Board
    system.addOfficer(Officer(301, "officer5", "pass123",
        "V.K. Gupta", "gupta@electricity.gov.in", "Electricity Board", 1));
    system.addOfficer(Officer(302, "officer6", "pass123",
        "P.N. Rao", "rao@electricity.gov.in", "Electricity Board", 2));

    // Department: Municipal (Commissioner — Level 3)
    system.addOfficer(Officer(103, "officer7", "pass123",
        "Commissioner Das", "das@municipal.gov.in", "Municipal", 3));

    // ── Register Senior Officers (with login) ──
    system.addSeniorOfficer(SeniorOfficer(401, "srofficer1", "pass123",
        "K.N. Jha", "jha@pwd.gov.in", "PWD", 2, "Infrastructure"));
    system.addSeniorOfficer(SeniorOfficer(402, "srofficer2", "pass123",
        "D.P. Mishra", "mishra@water.gov.in", "Water Supply", 2, "Pipeline"));

    // ── Register Citizens (with login) ──
    system.addCitizen(Citizen("citizen1", "pass123",
        "Rajesh Kumar", "9876543210", "rajesh@gmail.com", "12 MG Road, Delhi"));
    system.addCitizen(Citizen("citizen2", "pass123",
        "Priya Verma", "9988776655", "priya@gmail.com", "45 Sector 21, Noida"));
    system.addCitizen(Citizen("citizen3", "pass123",
        "Mohan Singh", "9012345678", "mohan@gmail.com", "78 Ward 7, Mumbai"));
    system.addCitizen(Citizen("citizen4", "pass123",
        "Anita Desai", "8877665544", "anita@gmail.com", "NH-48-Service Road, Gurgaon"));
    system.addCitizen(Citizen("citizen5", "pass123",
        "Sunil Mehta", "7766554433", "sunil@gmail.com", "Greenwood Colony, Pune"));

    // ── Register Admin (with login) ──
    system.addAdmin(Admin("admin", "admin123",
        "System Administrator", "admin@spcms.gov.in", "0000000000", "Super Admin"));

    // ═══════════════════════════════════════════
    //  PRE-LOAD SAMPLE COMPLAINTS (for demo)
    // ═══════════════════════════════════════════
    try {
        system.registerComplaintDirect("Road", "Large pothole on MG Road near Junction 4",
            "C:\\Evidence\\pothole_mg_road.jpg", "Emergency",
            "Rajesh Kumar", "+91-9876543210", "rajesh@gmail.com",
            "MG Road, Junction 4", "12");

        system.registerComplaintDirect("Water", "Water pipeline leak flooding Sector 21",
            "C:\\Evidence\\water_leak.jpg", "Normal",
            "Priya Verma", "+91-9988776655", "priya@gmail.com",
            "PL-2156", "Sector 21");

        system.registerComplaintDirect("Electricity", "Power outage in Ward 7, transformer sparking",
            "C:\\Evidence\\power_outage.jpg", "Emergency",
            "Mohan Singh", "+91-9012345678", "mohan@gmail.com",
            "TF-7042", "200");

        system.registerComplaintDirect("Road", "Broken streetlight on NH-48 service road",
            "C:\\Evidence\\streetlight.jpg", "Low",
            "Anita Desai", "+91-8877665544", "anita@gmail.com",
            "NH-48 Service Road", "5");

        system.registerComplaintDirect("Water", "No water supply in Greenwood Colony for 3 days",
            "C:\\Evidence\\no_water.jpg", "Emergency",
            "Sunil Mehta", "+91-7766554433", "sunil@gmail.com",
            "PL-3089", "Greenwood Colony");

        system.registerComplaintDirect("Electricity", "Frequent voltage fluctuations in Block C",
            "C:\\Evidence\\voltage.jpg", "Normal",
            "Kavita Joshi", "+91-6655443322", "kavita@gmail.com",
            "TF-8901", "50");

        // Set some varied statuses for demo
        system.updateComplaintStatus(2, "In Progress");
        system.updateComplaintStatus(3, "Escalated");

        // Submit resolution proof for complaint #4 (for demo)
        system.submitResolutionProof(4, "Streetlight repaired and tested. Photo: C:\\Evidence\\fixed_light.jpg");
        system.updateComplaintStatus(4, "Resolved");

        // Complaint #5 resolved and accepted
        system.submitResolutionProof(5, "Water supply restored. Pipeline PL-3089 replaced.");
        system.updateComplaintStatus(5, "Resolved");

    } catch (const exception& e) {
        ConsoleUI::drawError(string("Error loading demo data: ") + e.what());
    }

    // ═══════════════════════════════════════════
    //  LOGIN PORTAL — Main Application Loop
    // ═══════════════════════════════════════════
    bool systemRunning = true;
    while (systemRunning) {
        ConsoleUI::drawLoginScreen();
        int roleChoice = ConsoleUI::getMenuChoice(0, 4);

        switch (roleChoice) {
            // ──────── [1] Citizen Login ────────
            case 1: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("CITIZEN LOGIN");

                string username = ConsoleUI::getInput("Username");
                string password = ConsoleUI::getInput("Password");

                Citizen* citizen = system.authenticateCitizen(username, password);
                if (citizen) {
                    ConsoleUI::drawSuccess("Welcome, " + citizen->getName() + "!");
                    ConsoleUI::drawLoadingBar("Loading Citizen Dashboard", 800);
                    citizenDashboard(system, citizen);
                } else {
                    ConsoleUI::drawError("Invalid username or password.");
                    ConsoleUI::drawInfo("Demo accounts: citizen1/pass123, citizen2/pass123, ...");
                    ConsoleUI::pause();
                }
                break;
            }

            // ──────── [2] Officer Login ────────
            case 2: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("OFFICER LOGIN");

                string username = ConsoleUI::getInput("Username");
                string password = ConsoleUI::getInput("Password");

                Officer* officer = system.authenticateOfficer(username, password);
                if (officer) {
                    ConsoleUI::drawSuccess("Welcome, Officer " + officer->getName() + "!");
                    ConsoleUI::drawInfo("Department: " + officer->getDepartment() +
                                       " | Level: " + officer->getLevelTitle());
                    ConsoleUI::drawLoadingBar("Loading Officer Dashboard", 800);
                    officerDashboard(system, officer);
                } else {
                    ConsoleUI::drawError("Invalid username or password.");
                    ConsoleUI::drawInfo("Demo accounts: officer1/pass123, officer2/pass123, ...");
                    ConsoleUI::pause();
                }
                break;
            }

            // ──────── [3] Senior Officer Login ────────
            case 3: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("SENIOR OFFICER LOGIN");

                string username = ConsoleUI::getInput("Username");
                string password = ConsoleUI::getInput("Password");

                SeniorOfficer* srOfficer = system.authenticateSeniorOfficer(username, password);
                if (srOfficer) {
                    ConsoleUI::drawSuccess("Welcome, Sr. Officer " + srOfficer->getName() + "!");
                    ConsoleUI::drawInfo("Specialization: " + srOfficer->getSpecialization());
                    ConsoleUI::drawLoadingBar("Loading Senior Officer Dashboard", 800);
                    seniorOfficerDashboard(system, srOfficer);
                } else {
                    ConsoleUI::drawError("Invalid username or password.");
                    ConsoleUI::drawInfo("Demo accounts: srofficer1/pass123, srofficer2/pass123");
                    ConsoleUI::pause();
                }
                break;
            }

            // ──────── [4] Admin Login ────────
            case 4: {
                ConsoleUI::clearScreen();
                ConsoleUI::drawHeader();
                ConsoleUI::drawSectionTitle("ADMIN LOGIN");

                string username = ConsoleUI::getInput("Username");
                string password = ConsoleUI::getInput("Password");

                Admin* admin = system.authenticateAdmin(username, password);
                if (admin) {
                    ConsoleUI::drawSuccess("Welcome, Admin " + admin->getName() + "!");
                    ConsoleUI::drawInfo("Access Level: " + admin->getAdminLevel());
                    ConsoleUI::drawLoadingBar("Loading Admin Dashboard", 800);
                    adminDashboard(system, admin);
                } else {
                    ConsoleUI::drawError("Invalid username or password.");
                    ConsoleUI::drawInfo("Demo account: admin/admin123");
                    ConsoleUI::pause();
                }
                break;
            }

            // ──────── [0] Exit System ────────
            case 0: {
                ConsoleUI::clearScreen();
                cout << endl;

                // Save data before exiting
                try {
                    system.saveToFile();
                    system.writeDashboardJSON();
                } catch (const exception& e) {
                    ConsoleUI::drawWarning(string("Save warning: ") + e.what());
                }

                ConsoleUI::typeWriter("  Shutting down SPCMS...\n", 40);
                ConsoleUI::drawSuccess("Data saved. System closed. Goodbye!");
                cout << endl;
                systemRunning = false;
                break;
            }

            // ──────── Invalid ────────
            default: {
                ConsoleUI::drawError("Invalid choice. Please enter 0-4.");
                ConsoleUI::pause();
                break;
            }
        }
    }

    // ComplaintSystem destructor handles all cleanup (delete Complaint*)
    return 0;
}
