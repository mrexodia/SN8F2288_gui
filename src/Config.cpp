#include "Config.h"

ConfigStore & ConfigStore::instance()
{
    static ConfigStore i;
    return i;
}
