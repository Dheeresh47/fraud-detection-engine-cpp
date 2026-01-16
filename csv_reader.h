#ifndef CSV_READER_H
#define CSV_READER_H

#include "transaction.h"
#include <vector>
#include <string>

class CSVReader
{
public:
    static std::vector<Transaction> read(const std::string &filename);
};

#endif
