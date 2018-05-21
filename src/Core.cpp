#include "Core.h"

Core &Core::instance()
{
    static Core i;
    return i;
}

SN8F2288 &Core::chip()
{
    return instance()._chip;
}

ChipCpu *Core::cpu()
{
    return instance()._cpu;
}

Database &Core::db()
{
    return instance()._db;
}
