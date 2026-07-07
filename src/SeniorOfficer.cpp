#include "../include/SeniorOfficer.h"

// ── Default Constructor ──
SeniorOfficer::SeniorOfficer()
    : Officer(), escalatedHandled(0), specialization("General") {}

// ── Parameterized Constructor (backwards compatible) ──
SeniorOfficer::SeniorOfficer(int id, const string& name, const string& email,
                              const string& dept, int level,
                              const string& specialization)
    : Officer(id, name, email, dept, level),
      escalatedHandled(0), specialization(specialization) {}

// ── Full Constructor with login ──
SeniorOfficer::SeniorOfficer(int id, const string& username, const string& password,
                              const string& name, const string& email,
                              const string& dept, int level,
                              const string& specialization)
    : Officer(id, username, password, name, email, dept, level),
      escalatedHandled(0), specialization(specialization) {}

// ── Copy Constructor ──
SeniorOfficer::SeniorOfficer(const SeniorOfficer& other)
    : Officer(other), escalatedHandled(other.escalatedHandled),
      specialization(other.specialization) {}

// ── Destructor ──
SeniorOfficer::~SeniorOfficer() {}

// ── Handle Escalated Complaint ──
void SeniorOfficer::handleEscalatedComplaint() {
    escalatedHandled++;
}
