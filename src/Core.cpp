#include "Core.h"

Core::Core()
{
    setvbuf(stdout, nullptr, _IONBF, 0);
    _cpu = new ChipCpu(&_chip);
    _cpu->start();
}

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
