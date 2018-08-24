#pragma once

struct TimerCounter
{
    int whoami;
    Register<0xCE, 0, 14> & PC;

    TimerCounter(int whoami, Register<0xCE, 0, 14> & PC) : whoami(whoami), PC(PC) { }

    // Page 95
    bool ENB = false; //TC0 counter control bit (0: disable, 1: enable).
    uint8_t RATE = 0; //TC0 internal clock select bits. (Fcpu / (256 >> RATE))
    bool CKS = false; //TC0 clock source select bit. (0: internal clock, 1: external clock from P0.0/INT0 pin)
    bool ALOAD = false; //Auto-reload control bit. Only valid when PWM0OUT = 0. (0: disable, 1: enable)
    bool OUT = false; //TC0 time out toggle signal output control bit. Only valid when PWM0OUT = 0. (0: Disable, P5.3+n is I/O function, 1: Enable, P5.3+n is output OUT signal.)
    bool PWM = false; //PWM output control bit. (0 = Disable PWM output., 1 = Enable PWM output. PWM duty controlled by OUT, ALOAD bits.)

    // Page 97
    uint8_t COUNTER = 0; //TODO: wtf initial value

    // Page 98 (TODO: look at buffering)
    uint8_t AUTOLOAD = 0;

    uint8_t getMode()
    {
        printf("Timer%d: get MODE (PC: 0x%04x)\n", whoami, PC.get());
        return uint8_t((ENB << 7) | ((RATE & 7) << 4) | (CKS << 3) | (ALOAD << 2) | (OUT << 1) | (PWM << 0));
    }

    void setMode(uint8_t mode)
    {
        ENB = GETBIT(mode, 7);
        RATE = (mode >> 4) & 7;
        CKS = GETBIT(mode, 3);
        ALOAD = GETBIT(mode, 2);
        OUT = GETBIT(mode, 1);
        PWM = GETBIT(mode, 0);

        printf("Timer%d: set MODE to 0x%02x (PC: 0x%04x, ENB: %d, RATE: %u, CKS: %d, ALOAD: %d, OUT: %d, PWM: %d)\n",
               whoami,
               mode,
               PC.get(),
               ENB,
               RATE,
               CKS,
               ALOAD,
               OUT,
               PWM);

        if(CKS || OUT || PWM)
            printf("TODO: Timer%d state not implemented!\n", whoami);
    }

    uint8_t getCounter()
    {
        printf("Timer%d: get COUNTER (PC: 0x%04x)\n", whoami, PC.get());
        return COUNTER;
    }

    void setCounter(uint8_t counter)
    {
        printf("Timer%d: set COUNTER to 0x%02x (PC: 0x%04x)\n", whoami, counter, PC.get());
        COUNTER = counter;
    }

    uint8_t getAutoload()
    {
        printf("Timer%d: get AUTOLOAD (PC: 0x%04x)\n", whoami, PC.get());
        return AUTOLOAD;
    }

    void setAutoload(uint8_t autoload)
    {
        printf("Timer%d: set AUTOLOAD to 0x%02x (PC: 0x%04x)\n", whoami, autoload, PC.get());
        AUTOLOAD = autoload;
    }

    uint32_t stepCounter;

    bool step()
    {
        if(!ENB)
            return false;

        stepCounter++;
        const auto Fcpu = 12000;
        const auto clock = Fcpu / (256 >> RATE); //64
        if((stepCounter % clock) == 0)
            COUNTER++;

        if(COUNTER == 0) //overflow
        {
            if(ALOAD)
                COUNTER = AUTOLOAD;
            printf("Timer%d overflow (%d steps, PC: 0x%04x)!\n", whoami, stepCounter, PC.get());
            stepCounter = 0;
            return true;
        }

        return false;
    }
};
