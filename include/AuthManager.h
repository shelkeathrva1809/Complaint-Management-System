#pragma once
#include <string>

// ══════════════════════════════════════════════════════════
//  AuthManager — Authentication & Token Management
//  Demonstrates: Encapsulation, Composition, Nested struct
// ══════════════════════════════════════════════════════════

class AuthManager {
private:
    std::string jwtSecret;

    // Simple hash function (DJB2 variant + salt)
    std::string hashString(const std::string& input);
    std::string base64Encode(const std::string& data);
    std::string base64Decode(const std::string& data);

public:
    AuthManager(const std::string& secret = "spcms_secret_key_2026");

    // Password operations
    std::string createSalt();
    std::string hashPassword(const std::string& password, const std::string& salt);
    bool verifyPassword(const std::string& password,
                        const std::string& hash, const std::string& salt);

    // Token operations
    struct TokenPayload {
        int userId;
        std::string role;
        std::string name;
        bool valid;
    };

    std::string generateToken(int userId, const std::string& role, const std::string& name);
    TokenPayload verifyToken(const std::string& token);
};
