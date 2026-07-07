#pragma once
#include "httplib.h"
#include "Database.h"
#include "AuthManager.h"
#include "Validator.h"
#include "JsonUtil.h"
#include <string>
#include <functional>

// ══════════════════════════════════════════════════════════
//  WebServer — REST API Server (Composition Pattern)
//  Demonstrates: Composition, Encapsulation, Delegation
//  Uses existing OOP class hierarchy through Database layer
// ══════════════════════════════════════════════════════════

class WebServer {
private:
    httplib::Server server;
    Database& db;
    AuthManager auth;
    int port;

    // ── Route Setup ──
    void setupStaticFiles();
    void setupAuthRoutes();
    void setupComplaintRoutes();
    void setupDashboardRoutes();
    void setupNotificationRoutes();
    void setupUserRoutes();
    void setupUploadRoutes();
    void setupEscalationRoutes();
    void runAutoEscalation();

    // ── Middleware Helpers ──
    AuthManager::TokenPayload authenticate(const httplib::Request& req);
    bool requireAuth(const httplib::Request& req, httplib::Response& res);
    bool requireRole(const httplib::Request& req, httplib::Response& res, const std::string& role);

    // ── JSON Response Helpers ──
    std::string complaintToJson(const ComplaintRecord& c);
    std::string userToJson(const UserRecord& u);
    std::string notificationToJson(const NotificationRecord& n);
    void sendError(httplib::Response& res, int code, const std::string& msg);
    void sendSuccess(httplib::Response& res, const std::string& data);

    // ── CORS ──
    void setCorsHeaders(httplib::Response& res);

    // ── Seed demo data ──
    void seedIfEmpty();

public:
    WebServer(int port = 8080);
    void start();
    void stop();
};
