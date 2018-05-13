#include "Database.h"
#include <QDebug>

Database::Database()
{

}

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
