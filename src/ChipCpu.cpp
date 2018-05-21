#include "ChipCpu.h"

ChipCpu::ChipCpu(SN8F2288* chip, QObject* parent)
    : QThread(parent), chip(chip)
{
    connect(this, SIGNAL(haltCpu()), this, SLOT(haltCpuSlot()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(stepCpu()), this, SLOT(stepCpuSlot()), Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(runCpu()), this, SLOT(runCpuSlot()), Qt::BlockingQueuedConnection);
    clock = new QTimer(this);
    connect(clock, SIGNAL(timeout()), this, SLOT(stepCpuClockSlot()));
}

void ChipCpu::haltCpuSlot()
{
    if(clock->isActive())
    {
        clock->stop();
        emit cpuPaused();
    }
}

void ChipCpu::stepCpuSlot()
{
    if(!clock->isActive())
    {
        chip->step();
        emit cpuPaused();
    }
}

void ChipCpu::stepCpuClockSlot()
{
    if(!chip->step())
        clock->stop();
}

void ChipCpu::runCpuSlot()
{
    if(!clock->isActive())
    {
        clock->start();
        emit cpuResumed();
    }
}
