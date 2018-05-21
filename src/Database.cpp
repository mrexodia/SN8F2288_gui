#include "Database.h"
#include <QDebug>
#include <QFile>

#include "Utf8Ini/Utf8Ini.h"

bool Database::save(const QString & file)
{
    QFile f(file);
    if(!f.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "Failed to open file...";
        return false;
    }
    Utf8Ini ini;
    auto saveRamLabels = [&ini](const QString & section, const QMap<uint16_t, QString> & ram)
    {
        for(auto key : ram.keys())
            ini.SetValue(section.toStdString(), QString().sprintf("0x%04x", key).toStdString(), ram[key].toStdString());
    };
    auto saveRamBitLabels = [&ini](const QString & section, const QMap<QPair<uint16_t, uint8_t>, QString> & ram)
    {
        for(auto key : ram.keys())
            ini.SetValue(section.toStdString(), QString().sprintf("0x%04x.%d", key.first, key.second).toStdString(), ram[key].toStdString());
    };
    saveRamLabels("ram", globalRamLabels);
    saveRamBitLabels("ram", globalRamBitLabels);
    for(auto & romRange : romRanges)
    {
        auto section = QString().sprintf("ram@0x%04x-0x%04x", romRange.romStart, romRange.romEnd);
        saveRamLabels(section, romRange.ramLabels);
        saveRamBitLabels(section, romRange.ramBitLabels);
    }
    saveRamLabels("rom", romLabels);
    saveRamLabels("comment", romComments);
    f.write(ini.Serialize().c_str());
    f.close();
    setUnsavedChanges(false);
    return true;
}

bool Database::load(const QString & file)
{
    auto oldUnsavedChanges = unsavedChanges();
    QFile f(file);
    if(!f.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Failed to open file...";
        return false;
    }
    auto data = f.readAll().toStdString();
    f.close();
    Utf8Ini ini;
    int errorLine = -1;
    if(!ini.Deserialize(data, errorLine))
    {
        qDebug() << QString("Failed to deserialize INI (line %d)").arg(errorLine);
        return false;
    }
    for(auto & section : ini.Sections())
    {
        auto qsection = QString::fromStdString(section);
        if(section == "chip")
        {
            //ignore
        }
        else if(section == "ram")
        {
            for(auto & key : ini.Keys(section))
            {
                bool ok = false;
                auto ramAddrText = QString::fromStdString(key);
                auto bitIdx = ramAddrText.indexOf('.');
                if(bitIdx != -1)
                {
                    bool ok1 = false, ok2 = false;
                    auto ramAddr = ramAddrText.left(bitIdx).toUShort(&ok1, 16);
                    auto bitAddr = ramAddrText.right(1).toUShort(&ok2);
                    if(ok1 && ok2)
                        setGlobalRamBitLabel(ramAddr, bitAddr, QString::fromStdString(ini.GetValue(section, key)));
                    else
                        qDebug() << "global bit nein";
                }
                else
                {
                    auto ramAddr = ramAddrText.toUShort(&ok, 16);
                    if(ok)
                        setGlobalRamLabel(ramAddr, QString::fromStdString(ini.GetValue(section, key)));
                }
            }
        }
        else if(qsection.startsWith("ram@"))
        {
            auto midIdx = qsection.indexOf('-');
            if(midIdx != -1)
            {
                auto startText = qsection.mid(4, midIdx - 4);
                auto endText = qsection.mid(midIdx + 1);
                bool ok1 = false, ok2 = false;
                auto start = startText.toUShort(&ok1, 16);
                auto end = endText.toUShort(&ok2, 16);
                if(ok1 && ok2)
                {
                    auto romRange = addRomRange(start, end);
                    for(auto & key : ini.Keys(section))
                    {
                        bool ok = false;
                        auto ramAddrText = QString::fromStdString(key);
                        auto bitIdx = ramAddrText.indexOf('.');
                        if(bitIdx != -1)
                        {
                            bool ok1 = false, ok2 = false;
                            auto ramAddr = ramAddrText.left(bitIdx).toUShort(&ok1, 16);
                            auto bitAddr = ramAddrText.right(1).toUShort(&ok2);
                            if(ok1 && ok2)
                                romRange->ramBitLabels.insert({ramAddr, bitAddr}, QString::fromStdString(ini.GetValue(section, key)));
                            else
                                qDebug() << "local ram bit nein";
                        }
                        else
                        {
                            auto ramAddr = ramAddrText.toUShort(&ok, 16);
                            if(ok)
                                romRange->ramLabels.insert(ramAddr, QString::fromStdString(ini.GetValue(section, key)));
                        }
                    }
                }
                else
                {
                    qDebug() << QString("Incorrectly formatted section '%1'").arg(qsection);
                    return false;
                }
            }
            else
            {
                qDebug() << QString("Incorrectly formatted section '%1'").arg(qsection);
                return false;
            }
        }
        else if(section == "rom")
        {
            for(auto & key : ini.Keys(section))
            {
                bool ok = false;
                auto romAddr = QString(key.c_str()).toUShort(&ok, 16);
                if(ok)
                    setRomLabel(romAddr, QString::fromStdString(ini.GetValue(section, key)));
            }
        }
        else if(section == "comment")
        {
            for(auto & key : ini.Keys(section))
            {
                bool ok = false;
                auto romAddr = QString(key.c_str()).toUShort(&ok, 16);
                if(ok)
                    setRomComment(romAddr, QString::fromStdString(ini.GetValue(section, key)));
            }
        }
        else
        {
            qDebug() << "unsupported section" << section.c_str();
        }
    }
    setUnsavedChanges(oldUnsavedChanges);
    return true;
}

void Database::clear()
{
    romLabels.clear();
    romComments.clear();
    romRanges.clear();
    globalRamLabels.clear();
    globalRamBitLabels.clear();
    setUnsavedChanges(false);
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
    setUnsavedChanges();
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
    setUnsavedChanges();
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
    setUnsavedChanges();
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
    setUnsavedChanges();
}

Database::RomRange const* Database::findRomRangeConst(uint16_t romAddr) const
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
    setUnsavedChanges();
    return &romRanges.back();
}

QVector<QPair<uint16_t, uint16_t>> Database::getRomRanges() const
{
    QVector<QPair<uint16_t, uint16_t>> ranges;
    for(auto & range : romRanges)
        ranges.push_back({range.romStart, range.romEnd});
    return ranges;
}

bool Database::deleteRomRange(uint16_t romAddr)
{
    for(auto it = romRanges.begin(); it != romRanges.end(); ++it)
    {
        if(romAddr >= it->romStart && romAddr <= it->romEnd)
        {
            romRanges.erase(it);
            setUnsavedChanges();
            return true;
        }
    }
    return false;
}

QString Database::findLocalRamLabelByAddr(uint16_t romAddr, uint16_t ramAddr) const
{
    auto range = findRomRangeConst(romAddr);
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
    {
        range->ramLabels[ramAddr] = label;
        setUnsavedChanges();
    }
}

QString Database::findLocalRamBitLabelByAddr(uint16_t romAddr, uint16_t ramAddr, uint8_t bit) const
{
    auto range = findRomRangeConst(romAddr);
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
    {
        range->ramBitLabels[{ramAddr, bit}] = label;
        setUnsavedChanges();
    }
}
