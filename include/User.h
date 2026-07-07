#pragma once
#include <string>
#include <iostream>
using namespace std;

// ══════════════════════════════════════════════════════════
//  User — Abstract Base Class for all system users
//  Demonstrates:
//    - Abstraction (pure virtual getRole, getRoleString)
//    - Encapsulation (protected data, public interface)
//    - Inheritance (Citizen, Officer, Admin derive from User)
//    - Virtual Destructor (for polymorphic deletion)
//    - Inline Functions (getters)
//    - this Pointer (setters)
//    - Static Members (auto-increment ID)
// ══════════════════════════════════════════════════════════

// Enum for user roles — used for access control
enum class Role { CITIZEN, OFFICER, SENIOR_OFFICER, ADMIN };

class User {
protected:
    int userId;
    string username;        // login username
    string password;        // login password
    string name;
    string email;
    string phone;

    static int nextUserId;  // auto-increment user ID

public:
    // ── Constructors ──
    User();
    User(const string& username, const string& password,
         const string& name, const string& email, const string& phone);
    User(const User& other);

    // ── Virtual Destructor ──
    virtual ~User();

    // ══════════════════════════════════════════════════════
    //  PURE VIRTUAL — Makes User ABSTRACT
    //  Every derived class MUST override these
    // ══════════════════════════════════════════════════════
    virtual Role getRole() const = 0;
    virtual string getRoleString() const = 0;

    // ── Authentication ──
    bool authenticate(const string& uname, const string& pwd) const;

    // ── Getters (Inline) ──
    inline int getUserId() const { return userId; }
    inline string getUsername() const { return username; }
    inline string getName() const { return name; }
    inline string getEmail() const { return email; }
    inline string getPhone() const { return phone; }

    // ── Setters (using this pointer) ──
    void setUsername(const string& u) { this->username = u; }
    void setPassword(const string& p) { this->password = p; }
    void setName(const string& n) { this->name = n; }
    void setEmail(const string& e) { this->email = e; }
    void setPhone(const string& p) { this->phone = p; }

    // ── Operator Overloading ──
    bool operator==(const User& other) const;
    friend ostream& operator<<(ostream& os, const User& u);

    // ── Static ──
    static int getTotalUsers() { return nextUserId - 1; }
};
