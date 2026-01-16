#ifndef FRAUD_ENGINE_H
#define FRAUD_ENGINE_H

#include "transaction.h"
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <string>
#include <vector>
#include <fstream>

struct DecisionResult
{
    int ruleRisk;
    int shadowMLRisk;
    int finalRisk;
    std::string decision;
    std::vector<std::string> reasons;
    std::vector<std::string> alerts;
};

class FraudEngine
{
public:
    FraudEngine();
    DecisionResult processTransaction(const Transaction &t);
    void printSummary() const;

private:
    // ---------- Internal helpers ----------
    void validateTransaction(const Transaction &t);
    int clampRisk(double r);
    std::string getDecision(int risk);

    // ---------- User state ----------
    std::unordered_map<int, long long> user_total_amount;
    std::unordered_map<int, int> user_txn_count;
    std::unordered_map<int, std::unordered_set<std::string>> user_devices;
    std::unordered_map<int, std::unordered_set<std::string>> user_ips;
    std::unordered_map<int, std::deque<long>> user_timestamps;
    std::unordered_map<std::string, std::unordered_set<int>> devices_users;
    std::unordered_map<int, double> user_risk_score;
    std::unordered_map<int, long> user_last_seen;

    // ---------- Metrics ----------
    int total_tx = 0;
    int flagged = 0;
    int high_risk = 0;

    // ---------- CSV ----------
    std::ofstream csv;
};

#endif
