#pragma once
#include <string>

// ══════════════════════════════════════════════════════════
//  Validator — Strict Input Validation (OOP)
//  Demonstrates: Static methods, struct return types, encapsulation
// ══════════════════════════════════════════════════════════

struct ValidationResult {
    bool valid;
    std::string error;
};

class Validator {
public:
    // Citizen detail validation (strict rules)
    static ValidationResult validateName(const std::string& name);
    static ValidationResult validatePhone(const std::string& phone);
    static ValidationResult validateEmail(const std::string& email);
    static ValidationResult validatePassword(const std::string& password);

    // Complaint field validation
    static ValidationResult validateComplaintType(const std::string& type);
    static ValidationResult validateSeverity(const std::string& severity);
    static ValidationResult validateDescription(const std::string& desc);

    // Utility
    static std::string trim(const std::string& str);
    static std::string toLower(const std::string& str);
};
