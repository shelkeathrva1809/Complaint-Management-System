#pragma once
#include <exception>
#include <string>
using namespace std;

// ══════════════════════════════════════════════════════════
//  Custom Exception Classes
//  Demonstrates: Inheritance from std::exception,
//                Custom Exceptions, Multiple Catch Blocks
// ══════════════════════════════════════════════════════════

// Thrown when user provides invalid input (wrong menu choice, empty fields, etc.)
class InvalidInputException : public exception {
private:
    string message;
public:
    InvalidInputException(const string& msg = "Invalid input provided")
        : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Thrown when a complaint ID is not found in the system
class ComplaintNotFoundException : public exception {
private:
    string message;
public:
    ComplaintNotFoundException(int id = 0)
        : message("Complaint #" + to_string(id) + " not found in system") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Thrown when escalation fails (e.g., no higher authority available)
class EscalationException : public exception {
private:
    string message;
public:
    EscalationException(const string& msg = "Escalation failed — no higher authority available")
        : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Thrown when email sending fails
class EmailException : public exception {
private:
    string message;
public:
    EmailException(const string& msg = "Email sending failed")
        : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Thrown when file I/O operations fail
class FileIOException : public exception {
private:
    string message;
public:
    FileIOException(const string& msg = "File operation failed")
        : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
