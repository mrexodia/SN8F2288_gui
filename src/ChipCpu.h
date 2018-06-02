#ifndef CHIPCPU_H
#define CHIPCPU_H

#include "sn8f2288_chip.h"
#include <QThread>
#include <QTimer>

class ChipCpu : public QThread
{
    Q_OBJECT
public:
    enum PauseReason
    {
        SingleStep,
        SingleStepFailed,
        RunStepFailed,
        RunPaused
    };

signals:
    void paused(int reason);

public:
    SN8F2288* chip;
    ChipCpu(SN8F2288* chip, QObject* parent = nullptr);

    void run() override;
    virtual ~ChipCpu() override;

    enum
    {
        Step,
        Run,
        Kill
    } action;

    bool bWaiting = true;
    bool bActionReceived = false;
    bool bRunning = false;
    bool bCpuActive = true;

    void stepCpu();
    void runCpu();
    void haltCpu();
    void stopCpu();
};

#endif // CHIPCPU_H
