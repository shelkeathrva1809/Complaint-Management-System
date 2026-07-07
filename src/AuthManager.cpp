#include "../include/AuthManager.h"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <vector>

// ══════════════════════════════════════════════════════════
//  AuthManager Implementation
// ══════════════════════════════════════════════════════════

AuthManager::AuthManager(const std::string& secret) : jwtSecret(secret) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

// ── DJB2 hash variant (deterministic, fast) ──
std::string AuthManager::hashString(const std::string& input) {
    unsigned long long hash1 = 5381;
    unsigned long long hash2 = 52711;
    for (size_t i = 0; i < input.length(); i++) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        hash1 = ((hash1 << 5) + hash1) ^ c;
        hash2 = ((hash2 << 5) + hash2) ^ c;
        // Mix with position
        hash1 ^= (hash1 >> 16);
        hash2 += (hash2 << 3);
    }
    // Additional mixing rounds for better distribution
    hash1 ^= (hash1 >> 13);
    hash1 *= 0xff51afd7ed558ccdULL;
    hash1 ^= (hash1 >> 33);
    hash2 ^= (hash2 >> 17);
    hash2 *= 0xc4ceb9fe1a85ec53ULL;
    hash2 ^= (hash2 >> 31);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << hash1 << std::setw(16) << hash2;
    return oss.str();
}

// ── Base64 encoding ──
static const std::string BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string AuthManager::base64Encode(const std::string& data) {
    std::string result;
    int i = 0, val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result += BASE64_CHARS[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) result += BASE64_CHARS[((val << 8) >> (valb + 8)) & 0x3F];
    while (result.size() % 4) result += '=';
    return result;
}

std::string AuthManager::base64Decode(const std::string& data) {
    std::string result;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[BASE64_CHARS[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : data) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            result += char((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return result;
}

// ── Salt generation ──
std::string AuthManager::createSalt() {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string salt;
    for (int i = 0; i < 16; i++) {
        salt += chars[std::rand() % chars.length()];
    }
    return salt;
}

// ── Password hashing (hash + salt + pepper) ──
std::string AuthManager::hashPassword(const std::string& password, const std::string& salt) {
    std::string combined = salt + password + jwtSecret + "pepper_spcms";
    // Multiple rounds for strength
    std::string hash = combined;
    for (int round = 0; round < 100; round++) {
        hash = hashString(hash + salt + std::to_string(round));
    }
    return hash;
}

bool AuthManager::verifyPassword(const std::string& password,
                                  const std::string& hash,
                                  const std::string& salt) {
    return hashPassword(password, salt) == hash;
}

// ── Token generation (custom JWT-like format) ──
// Format: base64(userId:role:name:timestamp).signature
std::string AuthManager::generateToken(int userId, const std::string& role,
                                        const std::string& name) {
    std::time_t now = std::time(nullptr);
    std::string payload = std::to_string(userId) + ":"
                        + role + ":"
                        + name + ":"
                        + std::to_string(now);

    std::string encodedPayload = base64Encode(payload);
    std::string signature = hashString(encodedPayload + jwtSecret);

    return encodedPayload + "." + signature;
}

AuthManager::TokenPayload AuthManager::verifyToken(const std::string& token) {
    TokenPayload result{0, "", "", false};

    size_t dot = token.find('.');
    if (dot == std::string::npos) return result;

    std::string encodedPayload = token.substr(0, dot);
    std::string signature = token.substr(dot + 1);

    // Verify signature
    std::string expectedSig = hashString(encodedPayload + jwtSecret);
    if (signature != expectedSig) return result;

    // Decode payload
    std::string payload = base64Decode(encodedPayload);
    std::istringstream ss(payload);
    std::string part;
    std::vector<std::string> parts;
    while (std::getline(ss, part, ':')) {
        parts.push_back(part);
    }

    if (parts.size() < 4) return result;

    try {
        result.userId = std::stoi(parts[0]);
        result.role = parts[1];
        // Name might contain colons, rejoin remaining parts
        result.name = parts[2];
        for (size_t i = 3; i < parts.size() - 1; i++) {
            result.name += ":" + parts[i];
        }
        // Check token age (24 hour expiry)
        std::time_t tokenTime = std::stoll(parts.back());
        std::time_t now = std::time(nullptr);
        if (now - tokenTime > 86400) return result; // Expired

        result.valid = true;
    } catch (...) {
        return result;
    }

    return result;
}
