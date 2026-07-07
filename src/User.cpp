#include "../include/User.h"

// ══════════════════════════════════════════════════════════
//  Static Member Initialization
// ══════════════════════════════════════════════════════════
int User::nextUserId = 1;

// ══════════════════════════════════════════════════════════
//  CONSTRUCTORS & DESTRUCTOR
// ══════════════════════════════════════════════════════════

// Default Constructor
User::User()
    : userId(0), username(""), password(""), name(""),
      email(""), phone("") {}

// Parameterized Constructor
User::User(const string& username, const string& password,
           const string& name, const string& email, const string& phone)
    : userId(nextUserId++), username(username), password(password),
      name(name), email(email), phone(phone) {}

// Copy Constructor
User::User(const User& other)
    : userId(other.userId), username(other.username), password(other.password),
      name(other.name), email(other.email), phone(other.phone) {}

// Virtual Destructor
User::~User() {}

// ══════════════════════════════════════════════════════════
//  AUTHENTICATION
// ══════════════════════════════════════════════════════════

bool User::authenticate(const string& uname, const string& pwd) const {
    return (username == uname && password == pwd);
}

// ══════════════════════════════════════════════════════════
//  OPERATOR OVERLOADING
// ══════════════════════════════════════════════════════════

bool User::operator==(const User& other) const {
    return this->userId == other.userId;
}

ostream& operator<<(ostream& os, const User& u) {
    os << "[" << u.userId << "] " << u.name
       << " (" << u.getRoleString() << ")"
       << " | " << u.email;
    return os;
}
