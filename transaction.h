#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>

struct Transaction
{
    int user_id;
    std::string device_id;
    std::string ip;
    int amount;
    long timestamp;
};

#endif