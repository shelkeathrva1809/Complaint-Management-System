#include "../include/Validator.h"
#include <algorithm>
#include <cctype>

// ══════════════════════════════════════════════════════════
//  Validator Implementation
// ══════════════════════════════════════════════════════════

std::string Validator::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return str.substr(start, end - start + 1);
}

std::string Validator::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

ValidationResult Validator::validateName(const std::string& raw) {
    std::string name = trim(raw);
    if (name.length() < 2 || name.length() > 50) {
        return {false, "Invalid name. Please enter only letters"};
    }
    for (char c : name) {
        if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '.') {
            return {false, "Invalid name. Please enter only letters"};
        }
    }
    return {true, ""};
}

ValidationResult Validator::validatePhone(const std::string& raw) {
    std::string phone = trim(raw);
    if (phone.length() != 10) {
        return {false, "Invalid phone number. Enter a 10-digit valid number"};
    }
    for (char c : phone) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return {false, "Invalid phone number. Enter a 10-digit valid number"};
        }
    }
    if (phone[0] < '6' || phone[0] > '9') {
        return {false, "Invalid phone number. Enter a 10-digit valid number"};
    }
    return {true, ""};
}

ValidationResult Validator::validateEmail(const std::string& raw) {
    std::string email = toLower(trim(raw));
    if (email.empty()) {
        return {false, "Email is required"};
    }
    size_t atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0) {
        return {false, "Please enter a valid email address"};
    }
    std::string domain = email.substr(atPos + 1);
    // Must have at least one dot in domain
    size_t dotPos = domain.find('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == domain.length() - 1) {
        return {false, "Please enter a valid email address"};
    }
    std::string local = email.substr(0, atPos);
    for (char c : local) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '.' && c != '_' && c != '-' && c != '+') {
            return {false, "Please enter a valid email address"};
        }
    }
    return {true, ""};
}

ValidationResult Validator::validatePassword(const std::string& raw) {
    std::string password = trim(raw);
    if (password.length() < 8) {
        return {false, "Password must be at least 8 characters"};
    }
    bool hasUpper = false, hasDigit = false;
    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
    }
    if (!hasUpper) return {false, "Password must contain at least one uppercase letter"};
    if (!hasDigit) return {false, "Password must contain at least one number"};
    return {true, ""};
}

ValidationResult Validator::validateComplaintType(const std::string& raw) {
    std::string type = trim(raw);
    if (type != "Road" && type != "Water" && type != "Electricity") {
        return {false, "Invalid complaint type. Choose Road, Water, or Electricity"};
    }
    return {true, ""};
}

ValidationResult Validator::validateSeverity(const std::string& raw) {
    std::string sev = trim(raw);
    if (sev != "Low" && sev != "Normal" && sev != "Emergency") {
        return {false, "Invalid severity. Choose Low, Normal, or Emergency"};
    }
    return {true, ""};
}

ValidationResult Validator::validateDescription(const std::string& raw) {
    std::string desc = trim(raw);
    if (desc.length() < 10 || desc.length() > 500) {
        return {false, "Description must be 10-500 characters"};
    }
    return {true, ""};
}
