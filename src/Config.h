#ifndef CONFIG_H
#define CONFIG_H

#include <QColor>
#include <QMap>

#define Config() ConfigStore::instance()

class ConfigStore
{
public:
    static ConfigStore & instance();

    QColor disassemblerBackground = "#FFF8F0";
    QColor highlightColor = "#FF0000";
    QColor pcForeground = "#FFFFFF", pcBackground = "#000000";
    QColor addrForeground = "#808080", addrBackground = Qt::transparent;
    QColor spaceForeground = "#000000", spaceBackground = Qt::transparent;
    QColor commaForeground = "#000000", commaBackground = Qt::transparent;
    QColor labelForeground = "#FF0000", labelBackground = Qt::transparent;
    QColor mnemonicForeground = "#000000", mnemonicBackground = Qt::transparent;
    QColor regForeground = "#008300", regBackground = Qt::transparent;
    QColor immForeground = "#828200", immBackground = Qt::transparent;
    QColor ramForeground = "#B30059", ramBackground = Qt::transparent;
    QColor bitForeground = "#B30059", bitBackground = Qt::transparent;
    QColor romForeground = "#000000", romBackground = "#FFFF00";
    QColor commentForeground = "#1E7A00", commentBackground = Qt::transparent;
    QColor otherForeground = "#000000", otherBackground = Qt::transparent;
    QMap<QString, QPair<QColor, QColor>> mnemonicMap;
};

#endif // CONFIG_H
