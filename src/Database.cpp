#include "Database.h"
#include <QDebug>

bool Database::save(const QString & file) const
{
    Q_UNUSED(file);
    qDebug() << "Database::save not implemented!";
    return false;
}

bool Database::load(const QString & file)
{
    Q_UNUSED(file);
    qDebug() << "Database::load not implemented!";
    return false;
}

void Database::clear()
{
    romLabels.clear();
    romComments.clear();
}

QString Database::findRomLabelByAddr(uint16_t addr) const
{
    auto found = romLabels.find(addr);
    QString label;
    if(found != romLabels.end())
        label = found.value();
    return label;
}

bool Database::findRomLabelByName(const QString & label, uint16_t & addr) const
{
    for(auto labelAddr : romLabels.keys())
    {
        if(romLabels[labelAddr] == label)
        {
            addr = labelAddr;
            return true;
        }
    }
    return false;
}

void Database::setRomLabel(uint16_t addr, const QString & label)
{
    romLabels[addr] = label;
}

QString Database::findRomCommentByAddr(uint16_t addr) const
{
    auto found = romComments.find(addr);
    QString comment;
    if(found != romComments.end())
        comment = found.value();
    return comment;
}

void Database::setRomComment(uint16_t addr, const QString & comment)
{
    romComments[addr] = comment;
}

QString Database::findGlobalRamLabelByAddr(uint16_t addr) const
{
    auto found = globalRamLabels.find(addr);
    QString label;
    if(found != globalRamLabels.end())
        label = found.value();
    return label;
}

void Database::setGlobalRamLabel(uint16_t addr, const QString & label)
{
    globalRamLabels[addr] = label;
}

QString Database::findGlobalRamBitLabelByAddr(uint16_t addr, uint8_t bit) const
{
    auto found = globalRamBitLabels.find({addr, bit});
    QString label;
    if(found != globalRamBitLabels.end())
        label = found.value();
    return label;
}

void Database::setGlobalRamBitLabel(uint16_t addr, uint8_t bit, const QString & label)
{
    globalRamBitLabels[{addr, bit}] = label;
}

Database::RomRange const* Database::findRomRange(uint16_t romAddr) const
{
    for(const RomRange & range : romRanges)
        if(romAddr >= range.romStart && romAddr <= range.romEnd)
            return &range;
    return nullptr;
}

Database::RomRange* Database::findRomRange(uint16_t romAddr)
{
    for(RomRange & range : romRanges)
        if(romAddr >= range.romStart && romAddr <= range.romEnd)
            return &range;
    return nullptr;
}

Database::RomRange* Database::addRomRange(uint16_t romStart, uint16_t romEnd)
{
    auto found1 = findRomRange(romStart);
    if(found1)
    {
        qDebug() << "range already exists!";
        return found1;
    }
    auto found2 = findRomRange(romEnd);
    if(found2)
    {
        qDebug() << "range already exists!";
        return found2;
    }
    RomRange newRange;
    newRange.romStart = romStart;
    newRange.romEnd = romEnd;
    romRanges.push_back(newRange);
    return &romRanges.back();
}

QString Database::findLocalRamLabelByAddr(uint16_t romAddr, uint16_t ramAddr) const
{
    auto range = findRomRange(romAddr);
    QString label;
    if(range)
    {
        auto found = range->ramLabels.find(ramAddr);
        if(found != range->ramLabels.end())
            label = found.value();
    }
    return label;
}

void Database::setLocalRamLabel(uint16_t romAddr, uint16_t ramAddr, const QString & label)
{
    //TODO: error handling
    auto range = findRomRange(romAddr);
    if(range)
        range->ramLabels[ramAddr] = label;
}

QString Database::findLocalRamBitLabelByAddr(uint16_t romAddr, uint16_t ramAddr, uint8_t bit) const
{
    auto range = findRomRange(romAddr);
    QString label;
    if(range)
    {
        auto found = range->ramBitLabels.find({ramAddr, bit});
        if(found != range->ramBitLabels.end())
            label = found.value();
    }
    return label;
}

void Database::setLocalRamBitLabel(uint16_t romAddr, uint16_t ramAddr, uint8_t bit, const QString & label)
{
    //TODO: error handling
    auto range = findRomRange(romAddr);
    if(range)
        range->ramBitLabels[{ramAddr, bit}] = label;
}
