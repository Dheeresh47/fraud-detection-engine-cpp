#include "csv_reader.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

vector<Transaction> CSVReader::read(const string &filename)
{
    vector<Transaction> transactions;
    ifstream file(filename);

    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << filename << "\n";
        return transactions;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line))
    {
        stringstream ss(line);
        string token;
        Transaction t;

        getline(ss, token, ',');
        t.user_id = stoi(token);

        getline(ss, t.device_id, ',');
        getline(ss, t.ip, ',');

        getline(ss, token, ',');
        t.amount = stoi(token);

        getline(ss, token, ',');
        t.timestamp = stol(token);

        transactions.push_back(t);
    }

    return transactions;
}
