#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <ctime>
using namespace std;

// ══════════════════════════════════════════════════════════
//  Trackable Mixin Class
//  Demonstrates: Multiple Inheritance (used as mixin),
//                STL Vectors, Iterators, Virtual Destructor
//  Purpose: Any class that inherits Trackable gets
//           a timestamped event history
// ══════════════════════════════════════════════════════════

class Trackable {
private:
    vector<string> history;

public:
    // Constructor
    Trackable();

    // Virtual destructor (required for proper cleanup in multiple inheritance)
    virtual ~Trackable();

    // Add a timestamped event to tracking history
    void addEvent(const string& event);

    // Display full tracking history (uses STL iterators)
    void showHistory() const;

    // Get number of tracked events
    inline int getHistorySize() const { return static_cast<int>(history.size()); }

    // Get read-only access to history vector
    const vector<string>& getHistory() const { return history; }

    // Clear all history
    void clearHistory();
};
