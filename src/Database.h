#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QMap>

class Database
{
public:
    Database();

    bool save(const QString & file) const;
    bool load(const QString & file);
    void clear();

    QString findRomLabelByAddr(uint16_t addr) const;
    bool findRomLabelByName(const QString & label, uint16_t & addr) const;
    void setRomLabel(uint16_t addr, const QString & label);

    QString findRomCommentByAddr(uint16_t addr) const;
    void setRomComment(uint16_t addr, const QString & comment);

private:
    QMap<uint16_t, QString> romLabels;
    QMap<uint16_t, QString> romComments;
};

#endif // DATABASE_H
