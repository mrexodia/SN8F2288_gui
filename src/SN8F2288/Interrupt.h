#pragma once

#include <cstdint>

struct INTRQ1Data
{
    bool TC0IRQ = false;
    bool TC1IRQ = false;
    bool TC2IRQ = false;
    bool UTTXIRQ = false;
    bool UTRXIRQ = false;
    bool MSPIRQ = false;
    bool P0IRQ = false;
    bool P1IRQ = false;

    uint8_t get()
    {
        return uint8_t((TC0IRQ << 0) | (TC1IRQ << 1) | (TC2IRQ << 2) | (UTTXIRQ << 3) | (UTRXIRQ << 4) | (MSPIRQ << 5) | (P0IRQ << 6) | (P1IRQ << 7));
    }

    void set(uint8_t value)
    {
        TC0IRQ = GETBIT(value, 0);
        TC1IRQ = GETBIT(value, 1);
        TC2IRQ = GETBIT(value, 2);
        UTTXIRQ = GETBIT(value, 3);
        UTRXIRQ = GETBIT(value, 4);
        MSPIRQ = GETBIT(value, 5);
        P0IRQ = GETBIT(value, 6);
        P1IRQ = GETBIT(value, 7);
    }
};

struct INTEN1Data
{
    bool TC0IEN = false;
    bool TC1IEN = false;
    bool TC2IEN = false;
    bool UTTXIEN = false;
    bool UTRXIEN = false;
    bool MSPIEN = false;
    bool P0IEN = false;
    bool P1IEN = false;

    uint8_t get()
    {
        return uint8_t((TC0IEN << 0) | (TC1IEN << 1) | (TC2IEN << 2) | (UTTXIEN << 3) | (UTRXIEN << 4) | (MSPIEN << 5) | (P0IEN << 6) | (P1IEN << 7));
    }

    void set(uint8_t value)
    {
        TC0IEN = GETBIT(value, 0);
        TC1IEN = GETBIT(value, 1);
        TC2IEN = GETBIT(value, 2);
        UTTXIEN = GETBIT(value, 3);
        UTRXIEN = GETBIT(value, 4);
        MSPIEN = GETBIT(value, 5);
        P0IEN = GETBIT(value, 6);
        P1IEN = GETBIT(value, 7);
    }
};

struct INTRQData
{
    bool P00IRQ = false;
    bool P01IRQ = false;
    bool WAKEIRQ = false;
    bool SIOIRQ = false;
    bool T0IRQ = false;
    bool T1IRQ = false;
    bool USBIRQ = false;
    bool ADCIRQ = false;

    uint8_t get()
    {
        return uint8_t((P00IRQ << 0) | (P01IRQ << 1) | (WAKEIRQ << 2) | (SIOIRQ << 3) | (T0IRQ << 4) | (T1IRQ << 5) | (USBIRQ << 6) | (ADCIRQ << 7));
    }

    void set(uint8_t value)
    {
        P00IRQ = GETBIT(value, 0);
        P01IRQ = GETBIT(value, 1);
        WAKEIRQ = GETBIT(value, 2);
        SIOIRQ = GETBIT(value, 3);
        T0IRQ = GETBIT(value, 4);
        T1IRQ = GETBIT(value, 5);
        USBIRQ = GETBIT(value, 6);
        ADCIRQ = GETBIT(value, 7);
    }
};

struct INTENData
{
    bool P00IEN = false;
    bool P01IEN = false;
    bool WAKEIEN = false;
    bool SIOIEN = false;
    bool T0IEN = false;
    bool T1IEN = false;
    bool USBIEN = false;
    bool ADCIEN = false;

    uint8_t get()
    {
        return uint8_t((P00IEN << 0) | (P01IEN << 1) | (WAKEIEN << 2) | (SIOIEN << 3) | (T0IEN << 4) | (T1IEN << 5) | (USBIEN << 6) | (ADCIEN << 7));
    }

    void set(uint8_t value)
    {
        P00IEN = GETBIT(value, 0);
        P01IEN = GETBIT(value, 1);
        WAKEIEN = GETBIT(value, 2);
        SIOIEN = GETBIT(value, 3);
        T0IEN = GETBIT(value, 4);
        T1IEN = GETBIT(value, 5);
        USBIEN = GETBIT(value, 6);
        ADCIEN = GETBIT(value, 7);
    }
};
