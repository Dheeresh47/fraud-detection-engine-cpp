#include "fraud_engine.h"
#include <bits/stdc++.h>
#include <iostream>
#include <cassert>
using namespace std;

// ---------------- CONFIG ----------------
const int HIGH_AMOUNT_LIMIT = 80000;
const int VELOCITY_LIMIT = 3;
const int TIME_WINDOW = 120;

const int SCORE_HIGH_AMOUNT = 40;
const int SCORE_ABSOLUTE_LIMIT = 50;
const int SCORE_VELOCITY = 30;
const int SCORE_NEW_DEVICE = 20;
const int SCORE_NEW_IP = 15;
const int SCORE_SPENDING_SPIKE = 25;
const int SCORE_SHARED_DEVICE = 35;

const double DECAY_FACTOR = 0.9;
const int COOLDOWN_TIME = 600;

// ML shadow config
struct MLConfig
{
    static constexpr double WEIGHT_HIGH_AMOUNT = 0.8;
    static constexpr double WEIGHT_VELOCITY = 0.7;
    static constexpr double WEIGHT_NEW_DEVICE = 0.4;
    static constexpr double WEIGHT_NEW_IP = 0.3;
    static constexpr double WEIGHT_SPENDING_SPIKE = 0.5;
    static constexpr double WEIGHT_SHARED_DEVICE = 0.6;
    static constexpr double WEIGHT_USER_RISK = 0.02;
    static constexpr double SCORE_MULTIPLIER = 25.0;
};

// ---------------- ENGINE ----------------
FraudEngine::FraudEngine()
{
    csv.open("training_data.csv");
    csv << "high_amount,velocity,new_device,new_ip,spending_spike,shared_device,"
           "txn_amount,txn_count,user_risk,rule_risk,shadow_ml_risk,decision\n";
}

void FraudEngine::validateTransaction(const Transaction &t)
{
    assert(t.amount > 0);
    assert(t.timestamp > 0);
    assert(!t.device_id.empty());
    assert(!t.ip.empty());
}

int FraudEngine::clampRisk(double r)
{
    return max(0, min(100, (int)r));
}

string FraudEngine::getDecision(int risk)
{
    if (risk >= 80)
        return "BLOCK";
    if (risk >= 50)
        return "CHALLENGE";
    return "ALLOW";
}

DecisionResult FraudEngine::processTransaction(const Transaction &t)
{
    validateTransaction(t);
    total_tx++;

    DecisionResult res;
    vector<string> &reasons = res.reasons;

    long long prev_total = user_total_amount[t.user_id];
    int prev_count = user_txn_count[t.user_id];

    // Cooldown
    if (user_last_seen.count(t.user_id))
    {
        if (t.timestamp - user_last_seen[t.user_id] > COOLDOWN_TIME)
            user_risk_score[t.user_id] *= 0.7;
    }

    int ruleRisk = 0;
    int high_amount = 0, velocity = 0, new_device = 0,
        new_ip = 0, spending_spike = 0, shared_device = 0;

    // Rule 1 & 2
    if (prev_count > 0 && t.amount > (prev_total / prev_count) * 3)
    {
        ruleRisk += SCORE_HIGH_AMOUNT;
        high_amount = 1;
        reasons.push_back("Amount > 3x user average");
    }
    if (t.amount > HIGH_AMOUNT_LIMIT)
    {
        ruleRisk += SCORE_ABSOLUTE_LIMIT;
        high_amount = 1;
        reasons.push_back("Amount exceeds absolute threshold");
    }

    // Velocity
    auto &dq = user_timestamps[t.user_id];
    while (!dq.empty() && t.timestamp - dq.front() > TIME_WINDOW)
        dq.pop_front();
    dq.push_back(t.timestamp);

    if (dq.size() > VELOCITY_LIMIT)
    {
        ruleRisk += SCORE_VELOCITY;
        velocity = 1;
        reasons.push_back("High transaction velocity");
    }

    // New device/IP
    if (!user_devices[t.user_id].empty() &&
        !user_devices[t.user_id].count(t.device_id))
    {
        ruleRisk += SCORE_NEW_DEVICE;
        new_device = 1;
        reasons.push_back("New device detected");
    }

    if (!user_ips[t.user_id].empty() &&
        !user_ips[t.user_id].count(t.ip))
    {
        ruleRisk += SCORE_NEW_IP;
        new_ip = 1;
        reasons.push_back("New IP detected");
    }

    // Spending spike
    if (prev_count >= 3 && t.amount > (prev_total / prev_count) * 2)
    {
        ruleRisk += SCORE_SPENDING_SPIKE;
        spending_spike = 1;
        reasons.push_back("Spending spike detected");
    }

    // Shared device
    if (!devices_users[t.device_id].empty() &&
        devices_users[t.device_id].size() >= 2 &&
        !devices_users[t.device_id].count(t.user_id))
    {
        ruleRisk += SCORE_SHARED_DEVICE;
        shared_device = 1;
        reasons.push_back("Shared device across users");
    }

    // Shadow ML
    double ml_score =
        high_amount * MLConfig::WEIGHT_HIGH_AMOUNT +
        velocity * MLConfig::WEIGHT_VELOCITY +
        new_device * MLConfig::WEIGHT_NEW_DEVICE +
        new_ip * MLConfig::WEIGHT_NEW_IP +
        spending_spike * MLConfig::WEIGHT_SPENDING_SPIKE +
        shared_device * MLConfig::WEIGHT_SHARED_DEVICE +
        user_risk_score[t.user_id] * MLConfig::WEIGHT_USER_RISK;

    int shadowMLRisk = clampRisk(ml_score * MLConfig::SCORE_MULTIPLIER);

    user_risk_score[t.user_id] =
        clampRisk(user_risk_score[t.user_id] * DECAY_FACTOR + ruleRisk);

    int finalRisk = user_risk_score[t.user_id];
    string decision = getDecision(finalRisk);

    if (decision != "ALLOW")
        flagged++;
    if (finalRisk >= 70)
        high_risk++;

    csv << high_amount << "," << velocity << "," << new_device << ","
        << new_ip << "," << spending_spike << "," << shared_device << ","
        << t.amount << "," << prev_count << "," << user_risk_score[t.user_id]
        << "," << ruleRisk << "," << shadowMLRisk << "," << decision << "\n";

    // Update state
    user_total_amount[t.user_id] += t.amount;
    user_txn_count[t.user_id]++;
    user_devices[t.user_id].insert(t.device_id);
    user_ips[t.user_id].insert(t.ip);
    devices_users[t.device_id].insert(t.user_id);
    user_last_seen[t.user_id] = t.timestamp;

    res.ruleRisk = ruleRisk;
    res.shadowMLRisk = shadowMLRisk;
    res.finalRisk = finalRisk;
    res.decision = decision;

    return res;
}

void FraudEngine::printSummary() const
{
    cout << "\nSYSTEM SUMMARY\n";
    cout << "Total Transactions: " << total_tx << "\n";
    cout << "Flagged: " << flagged << "\n";
    cout << "High Risk: " << high_risk << "\n";
}