#pragma once
#include "Complaint.h"
#include "Trackable.h"
using namespace std;

// ══════════════════════════════════════════════════════════
//  WaterComplaint — Derived from Complaint + Trackable
//  Demonstrates: Hierarchical + Multiple Inheritance,
//                Function Overriding, Runtime Polymorphism
// ══════════════════════════════════════════════════════════

class WaterComplaint : public Complaint, public Trackable {
private:
    string pipelineId;      // identifier for affected pipeline
    string area;            // area/sector affected

public:
    // ── Constructors ──
    WaterComplaint();
    WaterComplaint(const string& desc, const string& photo, const string& sev,
                   const string& citName, const string& citContact, const string& citEmail,
                   const string& pipelineId, const string& area);
    WaterComplaint(const WaterComplaint& other);

    // ── Destructor ──
    ~WaterComplaint();

    // ── Override Pure Virtual Functions ──
    string getDepartment() const override;
    void displayTypeSpecificInfo() const override;

    // ── Override Virtual Function ──
    void forwardToOfficer() override;

    // ── Getters (Inline) ──
    inline string getPipelineId() const { return pipelineId; }
    inline string getArea() const { return area; }

    // ── Setters (this pointer) ──
    void setPipelineId(const string& p) { this->pipelineId = p; }
    void setArea(const string& a) { this->area = a; }
};
