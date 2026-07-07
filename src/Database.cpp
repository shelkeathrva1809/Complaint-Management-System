#include "../include/Database.h"
#include "../include/sqlite3.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cstdlib>

// ══════════════════════════════════════════════════════════
//  Database Implementation — Singleton + SQLite
// ══════════════════════════════════════════════════════════

Database* Database::instance = nullptr;

Database::Database() : db(nullptr) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Database::~Database() {
    if (db) sqlite3_close(db);
}

Database& Database::getInstance() {
    if (!instance) instance = new Database();
    return *instance;
}

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "Unknown DB error";
        if (err) sqlite3_free(err);
        throw std::runtime_error("SQL error: " + msg);
    }
}

void Database::initialize(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db)));
    }
    exec("PRAGMA journal_mode=WAL;");
    exec("PRAGMA foreign_keys=ON;");
    exec("PRAGMA busy_timeout=5000;");  // Wait up to 5s if DB is locked (thread safety)
    createTables();
}

void Database::createTables() {
    exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            phone TEXT NOT NULL,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            role TEXT NOT NULL CHECK(role IN ('citizen','officer','senior_officer','admin')),
            department TEXT DEFAULT '',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )");
    exec(R"(
        CREATE TABLE IF NOT EXISTS complaints (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            tracking_id TEXT UNIQUE,
            user_id INTEGER NOT NULL,
            citizen_name TEXT NOT NULL,
            citizen_email TEXT NOT NULL,
            citizen_phone TEXT NOT NULL,
            type TEXT NOT NULL CHECK(type IN ('Road','Water','Electricity')),
            description TEXT NOT NULL,
            location TEXT DEFAULT '',
            severity TEXT NOT NULL CHECK(severity IN ('Low','Normal','Emergency')),
            status TEXT DEFAULT 'Pending',
            assigned_officer INTEGER DEFAULT 0,
            attachment TEXT DEFAULT '',
            resolution_proof TEXT DEFAULT '',
            rating INTEGER DEFAULT 0,
            feedback TEXT DEFAULT '',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id)
        );
    )");
    exec(R"(
        CREATE TABLE IF NOT EXISTS notifications (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            complaint_id INTEGER DEFAULT 0,
            message TEXT NOT NULL,
            is_read INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id)
        );
    )");
}

// ── Helper: Get current timestamp (UTC for consistency) ──
static std::string nowTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::gmtime(&t);  // UTC — consistent with frontend parsing
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
    return std::string(buf);
}

// ══════════════════════════════════════════
//  USER OPERATIONS
// ══════════════════════════════════════════

int Database::createUser(const std::string& name, const std::string& email,
                         const std::string& phone, const std::string& hash,
                         const std::string& salt, const std::string& role,
                         const std::string& department) {
    const char* sql = "INSERT INTO users (name,email,phone,password_hash,salt,role,department) VALUES (?,?,?,?,?,?,?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, department.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return -1;
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

UserRecord Database::getUserByEmail(const std::string& email) {
    UserRecord u;
    const char* sql = "SELECT id,name,email,phone,password_hash,salt,role,department,created_at FROM users WHERE email=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return u;
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        u.id = sqlite3_column_int(stmt, 0);
        u.name = (const char*)sqlite3_column_text(stmt, 1);
        u.email = (const char*)sqlite3_column_text(stmt, 2);
        u.phone = (const char*)sqlite3_column_text(stmt, 3);
        u.passwordHash = (const char*)sqlite3_column_text(stmt, 4);
        u.salt = (const char*)sqlite3_column_text(stmt, 5);
        u.role = (const char*)sqlite3_column_text(stmt, 6);
        u.department = sqlite3_column_text(stmt, 7) ? (const char*)sqlite3_column_text(stmt, 7) : "";
        u.createdAt = sqlite3_column_text(stmt, 8) ? (const char*)sqlite3_column_text(stmt, 8) : "";
    }
    sqlite3_finalize(stmt);
    return u;
}

UserRecord Database::getUserById(int id) {
    UserRecord u;
    const char* sql = "SELECT id,name,email,phone,password_hash,salt,role,department,created_at FROM users WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return u;
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        u.id = sqlite3_column_int(stmt, 0);
        u.name = (const char*)sqlite3_column_text(stmt, 1);
        u.email = (const char*)sqlite3_column_text(stmt, 2);
        u.phone = (const char*)sqlite3_column_text(stmt, 3);
        u.passwordHash = (const char*)sqlite3_column_text(stmt, 4);
        u.salt = (const char*)sqlite3_column_text(stmt, 5);
        u.role = (const char*)sqlite3_column_text(stmt, 6);
        u.department = sqlite3_column_text(stmt, 7) ? (const char*)sqlite3_column_text(stmt, 7) : "";
        u.createdAt = sqlite3_column_text(stmt, 8) ? (const char*)sqlite3_column_text(stmt, 8) : "";
    }
    sqlite3_finalize(stmt);
    return u;
}

std::vector<UserRecord> Database::getUsersByRole(const std::string& role) {
    std::vector<UserRecord> users;
    const char* sql = "SELECT id,name,email,phone,password_hash,salt,role,department,created_at FROM users WHERE role=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return users;
    sqlite3_bind_text(stmt, 1, role.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserRecord u;
        u.id = sqlite3_column_int(stmt, 0);
        u.name = (const char*)sqlite3_column_text(stmt, 1);
        u.email = (const char*)sqlite3_column_text(stmt, 2);
        u.phone = (const char*)sqlite3_column_text(stmt, 3);
        u.passwordHash = (const char*)sqlite3_column_text(stmt, 4);
        u.salt = (const char*)sqlite3_column_text(stmt, 5);
        u.role = (const char*)sqlite3_column_text(stmt, 6);
        u.department = sqlite3_column_text(stmt, 7) ? (const char*)sqlite3_column_text(stmt, 7) : "";
        u.createdAt = sqlite3_column_text(stmt, 8) ? (const char*)sqlite3_column_text(stmt, 8) : "";
        users.push_back(u);
    }
    sqlite3_finalize(stmt);
    return users;
}

std::vector<UserRecord> Database::getAllUsers() {
    std::vector<UserRecord> users;
    const char* sql = "SELECT id,name,email,phone,password_hash,salt,role,department,created_at FROM users";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return users;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserRecord u;
        u.id = sqlite3_column_int(stmt, 0);
        u.name = (const char*)sqlite3_column_text(stmt, 1);
        u.email = (const char*)sqlite3_column_text(stmt, 2);
        u.phone = (const char*)sqlite3_column_text(stmt, 3);
        u.passwordHash = (const char*)sqlite3_column_text(stmt, 4);
        u.salt = (const char*)sqlite3_column_text(stmt, 5);
        u.role = (const char*)sqlite3_column_text(stmt, 6);
        u.department = sqlite3_column_text(stmt, 7) ? (const char*)sqlite3_column_text(stmt, 7) : "";
        u.createdAt = sqlite3_column_text(stmt, 8) ? (const char*)sqlite3_column_text(stmt, 8) : "";
        users.push_back(u);
    }
    sqlite3_finalize(stmt);
    return users;
}

bool Database::emailExists(const std::string& email) {
    const char* sql = "SELECT COUNT(*) FROM users WHERE email=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) exists = sqlite3_column_int(stmt, 0) > 0;
    sqlite3_finalize(stmt);
    return exists;
}

bool Database::deleteUser(int id) {
    const char* sql = "DELETE FROM users WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// ══════════════════════════════════════════
//  COMPLAINT OPERATIONS
// ══════════════════════════════════════════

std::string Database::generateTrackingId() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::gmtime(&t);  // UTC
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y", tm);
    // Use sequential count + small random to guarantee uniqueness
    int count = getComplaintCount() + 1;
    int random = std::rand() % 1000;
    return "SPCMS-" + std::string(buf) + "-" + std::to_string(count) + std::to_string(random);
}

int Database::createComplaint(int userId, const std::string& type,
                              const std::string& desc, const std::string& location,
                              const std::string& severity, const std::string& citizenName,
                              const std::string& citizenEmail, const std::string& citizenPhone,
                              const std::string& attachment) {

    // Auto-assign officer: pick officer with fewest complaints in matching department
    std::string dept = "";
    if (type == "Road") dept = "PWD";
    else if (type == "Water") dept = "Water Supply";
    else if (type == "Electricity") dept = "Electricity Board";

    int officerId = 0;
    if (!dept.empty()) {
        const char* oSql = "SELECT u.id FROM users u LEFT JOIN "
            "(SELECT assigned_officer, COUNT(*) as cnt FROM complaints GROUP BY assigned_officer) c "
            "ON u.id = c.assigned_officer WHERE u.role='officer' AND u.department=? "
            "ORDER BY COALESCE(c.cnt, 0) ASC LIMIT 1";
        sqlite3_stmt* oStmt;
        if (sqlite3_prepare_v2(db, oSql, -1, &oStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(oStmt, 1, dept.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(oStmt) == SQLITE_ROW) officerId = sqlite3_column_int(oStmt, 0);
            sqlite3_finalize(oStmt);
        }
    }

    const char* sql = "INSERT INTO complaints (tracking_id,user_id,citizen_name,citizen_email,"
        "citizen_phone,type,description,location,severity,status,assigned_officer,attachment) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?)";

    // Retry loop: if tracking_id collides (UNIQUE constraint), regenerate and retry
    const int MAX_RETRIES = 5;
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        std::string trackingId = generateTrackingId();

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
        sqlite3_bind_text(stmt, 1, trackingId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, userId);
        sqlite3_bind_text(stmt, 3, citizenName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, citizenEmail.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, citizenPhone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, desc.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, location.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, severity.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 10, officerId > 0 ? "Assigned" : "Pending", -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 11, officerId);
        sqlite3_bind_text(stmt, 12, attachment.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc == SQLITE_DONE) {
            // Success — get the new complaint ID and notify officer
            int cid = static_cast<int>(sqlite3_last_insert_rowid(db));
            if (officerId > 0) {
                addNotification(officerId, cid, "New complaint #" + std::to_string(cid) + " assigned to you: " + type);
            }
            return cid;
        }
        // If it failed due to UNIQUE constraint, retry with new tracking ID
        // Otherwise it's a different error, break
        if (sqlite3_errcode(db) != SQLITE_CONSTRAINT) return -1;
    }
    return -1;  // All retries exhausted
}

ComplaintRecord Database::getComplaintById(int id) {
    ComplaintRecord c;
    const char* sql = "SELECT c.*, COALESCE(u.name,'Unassigned') as officer_name, COALESCE(u.email,'') as officer_email FROM complaints c "
        "LEFT JOIN users u ON c.assigned_officer=u.id WHERE c.id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return c;
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        c.id = sqlite3_column_int(stmt, 0);
        c.trackingId = sqlite3_column_text(stmt, 1) ? (const char*)sqlite3_column_text(stmt, 1) : "";
        c.userId = sqlite3_column_int(stmt, 2);
        c.citizenName = (const char*)sqlite3_column_text(stmt, 3);
        c.citizenEmail = (const char*)sqlite3_column_text(stmt, 4);
        c.citizenPhone = (const char*)sqlite3_column_text(stmt, 5);
        c.type = (const char*)sqlite3_column_text(stmt, 6);
        c.description = (const char*)sqlite3_column_text(stmt, 7);
        c.location = sqlite3_column_text(stmt, 8) ? (const char*)sqlite3_column_text(stmt, 8) : "";
        c.severity = (const char*)sqlite3_column_text(stmt, 9);
        c.status = (const char*)sqlite3_column_text(stmt, 10);
        c.assignedOfficer = sqlite3_column_int(stmt, 11);
        c.attachment = sqlite3_column_text(stmt, 12) ? (const char*)sqlite3_column_text(stmt, 12) : "";
        c.resolutionProof = sqlite3_column_text(stmt, 13) ? (const char*)sqlite3_column_text(stmt, 13) : "";
        c.rating = sqlite3_column_int(stmt, 14);
        c.feedback = sqlite3_column_text(stmt, 15) ? (const char*)sqlite3_column_text(stmt, 15) : "";
        c.createdAt = sqlite3_column_text(stmt, 16) ? (const char*)sqlite3_column_text(stmt, 16) : "";
        c.updatedAt = sqlite3_column_text(stmt, 17) ? (const char*)sqlite3_column_text(stmt, 17) : "";
        c.officerName = (const char*)sqlite3_column_text(stmt, 18);
        c.officerEmail = sqlite3_column_text(stmt, 19) ? (const char*)sqlite3_column_text(stmt, 19) : "";
    }
    sqlite3_finalize(stmt);
    return c;
}

static std::vector<ComplaintRecord> queryComplaints(sqlite3* db, const std::string& sql, 
    const std::string& bindText = "", int bindInt = -1) {
    std::vector<ComplaintRecord> results;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return results;
    if (!bindText.empty()) sqlite3_bind_text(stmt, 1, bindText.c_str(), -1, SQLITE_TRANSIENT);
    else if (bindInt >= 0) sqlite3_bind_int(stmt, 1, bindInt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ComplaintRecord c;
        c.id = sqlite3_column_int(stmt, 0);
        c.trackingId = sqlite3_column_text(stmt, 1) ? (const char*)sqlite3_column_text(stmt, 1) : "";
        c.userId = sqlite3_column_int(stmt, 2);
        c.citizenName = (const char*)sqlite3_column_text(stmt, 3);
        c.citizenEmail = (const char*)sqlite3_column_text(stmt, 4);
        c.citizenPhone = (const char*)sqlite3_column_text(stmt, 5);
        c.type = (const char*)sqlite3_column_text(stmt, 6);
        c.description = (const char*)sqlite3_column_text(stmt, 7);
        c.location = sqlite3_column_text(stmt, 8) ? (const char*)sqlite3_column_text(stmt, 8) : "";
        c.severity = (const char*)sqlite3_column_text(stmt, 9);
        c.status = (const char*)sqlite3_column_text(stmt, 10);
        c.assignedOfficer = sqlite3_column_int(stmt, 11);
        c.attachment = sqlite3_column_text(stmt, 12) ? (const char*)sqlite3_column_text(stmt, 12) : "";
        c.resolutionProof = sqlite3_column_text(stmt, 13) ? (const char*)sqlite3_column_text(stmt, 13) : "";
        c.rating = sqlite3_column_int(stmt, 14);
        c.feedback = sqlite3_column_text(stmt, 15) ? (const char*)sqlite3_column_text(stmt, 15) : "";
        c.createdAt = sqlite3_column_text(stmt, 16) ? (const char*)sqlite3_column_text(stmt, 16) : "";
        c.updatedAt = sqlite3_column_text(stmt, 17) ? (const char*)sqlite3_column_text(stmt, 17) : "";
        c.officerName = sqlite3_column_text(stmt, 18) ? (const char*)sqlite3_column_text(stmt, 18) : "Unassigned";
        c.officerEmail = sqlite3_column_text(stmt, 19) ? (const char*)sqlite3_column_text(stmt, 19) : "";
        results.push_back(c);
    }
    sqlite3_finalize(stmt);
    return results;
}

static const std::string COMPLAINT_QUERY_BASE =
    "SELECT c.*, COALESCE(u.name,'Unassigned') as officer_name, COALESCE(u.email,'') as officer_email FROM complaints c "
    "LEFT JOIN users u ON c.assigned_officer=u.id";

std::vector<ComplaintRecord> Database::getComplaintsByUser(int userId) {
    return queryComplaints(db, COMPLAINT_QUERY_BASE + " WHERE c.user_id=? ORDER BY c.created_at DESC", "", userId);
}

std::vector<ComplaintRecord> Database::getComplaintsByOfficer(int officerId) {
    return queryComplaints(db, COMPLAINT_QUERY_BASE + " WHERE c.assigned_officer=? ORDER BY c.created_at DESC", "", officerId);
}

std::vector<ComplaintRecord> Database::getAllComplaints() {
    return queryComplaints(db, COMPLAINT_QUERY_BASE + " ORDER BY c.created_at DESC");
}

std::vector<ComplaintRecord> Database::getComplaintsByStatus(const std::string& status) {
    return queryComplaints(db, COMPLAINT_QUERY_BASE + " WHERE c.status=? ORDER BY c.created_at DESC", status);
}

std::vector<ComplaintRecord> Database::getComplaintsByType(const std::string& type) {
    return queryComplaints(db, COMPLAINT_QUERY_BASE + " WHERE c.type=? ORDER BY c.created_at DESC", type);
}

bool Database::updateComplaintStatus(int id, const std::string& status) {
    const char* sql = "UPDATE complaints SET status=?, updated_at=? WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    std::string now = nowTimestamp();
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Notify citizen
    if (rc == SQLITE_DONE) {
        ComplaintRecord c = getComplaintById(id);
        if (c.userId > 0) {
            addNotification(c.userId, id, "Complaint #" + std::to_string(id) + " status updated to: " + status);
        }
    }
    return rc == SQLITE_DONE;
}

bool Database::assignOfficer(int complaintId, int officerId) {
    const char* sql = "UPDATE complaints SET assigned_officer=?, status='Assigned', updated_at=? WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    std::string now = nowTimestamp();
    sqlite3_bind_int(stmt, 1, officerId);
    sqlite3_bind_text(stmt, 2, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, complaintId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
        addNotification(officerId, complaintId, "Complaint #" + std::to_string(complaintId) + " assigned to you");
    }
    return rc == SQLITE_DONE;
}

bool Database::submitResolution(int id, const std::string& proof) {
    const char* sql = "UPDATE complaints SET resolution_proof=?, status='Resolved', updated_at=? WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    std::string now = nowTimestamp();
    sqlite3_bind_text(stmt, 1, proof.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
        ComplaintRecord c = getComplaintById(id);
        addNotification(c.userId, id, "Resolution submitted for complaint #" + std::to_string(id) + ". Please review.");
    }
    return rc == SQLITE_DONE;
}

bool Database::acceptResolution(int id) {
    return updateComplaintStatus(id, "Closed");
}

bool Database::rejectResolution(int id) {
    const char* sql = "UPDATE complaints SET resolution_proof='', status='In Progress', updated_at=? WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    std::string now = nowTimestamp();
    sqlite3_bind_text(stmt, 1, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool Database::rateComplaint(int id, int rating, const std::string& feedback) {
    const char* sql = "UPDATE complaints SET rating=?, feedback=?, updated_at=? WHERE id=?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    std::string now = nowTimestamp();
    sqlite3_bind_int(stmt, 1, rating);
    sqlite3_bind_text(stmt, 2, feedback.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

int Database::getComplaintCount() {
    const char* sql = "SELECT COUNT(*) FROM complaints";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}

// ══════════════════════════════════════════
//  ANALYTICS
// ══════════════════════════════════════════

std::map<std::string, int> Database::getStatsSummary() {
    std::map<std::string, int> stats;
    stats["total"] = getComplaintCount();
    const char* sql = "SELECT status, COUNT(*) FROM complaints GROUP BY status";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string s = (const char*)sqlite3_column_text(stmt, 0);
            stats[s] = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }
    // Severity counts
    const char* sql2 = "SELECT severity, COUNT(*) FROM complaints GROUP BY severity";
    if (sqlite3_prepare_v2(db, sql2, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string s = (const char*)sqlite3_column_text(stmt, 0);
            stats["sev_" + s] = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }
    return stats;
}

std::map<std::string, int> Database::getComplaintsByCategory() {
    std::map<std::string, int> cats;
    const char* sql = "SELECT type, COUNT(*) FROM complaints GROUP BY type";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            cats[(const char*)sqlite3_column_text(stmt, 0)] = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }
    return cats;
}

std::map<std::string, int> Database::getStatusDistribution() {
    std::map<std::string, int> dist;
    const char* sql = "SELECT status, COUNT(*) FROM complaints GROUP BY status";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            dist[(const char*)sqlite3_column_text(stmt, 0)] = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }
    return dist;
}

std::vector<std::pair<std::string, int>> Database::getComplaintsOverTime(int days) {
    std::vector<std::pair<std::string, int>> timeline;
    std::string sql = "SELECT DATE(created_at) as d, COUNT(*) FROM complaints "
        "WHERE created_at >= DATE('now','-" + std::to_string(days) + " days') "
        "GROUP BY d ORDER BY d";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string date = (const char*)sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            timeline.push_back({date, count});
        }
        sqlite3_finalize(stmt);
    }
    return timeline;
}

std::vector<std::pair<std::string, double>> Database::getOfficerRatings() {
    std::vector<std::pair<std::string, double>> ratings;
    const char* sql = "SELECT u.name, AVG(c.rating) FROM complaints c "
        "JOIN users u ON c.assigned_officer=u.id WHERE c.rating>0 GROUP BY c.assigned_officer ORDER BY AVG(c.rating) DESC";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ratings.push_back({(const char*)sqlite3_column_text(stmt, 0), sqlite3_column_double(stmt, 1)});
        }
        sqlite3_finalize(stmt);
    }
    return ratings;
}

// ══════════════════════════════════════════
//  NOTIFICATIONS
// ══════════════════════════════════════════

int Database::addNotification(int userId, int complaintId, const std::string& msg) {
    const char* sql = "INSERT INTO notifications (user_id,complaint_id,message) VALUES (?,?,?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, complaintId);
    sqlite3_bind_text(stmt, 3, msg.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

std::vector<NotificationRecord> Database::getNotifications(int userId) {
    std::vector<NotificationRecord> notes;
    const char* sql = "SELECT id,user_id,complaint_id,message,is_read,created_at FROM notifications WHERE user_id=? ORDER BY created_at DESC LIMIT 50";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return notes;
    sqlite3_bind_int(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        NotificationRecord n;
        n.id = sqlite3_column_int(stmt, 0);
        n.userId = sqlite3_column_int(stmt, 1);
        n.complaintId = sqlite3_column_int(stmt, 2);
        n.message = (const char*)sqlite3_column_text(stmt, 3);
        n.isRead = sqlite3_column_int(stmt, 4) != 0;
        n.createdAt = sqlite3_column_text(stmt, 5) ? (const char*)sqlite3_column_text(stmt, 5) : "";
        notes.push_back(n);
    }
    sqlite3_finalize(stmt);
    return notes;
}

int Database::getUnreadCount(int userId) {
    const char* sql = "SELECT COUNT(*) FROM notifications WHERE user_id=? AND is_read=0";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, userId);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}

bool Database::markNotificationRead(int id) {
    exec("UPDATE notifications SET is_read=1 WHERE id=" + std::to_string(id));
    return true;
}

bool Database::markAllRead(int userId) {
    exec("UPDATE notifications SET is_read=1 WHERE user_id=" + std::to_string(userId));
    return true;
}

// ══════════════════════════════════════════
//  DEMO DATA SEEDER
// ══════════════════════════════════════════

void Database::seedDemoData() {
    // This will be called by WebServer during initialization
    // AuthManager handles password hashing, so seeding is done from WebServer
}
