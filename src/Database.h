#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QMap>
#include <QVector>

class Database
{
public:
    struct RomRange
    {
        uint16_t romStart = 0;
        uint16_t romEnd = 0;
        QMap<uint16_t, QString> ramLabels;
        QMap<QPair<uint16_t, uint8_t>, QString> ramBitLabels;
    };

    bool save(const QString & file);
    bool load(const QString & file);
    void clear();
    bool empty() const;
    bool unsavedChanges() const { return hasUnsavedChanges; }
    void setUnsavedChanges(bool value = true) { hasUnsavedChanges = value; }

    QString findRomLabelByAddr(uint16_t addr) const;
    bool findRomLabelByName(const QString & label, uint16_t & addr) const;
    void setRomLabel(uint16_t addr, const QString & label);

    QString findRomCommentByAddr(uint16_t addr) const;
    void setRomComment(uint16_t addr, const QString & comment);

    QString findGlobalRamLabelByAddr(uint16_t addr) const;
    void setGlobalRamLabel(uint16_t addr, const QString & label);

    QString findGlobalRamBitLabelByAddr(uint16_t addr, uint8_t bit) const;
    void setGlobalRamBitLabel(uint16_t addr, uint8_t bit, const QString & label);

    RomRange const* findRomRangeConst(uint16_t romAddr) const;
    RomRange* addRomRange(uint16_t romStart, uint16_t romEnd);
    QVector<QPair<uint16_t, uint16_t>> getRomRanges() const;
    bool deleteRomRange(uint16_t romAddr);

    QString findLocalRamLabelByAddr(uint16_t romAddr, uint16_t ramAddr) const;
    void setLocalRamLabel(uint16_t romAddr, uint16_t ramAddr, const QString & label);

    QString findLocalRamBitLabelByAddr(uint16_t romAddr, uint16_t ramAddr, uint8_t bit) const;
    void setLocalRamBitLabel(uint16_t romAddr, uint16_t ramAddr, uint8_t bit, const QString & label);

private:
    bool hasUnsavedChanges = false;

    QMap<uint16_t, QString> romLabels;
    QMap<uint16_t, QString> romComments;

    QVector<RomRange> romRanges;
    QMap<uint16_t, QString> globalRamLabels;
    QMap<uint16_t, QString> reservedGlobalRamLabels;
    QMap<QPair<uint16_t, uint8_t>, QString> globalRamBitLabels;
    QMap<QPair<uint16_t, uint8_t>, QString> reservedGlobalRamBitLabels;

    RomRange* findRomRange(uint16_t romAddr);
    void setReservedGlobalRamLabel(uint16_t addr, const QString & label);
    void setReservedGlobalRamBitLabel(uint16_t addr, uint8_t bit, const QString & label);
};

#endif // DATABASE_H
