#include "../include/WebServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <ctime>

// ══════════════════════════════════════════════════════════
//  WebServer Implementation — All REST API Routes
// ══════════════════════════════════════════════════════════

WebServer::WebServer(int p) : db(Database::getInstance()), auth("spcms_secret_2026"), port(p) {}

// ── CORS Headers ──
void WebServer::setCorsHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET,POST,PATCH,DELETE,OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type,Authorization");
}

// ── Auth middleware ──
AuthManager::TokenPayload WebServer::authenticate(const httplib::Request& req) {
    std::string header = req.get_header_value("Authorization");
    if (header.substr(0, 7) == "Bearer ") {
        return auth.verifyToken(header.substr(7));
    }
    return {0, "", "", false};
}

bool WebServer::requireAuth(const httplib::Request& req, httplib::Response& res) {
    auto token = authenticate(req);
    if (!token.valid) { sendError(res, 401, "Authentication required"); return false; }
    return true;
}

bool WebServer::requireRole(const httplib::Request& req, httplib::Response& res, const std::string& role) {
    auto token = authenticate(req);
    if (!token.valid) { sendError(res, 401, "Authentication required"); return false; }
    if (token.role != role && token.role != "admin") { sendError(res, 403, "Access denied"); return false; }
    return true;
}

// ── JSON Helpers ──
void WebServer::sendError(httplib::Response& res, int code, const std::string& msg) {
    setCorsHeaders(res);
    res.status = code;
    res.set_content(Json::object({{"success", Json::boolean(false)}, {"error", Json::str(msg)}}), "application/json");
}

void WebServer::sendSuccess(httplib::Response& res, const std::string& data) {
    setCorsHeaders(res);
    res.status = 200;
    res.set_content(data, "application/json");
}

std::string WebServer::complaintToJson(const ComplaintRecord& c) {
    return Json::object({
        {"id", Json::num(c.id)}, {"trackingId", Json::str(c.trackingId)},
        {"userId", Json::num(c.userId)}, {"citizenName", Json::str(c.citizenName)},
        {"citizenEmail", Json::str(c.citizenEmail)}, {"citizenPhone", Json::str(c.citizenPhone)},
        {"type", Json::str(c.type)}, {"description", Json::str(c.description)},
        {"location", Json::str(c.location)}, {"severity", Json::str(c.severity)},
        {"status", Json::str(c.status)}, {"assignedOfficer", Json::num(c.assignedOfficer)},
        {"officerName", Json::str(c.officerName)}, {"officerEmail", Json::str(c.officerEmail)},
        {"attachment", Json::str(c.attachment)},
        {"resolutionProof", Json::str(c.resolutionProof)}, {"rating", Json::num(c.rating)},
        {"feedback", Json::str(c.feedback)}, {"createdAt", Json::str(c.createdAt)},
        {"updatedAt", Json::str(c.updatedAt)}
    });
}

std::string WebServer::userToJson(const UserRecord& u) {
    return Json::object({
        {"id", Json::num(u.id)}, {"name", Json::str(u.name)},
        {"email", Json::str(u.email)}, {"phone", Json::str(u.phone)},
        {"role", Json::str(u.role)}, {"department", Json::str(u.department)},
        {"createdAt", Json::str(u.createdAt)}
    });
}

std::string WebServer::notificationToJson(const NotificationRecord& n) {
    return Json::object({
        {"id", Json::num(n.id)}, {"complaintId", Json::num(n.complaintId)},
        {"message", Json::str(n.message)}, {"isRead", Json::boolean(n.isRead)},
        {"createdAt", Json::str(n.createdAt)}
    });
}

// ══════════════════════════════════════════
//  STATIC FILES
// ══════════════════════════════════════════

void WebServer::setupStaticFiles() {
    server.set_mount_point("/", "./public");
    server.set_mount_point("/uploads", "./uploads");
}

// ══════════════════════════════════════════
//  AUTH ROUTES
// ══════════════════════════════════════════

void WebServer::setupAuthRoutes() {
    // ── POST /api/auth/signup ──
    server.Post("/api/auth/signup", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto body = Json::parse(req.body);
        std::string name = Json::get(body, "name");
        std::string email = Validator::toLower(Validator::trim(Json::get(body, "email")));
        std::string phone = Validator::trim(Json::get(body, "phone"));
        std::string password = Json::get(body, "password");
        std::string role = Json::get(body, "role", "citizen");
        std::string department = Json::get(body, "department", "");

        // Validate
        auto v = Validator::validateName(name);
        if (!v.valid) { sendError(res, 400, v.error); return; }
        v = Validator::validateEmail(email);
        if (!v.valid) { sendError(res, 400, v.error); return; }
        v = Validator::validatePhone(phone);
        if (!v.valid) { sendError(res, 400, v.error); return; }
        v = Validator::validatePassword(password);
        if (!v.valid) { sendError(res, 400, v.error); return; }
        if (db.emailExists(email)) { sendError(res, 409, "Email already registered"); return; }

        std::string salt = auth.createSalt();
        std::string hash = auth.hashPassword(password, salt);
        int userId = db.createUser(Validator::trim(name), email, phone, hash, salt, role, department);
        if (userId < 0) { sendError(res, 500, "Failed to create user"); return; }

        std::string token = auth.generateToken(userId, role, Validator::trim(name));
        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)}, {"token", Json::str(token)},
            {"user", Json::object({
                {"id", Json::num(userId)}, {"name", Json::str(Validator::trim(name))},
                {"email", Json::str(email)}, {"role", Json::str(role)}
            })}
        }));
    });

    // ── POST /api/auth/login ──
    server.Post("/api/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto body = Json::parse(req.body);
        std::string email = Validator::toLower(Validator::trim(Json::get(body, "email")));
        std::string password = Json::get(body, "password");

        if (email.empty() || password.empty()) { sendError(res, 400, "Email and password required"); return; }

        UserRecord user = db.getUserByEmail(email);
        if (user.id == 0) { sendError(res, 401, "Invalid email or password"); return; }
        if (!auth.verifyPassword(password, user.passwordHash, user.salt)) {
            sendError(res, 401, "Invalid email or password"); return;
        }

        std::string token = auth.generateToken(user.id, user.role, user.name);
        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)}, {"token", Json::str(token)},
            {"user", userToJson(user)}
        }));
    });

    // ── GET /api/auth/me ──
    server.Get("/api/auth/me", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Invalid token"); return; }
        UserRecord user = db.getUserById(token.userId);
        if (user.id == 0) { sendError(res, 404, "User not found"); return; }
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"user", userToJson(user)}}));
    });
}

// ══════════════════════════════════════════
//  COMPLAINT ROUTES
// ══════════════════════════════════════════

void WebServer::setupComplaintRoutes() {
    // ── GET /api/complaints ──
    server.Get("/api/complaints", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }

        std::vector<ComplaintRecord> complaints;
        std::string status = req.get_param_value("status");
        std::string type = req.get_param_value("type");

        if (token.role == "citizen") {
            complaints = db.getComplaintsByUser(token.userId);
        } else if (token.role == "officer") {
            complaints = db.getComplaintsByOfficer(token.userId);
        } else {
            if (!status.empty()) complaints = db.getComplaintsByStatus(status);
            else if (!type.empty()) complaints = db.getComplaintsByType(type);
            else complaints = db.getAllComplaints();
        }

        std::vector<std::string> items;
        for (auto& c : complaints) items.push_back(complaintToJson(c));
        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)},
            {"complaints", Json::array(items)},
            {"total", Json::num((int)items.size())}
        }));
    });

    // ── GET /api/complaints/:id ──
    server.Get(R"(/api/complaints/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        int id = std::stoi(req.matches[1]);
        ComplaintRecord c = db.getComplaintById(id);
        if (c.id == 0) { sendError(res, 404, "Complaint not found"); return; }
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"complaint", complaintToJson(c)}}));
    });

    // ── POST /api/complaints ──
    server.Post("/api/complaints", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        if (token.role != "citizen") { sendError(res, 403, "Only citizens can file complaints"); return; }

        auto body = Json::parse(req.body);
        std::string type = Validator::trim(Json::get(body, "type"));
        std::string desc = Validator::trim(Json::get(body, "description"));
        std::string location = Validator::trim(Json::get(body, "location"));
        std::string severity = Validator::trim(Json::get(body, "severity"));

        auto v = Validator::validateComplaintType(type);
        if (!v.valid) { sendError(res, 400, v.error); return; }
        v = Validator::validateSeverity(severity);
        if (!v.valid) { sendError(res, 400, v.error); return; }
        v = Validator::validateDescription(desc);
        if (!v.valid) { sendError(res, 400, v.error); return; }

        UserRecord user = db.getUserById(token.userId);
        int cid = db.createComplaint(token.userId, type, desc, location, severity,
                                     user.name, user.email, user.phone);
        if (cid < 0) { sendError(res, 500, "Failed to create complaint"); return; }

        ComplaintRecord created = db.getComplaintById(cid);
        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)},
            {"message", Json::str("Complaint registered successfully")},
            {"complaint", complaintToJson(created)}
        }));
    });

    // ── PATCH /api/complaints/:id/status ──
    server.Patch(R"(/api/complaints/(\d+)/status)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        if (token.role != "officer" && token.role != "senior_officer" && token.role != "admin") { sendError(res, 403, "Officers only"); return; }

        int id = std::stoi(req.matches[1]);
        auto body = Json::parse(req.body);
        std::string status = Json::get(body, "status");

        if (status != "Pending" && status != "Assigned" && status != "In Progress" &&
            status != "Resolved" && status != "Escalated" && status != "Closed") {
            sendError(res, 400, "Invalid status"); return;
        }

        if (db.updateComplaintStatus(id, status)) {
            sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("Status updated")}}));
        } else { sendError(res, 500, "Update failed"); }
    });

    // ── PATCH /api/complaints/:id/resolve ──
    server.Patch(R"(/api/complaints/(\d+)/resolve)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid || (token.role != "officer" && token.role != "senior_officer" && token.role != "admin")) {
            sendError(res, 403, "Officers only"); return;
        }
        int id = std::stoi(req.matches[1]);
        auto body = Json::parse(req.body);
        std::string proof = Json::get(body, "proof");
        if (proof.empty()) { sendError(res, 400, "Resolution proof required"); return; }
        if (db.submitResolution(id, proof)) {
            sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("Resolution submitted")}}));
        } else { sendError(res, 500, "Failed"); }
    });

    // ── PATCH /api/complaints/:id/accept ──
    server.Patch(R"(/api/complaints/(\d+)/accept)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        int id = std::stoi(req.matches[1]);
        db.acceptResolution(id);
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("Resolution accepted")}}));
    });

    // ── PATCH /api/complaints/:id/reject ──
    server.Patch(R"(/api/complaints/(\d+)/reject)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        int id = std::stoi(req.matches[1]);
        db.rejectResolution(id);
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("Resolution rejected")}}));
    });

    // ── POST /api/complaints/:id/rate ──
    server.Post(R"(/api/complaints/(\d+)/rate)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        int id = std::stoi(req.matches[1]);
        auto body = Json::parse(req.body);
        int rating = Json::getInt(body, "rating");
        std::string feedback = Json::get(body, "feedback");
        if (rating < 1 || rating > 5) { sendError(res, 400, "Rating must be 1-5"); return; }
        db.rateComplaint(id, rating, feedback);

        ComplaintRecord c = db.getComplaintById(id);
        if (c.assignedOfficer > 0) {
            db.addNotification(c.assignedOfficer, id,
                "You received a " + std::to_string(rating) + "-star rating for complaint #" + std::to_string(id));
        }
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("Rating submitted")}}));
    });
}

// ══════════════════════════════════════════
//  DASHBOARD / ANALYTICS ROUTES
// ══════════════════════════════════════════

void WebServer::setupDashboardRoutes() {
    // ── GET /api/dashboard/stats ──
    server.Get("/api/dashboard/stats", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }

        // For citizens, return stats scoped to THEIR complaints only
        if (token.role == "citizen") {
            auto myComplaints = db.getComplaintsByUser(token.userId);
            int total = (int)myComplaints.size(), pending = 0, resolved = 0;
            int emergency = 0, escalated = 0, inProgress = 0;
            for (auto& c : myComplaints) {
                if (c.status == "Pending" || c.status == "Assigned") pending++;
                if (c.status == "Resolved" || c.status == "Closed") resolved++;
                if (c.status == "In Progress") inProgress++;
                if (c.status == "Escalated") escalated++;
                if (c.severity == "Emergency") emergency++;
            }
            sendSuccess(res, Json::object({
                {"success", Json::boolean(true)},
                {"stats", Json::object({
                    {"total", Json::num(total)},
                    {"pending", Json::num(pending)},
                    {"inProgress", Json::num(inProgress)},
                    {"resolved", Json::num(resolved)},
                    {"escalated", Json::num(escalated)},
                    {"emergency", Json::num(emergency)},
                    {"road", Json::num(0)}, {"water", Json::num(0)}, {"electricity", Json::num(0)}
                })}
            }));
            return;
        }

        // For officers/admins, return global stats
        auto stats = db.getStatsSummary();
        auto cats = db.getComplaintsByCategory();

        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)},
            {"stats", Json::object({
                {"total", Json::num(stats["total"])},
                {"pending", Json::num(stats["Pending"] + stats["Assigned"])},
                {"inProgress", Json::num(stats["In Progress"])},
                {"resolved", Json::num(stats["Resolved"] + stats["Closed"])},
                {"escalated", Json::num(stats["Escalated"])},
                {"emergency", Json::num(stats["sev_Emergency"])},
                {"road", Json::num(cats["Road"])},
                {"water", Json::num(cats["Water"])},
                {"electricity", Json::num(cats["Electricity"])}
            })}
        }));
    });

    // ── GET /api/dashboard/charts ──
    server.Get("/api/dashboard/charts", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }

        auto cats = db.getComplaintsByCategory();
        auto statDist = db.getStatusDistribution();
        auto timeline = db.getComplaintsOverTime(30);
        auto ratings = db.getOfficerRatings();

        // Category data
        std::vector<std::string> catLabels, catValues;
        for (auto& p : cats) { catLabels.push_back(Json::str(p.first)); catValues.push_back(Json::num(p.second)); }

        // Status data
        std::vector<std::string> statLabels, statValues;
        for (auto& p : statDist) { statLabels.push_back(Json::str(p.first)); statValues.push_back(Json::num(p.second)); }

        // Timeline data
        std::vector<std::string> timeLabels, timeValues;
        for (auto& p : timeline) { timeLabels.push_back(Json::str(p.first)); timeValues.push_back(Json::num(p.second)); }

        // Officer ratings
        std::vector<std::string> ratingItems;
        for (auto& p : ratings) {
            ratingItems.push_back(Json::object({{"name", Json::str(p.first)}, {"rating", Json::num(p.second)}}));
        }

        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)},
            {"categories", Json::object({{"labels", Json::array(catLabels)}, {"values", Json::array(catValues)}})},
            {"statusDist", Json::object({{"labels", Json::array(statLabels)}, {"values", Json::array(statValues)}})},
            {"timeline", Json::object({{"labels", Json::array(timeLabels)}, {"values", Json::array(timeValues)}})},
            {"officerRatings", Json::array(ratingItems)}
        }));
    });

    // ── GET /api/export/csv ──
    server.Get("/api/export/csv", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid || token.role != "admin") { sendError(res, 403, "Admin only"); return; }

        auto complaints = db.getAllComplaints();
        std::string csv = "ID,Tracking ID,Citizen,Email,Type,Description,Severity,Status,Officer,Rating,Created\n";
        for (auto& c : complaints) {
            csv += std::to_string(c.id) + "," + c.trackingId + "," + c.citizenName + ","
                + c.citizenEmail + "," + c.type + ",\"" + c.description + "\"," + c.severity + ","
                + c.status + "," + c.officerName + "," + std::to_string(c.rating) + "," + c.createdAt + "\n";
        }
        res.set_header("Content-Disposition", "attachment; filename=spcms_complaints.csv");
        res.set_content(csv, "text/csv");
    });
}

// ══════════════════════════════════════════
//  NOTIFICATION ROUTES
// ══════════════════════════════════════════

void WebServer::setupNotificationRoutes() {
    server.Get("/api/notifications", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        auto notes = db.getNotifications(token.userId);
        std::vector<std::string> items;
        for (auto& n : notes) items.push_back(notificationToJson(n));
        int unread = db.getUnreadCount(token.userId);
        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)}, {"notifications", Json::array(items)},
            {"unreadCount", Json::num(unread)}
        }));
    });

    server.Post("/api/notifications/read-all", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        db.markAllRead(token.userId);
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}}));
    });
}

// ══════════════════════════════════════════
//  USER MANAGEMENT ROUTES (Admin)
// ══════════════════════════════════════════

void WebServer::setupUserRoutes() {
    server.Get("/api/users", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid || token.role != "admin") { sendError(res, 403, "Admin only"); return; }
        auto users = db.getAllUsers();
        std::vector<std::string> items;
        for (auto& u : users) items.push_back(userToJson(u));
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"users", Json::array(items)}}));
    });

    server.Delete(R"(/api/users/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid || token.role != "admin") { sendError(res, 403, "Admin only"); return; }
        int id = std::stoi(req.matches[1]);
        if (id == token.userId) { sendError(res, 400, "Cannot delete yourself"); return; }
        db.deleteUser(id);
        sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("User deleted")}}));
    });
}

// ══════════════════════════════════════════
//  FILE UPLOAD ROUTES
// ══════════════════════════════════════════

void WebServer::setupUploadRoutes() {
    server.Post("/api/upload", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }

        if (!req.has_file("file")) { sendError(res, 400, "No file uploaded"); return; }
        auto file = req.get_file_value("file");

        // Check size (5MB max)
        if (file.content.size() > 5 * 1024 * 1024) { sendError(res, 400, "File too large (max 5MB)"); return; }

        // Generate unique filename
        std::string ext = "";
        size_t dotPos = file.filename.rfind('.');
        if (dotPos != std::string::npos) ext = file.filename.substr(dotPos);
        std::string filename = "upload_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(rand() % 10000) + ext;

        std::ofstream ofs("uploads/" + filename, std::ios::binary);
        ofs.write(file.content.c_str(), file.content.size());
        ofs.close();

        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)},
            {"filename", Json::str(filename)},
            {"path", Json::str("/uploads/" + filename)}
        }));
    });
}

// ══════════════════════════════════════════
//  SEED DEMO DATA
// ══════════════════════════════════════════

void WebServer::seedIfEmpty() {
    if (db.emailExists("admin@spcms.gov.in")) return;
    std::cout << "  Seeding demo data..." << std::endl;

    // Create admin
    std::string s1 = auth.createSalt(); db.createUser("System Administrator", "admin@spcms.gov.in", "0000000000", auth.hashPassword("Admin@123", s1), s1, "admin");

    // Create officers
    std::string s2 = auth.createSalt(); db.createUser("R.K. Sharma", "sharma@pwd.gov.in", "9111111111", auth.hashPassword("Officer@123", s2), s2, "officer", "PWD");
    std::string s3 = auth.createSalt(); db.createUser("A.K. Patel", "patel@water.gov.in", "9222222222", auth.hashPassword("Officer@123", s3), s3, "officer", "Water Supply");
    std::string s4 = auth.createSalt(); db.createUser("V.K. Gupta", "gupta@electricity.gov.in", "9333333333", auth.hashPassword("Officer@123", s4), s4, "officer", "Electricity Board");

    // Create senior officer
    std::string s8 = auth.createSalt(); db.createUser("Sr. Officer Verma", "senior@spcms.gov.in", "9444444444", auth.hashPassword("Senior@123", s8), s8, "senior_officer", "PWD");

    // Create citizens
    std::string s5 = auth.createSalt(); db.createUser("Rajesh Kumar", "rajesh@gmail.com", "9876543210", auth.hashPassword("Citizen@123", s5), s5, "citizen");
    std::string s6 = auth.createSalt(); db.createUser("Priya Verma", "priya@gmail.com", "9988776655", auth.hashPassword("Citizen@123", s6), s6, "citizen");
    std::string s7 = auth.createSalt(); db.createUser("Mohan Singh", "mohan@gmail.com", "9012345678", auth.hashPassword("Citizen@123", s7), s7, "citizen");

    // Create sample complaints (user IDs: admin=1, sharma=2, patel=3, gupta=4, senior=5, rajesh=6, priya=7, mohan=8)
    db.createComplaint(6, "Road", "Large pothole on MG Road near Junction 4", "MG Road, Junction 4", "Emergency", "Rajesh Kumar", "rajesh@gmail.com", "9876543210");
    db.createComplaint(7, "Water", "Water pipeline leak flooding Sector 21", "Sector 21", "Normal", "Priya Verma", "priya@gmail.com", "9988776655");
    db.createComplaint(8, "Electricity", "Power outage in Ward 7, transformer sparking", "Ward 7", "Emergency", "Mohan Singh", "mohan@gmail.com", "9012345678");
    db.createComplaint(6, "Road", "Broken streetlight on NH-48 service road", "NH-48 Service Road", "Low", "Rajesh Kumar", "rajesh@gmail.com", "9876543210");
    db.createComplaint(7, "Water", "No water supply for 3 days in Greenwood Colony", "Greenwood Colony", "Emergency", "Priya Verma", "priya@gmail.com", "9988776655");
    db.createComplaint(8, "Electricity", "Frequent voltage fluctuations in Block C", "Block C", "Normal", "Mohan Singh", "mohan@gmail.com", "9012345678");

    // Update some statuses for variety
    db.updateComplaintStatus(2, "In Progress");
    db.updateComplaintStatus(3, "Escalated");
    db.submitResolution(4, "Streetlight repaired and tested. Photo evidence attached.");

    std::cout << "  Demo data seeded successfully!" << std::endl;
}

// ══════════════════════════════════════════
//  OPTIONS handler for CORS preflight
// ══════════════════════════════════════════

// ══════════════════════════════════════════
//  START / STOP
// ══════════════════════════════════════════

// ══════════════════════════════════════════
//  ESCALATION ROUTES (Senior Officer / Admin)
// ══════════════════════════════════════════

void WebServer::setupEscalationRoutes() {
    // ── GET /api/complaints/escalated ──
    server.Get("/api/complaints/escalated", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        if (token.role != "senior_officer" && token.role != "admin") {
            sendError(res, 403, "Senior Officers and Admins only"); return;
        }
        auto complaints = db.getComplaintsByStatus("Escalated");
        std::vector<std::string> items;
        for (auto& c : complaints) items.push_back(complaintToJson(c));
        sendSuccess(res, Json::object({
            {"success", Json::boolean(true)},
            {"complaints", Json::array(items)},
            {"total", Json::num((int)items.size())}
        }));
    });

    // ── PATCH /api/complaints/:id/escalate ──
    server.Patch(R"(/api/complaints/(\d+)/escalate)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        if (token.role != "officer" && token.role != "senior_officer" && token.role != "admin") {
            sendError(res, 403, "Officers only"); return;
        }
        int id = std::stoi(req.matches[1]);
        if (db.updateComplaintStatus(id, "Escalated")) {
            sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("Complaint escalated to senior officer")}}));
        } else { sendError(res, 500, "Escalation failed"); }
    });

    // ── PATCH /api/complaints/:id/reassign ──
    server.Patch(R"(/api/complaints/(\d+)/reassign)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        if (token.role != "senior_officer" && token.role != "admin") {
            sendError(res, 403, "Senior Officers only"); return;
        }
        int id = std::stoi(req.matches[1]);
        auto body = Json::parse(req.body);
        int officerId = Json::getInt(body, "officerId");
        if (officerId <= 0) {
            // Auto-pick an officer from the complaint's department
            ComplaintRecord c = db.getComplaintById(id);
            std::string dept = "";
            if (c.type == "Road") dept = "PWD";
            else if (c.type == "Water") dept = "Water Supply";
            else if (c.type == "Electricity") dept = "Electricity Board";
            auto officers = db.getUsersByRole("officer");
            for (auto& o : officers) {
                if (o.department == dept && o.id != c.assignedOfficer) {
                    officerId = o.id; break;
                }
            }
            if (officerId <= 0 && !officers.empty()) officerId = officers[0].id;
        }
        if (officerId <= 0) { sendError(res, 400, "No officer available for reassignment"); return; }
        if (db.assignOfficer(id, officerId)) {
            UserRecord officer = db.getUserById(officerId);
            sendSuccess(res, Json::object({
                {"success", Json::boolean(true)},
                {"message", Json::str("Complaint reassigned to " + officer.name)}
            }));
        } else { sendError(res, 500, "Reassignment failed"); }
    });

    // ── POST /api/complaints/:id/assign ──  (Officer self-assign)
    server.Post(R"(/api/complaints/(\d+)/assign)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        auto token = authenticate(req);
        if (!token.valid) { sendError(res, 401, "Auth required"); return; }
        if (token.role != "officer" && token.role != "senior_officer" && token.role != "admin") {
            sendError(res, 403, "Officers only"); return;
        }
        int id = std::stoi(req.matches[1]);
        if (db.assignOfficer(id, token.userId)) {
            sendSuccess(res, Json::object({{"success", Json::boolean(true)}, {"message", Json::str("Complaint assigned to you")}}));
        } else { sendError(res, 500, "Assignment failed"); }
    });
}

void WebServer::start() {
    setupStaticFiles();
    setupAuthRoutes();
    setupComplaintRoutes();
    setupDashboardRoutes();
    setupNotificationRoutes();
    setupUserRoutes();
    setupUploadRoutes();
    setupEscalationRoutes();

    // CORS preflight
    server.Options(R"(.*)", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        res.status = 204;
    });

    seedIfEmpty();

    std::cout << "\n";
    std::cout << "  ===============================================" << std::endl;
    std::cout << "   SPCMS Web Server Running!" << std::endl;
    std::cout << "   Open: http://localhost:" << port << std::endl;
    std::cout << "  ===============================================" << std::endl;
    std::cout << "\n  Demo Accounts:" << std::endl;
    std::cout << "    Admin:     admin@spcms.gov.in / Admin@123" << std::endl;
    std::cout << "    Sr.Officer: senior@spcms.gov.in / Senior@123" << std::endl;
    std::cout << "    Officer:   sharma@pwd.gov.in / Officer@123" << std::endl;
    std::cout << "    Citizen:   rajesh@gmail.com / Citizen@123" << std::endl;
    // Start auto-escalation background thread (checks every 5 minutes)
    std::thread escalationThread([this]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::minutes(5));
            runAutoEscalation();
        }
    });
    escalationThread.detach();

    std::cout << "  Auto-escalation: Enabled (24-hour deadline)" << std::endl;
    std::cout << "\n  Press Ctrl+C to stop.\n" << std::endl;

    server.listen("0.0.0.0", port);
}

void WebServer::runAutoEscalation() {
    // Get all complaints that are Assigned or In Progress
    auto assigned = db.getComplaintsByStatus("Assigned");
    auto inProgress = db.getComplaintsByStatus("In Progress");
    assigned.insert(assigned.end(), inProgress.begin(), inProgress.end());

    std::time_t now = std::time(nullptr);
    int escalated = 0;

    for (auto& c : assigned) {
        // Parse the updated_at or created_at timestamp
        std::string timeStr = c.updatedAt.empty() ? c.createdAt : c.updatedAt;
        if (timeStr.empty()) continue;

        // Parse "YYYY-MM-DD HH:MM:SS"
        std::tm tm = {};
        std::istringstream ss(timeStr);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (ss.fail()) continue;

        std::time_t complaintTime = std::mktime(&tm);
        double hoursElapsed = std::difftime(now, complaintTime) / 3600.0;

        // If more than 24 hours have passed, auto-escalate
        if (hoursElapsed >= 24.0) {
            db.updateComplaintStatus(c.id, "Escalated");
            // Notify all senior officers
            auto seniors = db.getUsersByRole("senior_officer");
            for (auto& sr : seniors) {
                db.addNotification(sr.id, c.id,
                    "Auto-escalated: Complaint #" + std::to_string(c.id) +
                    " (" + c.type + ") exceeded 24-hour deadline");
            }
            // Notify the assigned officer
            if (c.assignedOfficer > 0) {
                db.addNotification(c.assignedOfficer, c.id,
                    "Complaint #" + std::to_string(c.id) +
                    " has been auto-escalated due to exceeding 24-hour deadline");
            }
            escalated++;
        }
    }

    if (escalated > 0) {
        std::cout << "  [Auto-Escalation] " << escalated
                  << " complaint(s) escalated (exceeded 24-hour deadline)" << std::endl;
    }
}

void WebServer::stop() {
    server.stop();
}
