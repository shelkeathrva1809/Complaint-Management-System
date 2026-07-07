#include "../include/Trackable.h"

// ── Constructor ──
Trackable::Trackable() {}

// ── Destructor ──
Trackable::~Trackable() {}

// ── Add Event with Timestamp ──
void Trackable::addEvent(const string& event) {
    time_t now = time(0);
    struct tm* timeInfo = localtime(&now);
    char buffer[80];
    strftime(buffer, 80, "[%Y-%m-%d %H:%M:%S]", timeInfo);

    string entry = string(buffer) + " " + event;
    history.push_back(entry);
}

// ── Display History (uses STL iterator) ──
void Trackable::showHistory() const {
    if (history.empty()) {
        cout << "  No tracking history available." << endl;
        return;
    }
    cout << "  --- Tracking History ---" << endl;
    // Demonstrating STL const_iterator usage
    for (vector<string>::const_iterator it = history.begin(); it != history.end(); ++it) {
        cout << "    " << *it << endl;
    }
}

// ── Clear History ──
void Trackable::clearHistory() {
    history.clear();
}
