#include "ChipCpu.h"

ChipCpu::ChipCpu(SN8F2288* chip, QObject* parent)
    : QThread(parent), chip(chip)
{
    connect(this, SIGNAL(haltCpu()), this, SLOT(haltCpuSlot()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(stepCpu()), this, SLOT(stepCpuSlot()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(runCpu()), this, SLOT(runCpuSlot()), Qt::BlockingQueuedConnection);
    clock = new QTimer(this);
    connect(clock, SIGNAL(timeout()), this, SLOT(stepCpuSlot()));
}

void ChipCpu::haltCpuSlot()
{
    clock->stop();
}

void ChipCpu::stepCpuSlot()
{
    if(!chip->step())
        clock->stop();
}

void ChipCpu::runCpuSlot()
{
    clock->start();
}
