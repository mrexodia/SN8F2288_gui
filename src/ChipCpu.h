#ifndef CHIPCPU_H
#define CHIPCPU_H

#include "sn8f2288_chip.h"
#include <QThread>
#include <QTimer>

class ChipCpu : public QThread
{
    Q_OBJECT
public:
    ChipCpu(SN8F2288* chip, QObject* parent = nullptr);

signals:
    void haltCpu();
    void stepCpu();
    void runCpu();

private slots:
    void haltCpuSlot();
    void stepCpuSlot();
    void runCpuSlot();

protected:

private:
    SN8F2288* chip;
    QTimer* clock;
};

#endif // CHIPCPU_H
