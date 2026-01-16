# Hybrid Fraud Detection Engine

A production-inspired fraud detection system that combines rule-based risk
scoring with ML-ready feature generation. Designed to simulate real-world
banking fraud pipelines.

---

## Fraud Rules

**Rule 1: High Amount Transaction**

- Transaction amount > user’s average × 3
- OR amount > absolute threshold (₹80,000)

**Rule 2: High Frequency Transactions (Velocity)**

- More than N transactions within a short time window

**Rule 3: Sudden Device or IP Change**

- New device OR new IP address

**Rule 4: Spending Spike**

- Current transaction significantly higher than historical average

**Rule 5: Shared Device Usage**

- Same device used by multiple user IDs

### Risk Decisions

- **ALLOW**: Risk < 50
- **CHALLENGE**: Risk 50–79
- **BLOCK**: Risk ≥ 80

---

## Key Features

- Rule-based fraud detection using velocity, device and IP anomalies
- Stateful user risk profiling with decay and cooldown logic
- Shadow ML scoring pipeline for offline model comparison
- Automatic ML training data generation in CSV format
- Scalable O(1) transaction processing

---

## System Architecture

CSV Input → Feature Engineering → Rule Engine →  
User Risk State → Decision Engine → ML Training Data (CSV)

---

## Project Structure

fraud-detection-engine/
│
├── data/
│ └── sample_transactions.csv
│
├── main.cpp
├── fraud_engine.h / fraud_engine.cpp
├── csv_reader.h / csv_reader.cpp
├── transaction.h
└── README.md

---

## How to run

g++ -std=c++17 main.cpp fraud_engine.cpp csv_reader.cpp -o fraud
./fraud
