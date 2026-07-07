#pragma once
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cctype>

// ══════════════════════════════════════════════════════════
//  JsonUtil — Lightweight JSON Builder & Parser
//  Header-only, no external dependencies
//  Demonstrates: Namespaces, inline functions, templates
// ══════════════════════════════════════════════════════════

namespace Json {

    // ── Escape special characters ──
    inline std::string escape(const std::string& s) {
        std::string r;
        r.reserve(s.length() + 10);
        for (char c : s) {
            switch (c) {
                case '"':  r += "\\\""; break;
                case '\\': r += "\\\\"; break;
                case '\n': r += "\\n"; break;
                case '\r': r += "\\r"; break;
                case '\t': r += "\\t"; break;
                default:   r += c;
            }
        }
        return r;
    }

    // ── Value converters ──
    inline std::string str(const std::string& s) { return "\"" + escape(s) + "\""; }
    inline std::string num(int n) { return std::to_string(n); }
    inline std::string num(double n) { std::ostringstream o; o << n; return o.str(); }
    inline std::string boolean(bool b) { return b ? "true" : "false"; }
    inline std::string null() { return "null"; }

    // ── Build JSON object from key-value pairs ──
    inline std::string object(const std::vector<std::pair<std::string, std::string>>& fields) {
        std::string r = "{";
        for (size_t i = 0; i < fields.size(); i++) {
            if (i > 0) r += ",";
            r += "\"" + escape(fields[i].first) + "\":" + fields[i].second;
        }
        return r + "}";
    }

    // ── Build JSON array ──
    inline std::string array(const std::vector<std::string>& items) {
        std::string r = "[";
        for (size_t i = 0; i < items.size(); i++) {
            if (i > 0) r += ",";
            r += items[i];
        }
        return r + "]";
    }

    // ── Parse flat JSON object → map<string, string> ──
    // String values are returned WITHOUT quotes
    // Numbers/booleans returned as-is
    inline std::map<std::string, std::string> parse(const std::string& json) {
        std::map<std::string, std::string> result;
        size_t i = json.find('{');
        if (i == std::string::npos) return result;
        i++;

        while (i < json.length()) {
            while (i < json.length() && std::isspace(json[i])) i++;
            if (i >= json.length() || json[i] == '}') break;
            if (json[i] == ',') { i++; continue; }

            // Read key
            if (json[i] != '"') break;
            i++;
            std::string key;
            while (i < json.length() && json[i] != '"') {
                if (json[i] == '\\' && i + 1 < json.length()) { key += json[i + 1]; i += 2; }
                else { key += json[i]; i++; }
            }
            if (i < json.length()) i++;

            while (i < json.length() && (std::isspace(json[i]) || json[i] == ':')) i++;

            // Read value
            std::string value;
            if (i < json.length() && json[i] == '"') {
                i++;
                while (i < json.length() && json[i] != '"') {
                    if (json[i] == '\\' && i + 1 < json.length()) {
                        if (json[i+1] == '"') value += '"';
                        else if (json[i+1] == '\\') value += '\\';
                        else if (json[i+1] == 'n') value += '\n';
                        else if (json[i+1] == 't') value += '\t';
                        else value += json[i+1];
                        i += 2;
                    } else { value += json[i]; i++; }
                }
                if (i < json.length()) i++;
            } else if (i < json.length() && (json[i] == '{' || json[i] == '[')) {
                char open = json[i], close = (open == '{') ? '}' : ']';
                int depth = 1;
                value += json[i]; i++;
                while (i < json.length() && depth > 0) {
                    if (json[i] == '"') {
                        value += json[i]; i++;
                        while (i < json.length() && json[i] != '"') {
                            if (json[i] == '\\' && i + 1 < json.length()) { value += json[i]; value += json[i+1]; i += 2; }
                            else { value += json[i]; i++; }
                        }
                        if (i < json.length()) { value += json[i]; i++; }
                    } else {
                        if (json[i] == open) depth++;
                        else if (json[i] == close) depth--;
                        value += json[i]; i++;
                    }
                }
            } else {
                while (i < json.length() && json[i] != ',' && json[i] != '}' && !std::isspace(json[i])) {
                    value += json[i]; i++;
                }
            }
            result[key] = value;
        }
        return result;
    }

    // ── Helper to get values from parsed map ──
    inline std::string get(const std::map<std::string, std::string>& m,
                           const std::string& key, const std::string& def = "") {
        auto it = m.find(key);
        return it != m.end() ? it->second : def;
    }

    inline int getInt(const std::map<std::string, std::string>& m,
                      const std::string& key, int def = 0) {
        auto it = m.find(key);
        if (it == m.end() || it->second.empty()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    }

    inline double getDouble(const std::map<std::string, std::string>& m,
                            const std::string& key, double def = 0.0) {
        auto it = m.find(key);
        if (it == m.end() || it->second.empty()) return def;
        try { return std::stod(it->second); } catch (...) { return def; }
    }

} // namespace Json
