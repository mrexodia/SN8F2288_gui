#ifndef CORE_H
#define CORE_H

#include "ChipCpu.h"
#include "Database.h"

class Core
{
public:
    static Core & instance();
    static SN8F2288 & chip();
    static ChipCpu* cpu();
    static Database & db();

private:
    SN8F2288 _chip;
    ChipCpu* _cpu;
    Database _db;
};

#endif // CORE_H
