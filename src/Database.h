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

    bool save(const QString & file) const;
    bool load(const QString & file);
    void clear();

    QString findRomLabelByAddr(uint16_t addr) const;
    bool findRomLabelByName(const QString & label, uint16_t & addr) const;
    void setRomLabel(uint16_t addr, const QString & label);

    QString findRomCommentByAddr(uint16_t addr) const;
    void setRomComment(uint16_t addr, const QString & comment);

    QString findGlobalRamLabelByAddr(uint16_t addr) const;
    void setGlobalRamLabel(uint16_t addr, const QString & label);

    QString findGlobalRamBitLabelByAddr(uint16_t addr, uint8_t bit) const;
    void setGlobalRamBitLabel(uint16_t addr, uint8_t bit, const QString & label);

    RomRange const* findRomRange(uint16_t romAddr) const;
    RomRange* findRomRange(uint16_t romAddr);
    RomRange* addRomRange(uint16_t romStart, uint16_t romEnd);

    QString findLocalRamLabelByAddr(uint16_t romAddr, uint16_t ramAddr) const;
    void setLocalRamLabel(uint16_t romAddr, uint16_t ramAddr, const QString & label);

    QString findLocalRamBitLabelByAddr(uint16_t romAddr, uint16_t ramAddr, uint8_t bit) const;
    void setLocalRamBitLabel(uint16_t romAddr, uint16_t ramAddr, uint8_t bit, const QString & label);

private:
    QMap<uint16_t, QString> romLabels;
    QMap<uint16_t, QString> romComments;

    QVector<RomRange> romRanges;
    QMap<uint16_t, QString> globalRamLabels;
    QMap<QPair<uint16_t, uint8_t>, QString> globalRamBitLabels;
};

#endif // DATABASE_H
