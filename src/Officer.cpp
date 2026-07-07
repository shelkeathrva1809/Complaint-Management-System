#include "../include/Officer.h"

// ── Default Constructor ──
Officer::Officer()
    : User(), officerId(0), department(""), level(0),
      totalRating(0.0), ratingCount(0) {}

// ── Parameterized Constructor (backwards compatible — no login) ──
Officer::Officer(int id, const string& name, const string& email,
                 const string& dept, int level)
    : User("", "", name, email, ""),
      officerId(id), department(dept), level(level),
      totalRating(0.0), ratingCount(0) {}

// ── Full Constructor with login credentials ──
Officer::Officer(int id, const string& username, const string& password,
                 const string& name, const string& email,
                 const string& dept, int level)
    : User(username, password, name, email, ""),
      officerId(id), department(dept), level(level),
      totalRating(0.0), ratingCount(0) {}

// ── Copy Constructor ──
Officer::Officer(const Officer& other)
    : User(other), officerId(other.officerId),
      department(other.department), level(other.level),
      totalRating(other.totalRating), ratingCount(other.ratingCount) {}

// ── Destructor ──
Officer::~Officer() {}

// ── Get Level Title ──
string Officer::getLevelTitle() const {
    switch (level) {
        case 1: return "Field Officer";
        case 2: return "Supervisor";
        case 3: return "Commissioner";
        default: return "Unknown";
    }
}

// ── Rating System ──
void Officer::addRating(int rating) {
    if (rating >= 1 && rating <= 5) {
        totalRating += rating;
        ratingCount++;
    }
}

// ── Operator == : Compare by ID ──
bool Officer::operator==(const Officer& other) const {
    return this->officerId == other.officerId;
}

// ── Operator << : Stream Insertion ──
ostream& operator<<(ostream& os, const Officer& o) {
    os << "[" << o.officerId << "] " << o.getName()
       << " | " << o.getEmail()
       << " | " << o.department
       << " | Level " << o.level << " (" << o.getLevelTitle() << ")"
       << " | Rating: ";
    if (o.ratingCount > 0) {
        os << fixed;
        os.precision(1);
        os << o.getAverageRating() << "/5 (" << o.ratingCount << " reviews)";
    } else {
        os << "No ratings";
    }
    return os;
}
