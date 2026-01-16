#include "fraud_engine.h"
#include "csv_reader.h"
#include <iostream>
#include <vector>
using namespace std;

int main()
{
    FraudEngine engine;

    auto transactions =
        CSVReader::read("data/sample_transactions.csv");

    if (transactions.empty())
    {
        cerr << "No transactions loaded.\n";
        return 1;
    }

    for (auto &t : transactions)
    {
        auto res = engine.processTransaction(t);
        cout << "User " << t.user_id
             << " | Final Risk: " << res.finalRisk
             << " | Decision: " << res.decision << "\n";
    }

    engine.printSummary();
    return 0;
}
