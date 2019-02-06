#include "ChipCpu.h"
#include <QTime>
#include <QDebug>


ChipCpu::ChipCpu(SN8F2288* chip, QObject* parent) : QThread(parent), chip(chip) {}

void ChipCpu::run()
{
    //TODO: refactor this to use locks, etc
    auto told = QTime::currentTime();
    auto count = 0;
    while(bCpuActive)
    {
        bWaiting = true;
        while(bWaiting)
            QThread::usleep(1);
        bActionReceived = true;

        switch(action)
        {
        case Kill:
            break;
        case Step:
            if(chip->step())
                paused(SingleStep);
            else
                paused(SingleStepFailed);
            break;
        case Run:
            told = QTime::currentTime();
            bRunning = true;
            while(bRunning)
            {
                /*auto tnew = QTime::currentTime();
                auto diff = told.msecsTo(tnew);
                if(diff >= 10000)
                {
                    told = tnew;
                    qDebug() << QString("%1 steps/s").arg(count);
                    count = 0;
                }*/
                if(!chip->step())
                {
                    bRunning = false;
                    paused(RunStepFailed);
                    goto runFailed;
                }
                count++;

                /*if(chip->PC.get() == 8) //super dumb breakpoint
                    bRunning = false;*/
            }
            paused(RunPaused);
runFailed:
            break;
        }
        auto tnew = QTime::currentTime();
        auto diff = told.msecsTo(tnew);
        qDebug() << QString("%1 steps in %2ms").arg(count).arg(diff);
        told = tnew;
        count = 0;
    }
}

void ChipCpu::stepCpu()
{
    if(!bCpuActive)
    {
        qDebug() << "trying to step on dead cpu";
        return;
    }
    if(bRunning)
    {
        //nein
        return;
    }
    bActionReceived = false;
    action = Step;
    bWaiting = false;
    while(!bActionReceived)
        QThread::usleep(1);
}

void ChipCpu::runCpu()
{
    if(!bCpuActive)
    {
        qDebug() << "trying to run on dead cpu";
        return;
    }
    if(bRunning)
    {
        return;
    }
    bActionReceived = false;
    action = Run;
    bWaiting = false;
    while(!bActionReceived)
        QThread::sleep(1);
}

void ChipCpu::haltCpu()
{
    if(!bCpuActive)
    {
        qDebug() << "trying to halt dead cpu";
        return;
    }
    if(!bRunning)
    {
        return;
    }
    bWaiting = false;
    bRunning = false;
    while(!bWaiting)
        QThread::sleep(1);
}

void ChipCpu::stopCpu()
{
    if(!bCpuActive)
    {
        qDebug() << "trying to kill dead cpu";
        return;
    }
    bActionReceived = false;
    action = Kill;
    bWaiting = false;

    bRunning = false;
    bCpuActive = false;
}

ChipCpu::~ChipCpu() = default;
