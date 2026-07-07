#include "../include/Citizen.h"

// ── Static Member Initialization ──
int Citizen::nextCitizenId = 1;

// ── Default Constructor ──
Citizen::Citizen()
    : User(), citizenId(0), address("") {}

// ── Parameterized Constructor (no login — backwards compatible) ──
Citizen::Citizen(const string& name, const string& phone,
                 const string& email, const string& address)
    : User("", "", name, email, phone),
      citizenId(nextCitizenId++), address(address) {}

// ── Full Constructor with login credentials ──
Citizen::Citizen(const string& username, const string& password,
                 const string& name, const string& phone,
                 const string& email, const string& address)
    : User(username, password, name, email, phone),
      citizenId(nextCitizenId++), address(address) {}

// ── Copy Constructor ──
Citizen::Citizen(const Citizen& other)
    : User(other), citizenId(other.citizenId), address(other.address) {}

// ── Destructor ──
Citizen::~Citizen() {}

// ── Operator == : Compare by ID ──
bool Citizen::operator==(const Citizen& other) const {
    return this->citizenId == other.citizenId;
}

// ── Operator << : Stream Insertion ──
ostream& operator<<(ostream& os, const Citizen& c) {
    os << "Citizen [" << c.citizenId << "] " << c.getName()
       << " | Phone: " << c.getPhone()
       << " | Email: " << c.getEmail()
       << " | Addr: " << c.address;
    return os;
}
