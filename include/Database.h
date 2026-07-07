#pragma once
#include <string>
#include <vector>
#include <map>

// Forward declare sqlite3 types to avoid including sqlite3.h in header
struct sqlite3;
struct sqlite3_stmt;

// ══════════════════════════════════════════════════════════
//  Database — SQLite Wrapper (Singleton Pattern)
//  Demonstrates: Singleton, RAII, Encapsulation, Composition
// ══════════════════════════════════════════════════════════

struct UserRecord {
    int id = 0;
    std::string name, email, phone;
    std::string passwordHash, salt;
    std::string role, department;
    std::string createdAt;
};

struct ComplaintRecord {
    int id = 0;
    std::string trackingId;
    int userId = 0;
    std::string citizenName, citizenEmail, citizenPhone;
    std::string type, description, location;
    std::string severity, status;
    int assignedOfficer = 0;
    std::string officerName, officerEmail;
    std::string attachment, resolutionProof;
    int rating = 0;
    std::string feedback;
    std::string createdAt, updatedAt;
};

struct NotificationRecord {
    int id = 0;
    int userId = 0;
    int complaintId = 0;
    std::string message;
    bool isRead = false;
    std::string createdAt;
};

class Database {
private:
    sqlite3* db;
    static Database* instance;

    Database();
    void createTables();
    void exec(const std::string& sql);

public:
    ~Database();

    // Singleton access
    static Database& getInstance();
    void initialize(const std::string& path = "data/spcms.db");

    // ── User Operations ──
    int createUser(const std::string& name, const std::string& email,
                   const std::string& phone, const std::string& hash,
                   const std::string& salt, const std::string& role,
                   const std::string& department = "");
    UserRecord getUserByEmail(const std::string& email);
    UserRecord getUserById(int id);
    std::vector<UserRecord> getUsersByRole(const std::string& role);
    std::vector<UserRecord> getAllUsers();
    bool emailExists(const std::string& email);
    bool deleteUser(int id);

    // ── Complaint Operations ──
    int createComplaint(int userId, const std::string& type,
                       const std::string& desc, const std::string& location,
                       const std::string& severity, const std::string& citizenName,
                       const std::string& citizenEmail, const std::string& citizenPhone,
                       const std::string& attachment = "");
    ComplaintRecord getComplaintById(int id);
    std::vector<ComplaintRecord> getComplaintsByUser(int userId);
    std::vector<ComplaintRecord> getComplaintsByOfficer(int officerId);
    std::vector<ComplaintRecord> getAllComplaints();
    std::vector<ComplaintRecord> getComplaintsByStatus(const std::string& status);
    std::vector<ComplaintRecord> getComplaintsByType(const std::string& type);
    bool updateComplaintStatus(int id, const std::string& status);
    bool assignOfficer(int complaintId, int officerId);
    bool submitResolution(int id, const std::string& proof);
    bool acceptResolution(int id);
    bool rejectResolution(int id);
    bool rateComplaint(int id, int rating, const std::string& feedback);
    int getComplaintCount();
    std::string generateTrackingId();

    // ── Analytics ──
    std::map<std::string, int> getStatsSummary();
    std::map<std::string, int> getComplaintsByCategory();
    std::map<std::string, int> getStatusDistribution();
    std::vector<std::pair<std::string, int>> getComplaintsOverTime(int days = 30);
    std::vector<std::pair<std::string, double>> getOfficerRatings();

    // ── Notifications ──
    int addNotification(int userId, int complaintId, const std::string& msg);
    std::vector<NotificationRecord> getNotifications(int userId);
    int getUnreadCount(int userId);
    bool markNotificationRead(int id);
    bool markAllRead(int userId);

    // ── Seeding ──
    void seedDemoData();
};
