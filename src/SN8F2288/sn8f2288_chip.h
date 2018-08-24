#include <vector>
#include <chrono>
#include <thread>
#include <string>

#include <stdint.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif //_MSC_VER

#define GETBIT(value, n) (((value >> n) & 1) != 0)

#include "InstrEntry.h"
#include "Memory.h"
#include "Interrupt.h"
#include "USB.h"
#include "TimerCounter.h"

#undef GETBIT

extern InstrEntry entries[256];

std::vector<uint8_t> readFile(const char* filename);
void initializeInstructions();
std::string disassemble(uint16_t address, uint16_t instruction);
const char* instrTypeToString(InstrType type);

enum ResetType
{
    ResetPowerOn,
    ResetWatchdog,
    ResetExternal
};

struct SN8F2288
{
    static uint8_t ramReadStatic(void* thisptr, uint16_t index)
    {
        return ((SN8F2288*)thisptr)->ramRead(index);
    }

    static void ramWriteStatic(void* thisptr, uint16_t index, uint8_t value)
    {
        ((SN8F2288*)thisptr)->ramWrite(index, value);
    }

    SN8F2288()
        : ram(this, ramReadStatic, ramWriteStatic),
          ACC(0),
          R(ram), Z(ram), Y(ram), PFLAG(ram), RBANK(ram),
          TC0M(ram), TC0C(ram), TC0R(ram),
          TC1M(ram), TC1C(ram), TC1R(ram),
          TC2M(ram), TC2C(ram), TC2R(ram),
          UDA(ram), USTATUS(ram), EP0OUT_CNT(ram), USB_INT_EN(ram), EP_ACK(ram), EP_NAK(ram), UE0R(ram), UE1R(ram), UE1R_C(ram), UE2R(ram), UE2R_C(ram), UE3R(ram), UR3R_C(ram), UE4R(ram), UE4R_C(ram), EP2FIFO_ADDR(ram), EP3FIFO_ADDR(ram), EP4FIFO_ADDR(ram), UDP0(ram), UDR0_R(ram), UDR0_W(ram), UPID(ram), UTOGGLE(ram),
          URTX(ram), URRX(ram), URBRC(ram), URTXD1(ram), URTXD2(ram), URRXD1(ram), URRXD2(ram),
          SIOM(ram), SIOR(ram), SIOB(ram), P0M(ram), ADM(ram), ADB(ram), ADR(ram), P4CON(ram), PECMD(ram), PEROM_L(ram), PEROM_H(ram), PERAM_L(ram), PERAM_CNT(ram), PEDGE(ram), P1W(ram), P1M(ram), P2M(ram), P4M(ram), P5M(ram), INTRQ1(ram), INTEN1(ram), INTRQ(ram), INTEN(ram), OSCM(ram), WDTR(ram), PCL(ram), PCH(ram), P0(ram), P1(ram), P2(ram), P4(ram), P5(ram), T0M(ram), T0C(ram), T1M(ram), T1CL(ram), T1CH(ram), STKP(ram), P0UR(ram), P1UR(ram), P2UR(ram), P4UR(ram), P5UR(ram), AT_YZ(ram), P1OC(ram), MSPSTAT(ram), MSPM1(ram), MSPM2(ram), MSPBUF(ram), MSPADR(ram), STK7L(ram), STK7H(ram), STK6L(ram), STK6H(ram), STK5L(ram), STK5H(ram), STK4L(ram), STK4H(ram), STK3L(ram), STK3H(ram), STK2L(ram), STK2H(ram), STK1L(ram), STK1H(ram), STK0L(ram), STK0H(ram),
          PC(ram), YZ(ram),
          GIE(ram), STK7(ram), STK6(ram), STK5(ram), STK4(ram), STK3(ram), STK2(ram), STK1(ram), STK0(ram),
          FZ(ram), FDC(ram), FC(ram), FNPD(ram), FNT0(ram),
          pin1(i2c), pin2(i2c),
          usb(fifo, PC),
          tc0(0, PC), tc1(1, PC), tc2(2, PC)
    {
        initializeInstructions();
    }

    //12K words ROM (page 16)
    BaseArray<uint16_t, 0x3000> rom;
    //512 X 8-bit RAM (page 27), actually 640 bytes?
    uint8_t ramData[0x280];
    ArrayFunctions<uint8_t> ram;
    //136 x 8-bit RAM for USB DATA FIFO
    BaseArray<uint8_t, 136> fifo;
    uint8_t ACC; //not in RAM (TODO: logging)

    uint8_t ACC_push = 0;
    uint8_t PFLAG_push = 0;

    Register<0x82> R;
    Register<0x83> Z;
    Register<0x84> Y;
    Register<0x86> PFLAG;
    Register<0x87, 0, 2> RBANK;
    Register<0x88> TC0M;
    Register<0x89> TC0C;
    Register<0x8A> TC0R; //W
    Register<0x8B> TC1M;
    Register<0x8C> TC1C;
    Register<0x8D> TC1R; //W
    Register<0x8E> TC2M;
    Register<0x8F> TC2C;
    Register<0x90> TC2R; //W
    //USB
    Register<0x91> UDA;
    Register<0x92> USTATUS;
    Register<0x93> EP0OUT_CNT;
    Register<0x94> USB_INT_EN;
    Register<0x95> EP_ACK;
    Register<0x96> EP_NAK;
    Register<0x97> UE0R;
    Register<0x98> UE1R;
    Register<0x99> UE1R_C;
    Register<0x9A> UE2R;
    Register<0x9B> UE2R_C;
    Register<0x9C> UE3R;
    Register<0x9D> UR3R_C;
    Register<0x9E> UE4R;
    Register<0x9F> UE4R_C;
    Register<0xA0> EP2FIFO_ADDR;
    Register<0xA1> EP3FIFO_ADDR;
    Register<0xA2> EP4FIFO_ADDR;
    Register<0xA3> UDP0;
    Register<0xA5> UDR0_R;
    Register<0xA6> UDR0_W;
    Register<0xA7> UPID;
    Register<0xA8> UTOGGLE;

    Register<0xA9> URTX;
    Register<0xAA> URRX;
    Register<0xAB> URBRC;
    Register<0xAC> URTXD1;
    Register<0xAD> URTXD2;
    Register<0xAE> URRXD1; //R
    Register<0xAF> URRXD2; //R
    Register<0xB0> SIOM;
    Register<0xB1> SIOR; //W
    Register<0xB2> SIOB;
    Register<0xB5> P0M;
    Register<0xB6> ADM;
    Register<0xB7> ADB; //R
    Register<0xB8> ADR;
    Register<0xB9> P4CON; //W
    Register<0xBA> PECMD; //W
    Register<0xBB> PEROM_L;
    Register<0xBC> PEROM_H;
    Register<0xBD> PERAM_L;
    Register<0xBE> PERAM_CNT;
    Register<0xBF> PEDGE;
    Register<0xC0> P1W;
    Register<0xC1> P1M;
    Register<0xC2> P2M;
    Register<0xC4> P4M;
    Register<0xC5> P5M;
    Register<0xC6> INTRQ1;
    Register<0xC7> INTEN1;
    Register<0xC8> INTRQ;
    Register<0xC9> INTEN;
    Register<0xCA> OSCM;
    Register<0xCC> WDTR; //W
    Register<0xCE> PCL;
    Register<0xCF> PCH;
    Register<0xD0> P0;
    Register<0xD1> P1;
    Register<0xD2> P2;
    Register<0xD4> P4;
    Register<0xD5> P5;
    Register<0xD8> T0M;
    Register<0xD9> T0C;
    Register<0xDA> T1M;
    Register<0xDB> T1CL;
    Register<0xDC> T1CH;
    Register<0xDF, 0, 3> STKP;
    Register<0xE0> P0UR; //W
    Register<0xE1> P1UR; //W
    Register<0xE2> P2UR; //W
    Register<0xE4> P4UR; //W
    Register<0xE5> P5UR; //W
    Register<0xE7> AT_YZ; //TODO
    Register<0xE9> P1OC; //W
    Register<0xEA> MSPSTAT; //R
    Register<0xEB> MSPM1;
    Register<0xEC> MSPM2;
    Register<0xED> MSPBUF;
    Register<0xEE> MSPADR;
    Register<0xF0> STK7L;
    Register<0xF1> STK7H;
    Register<0xF2> STK6L;
    Register<0xF3> STK6H;
    Register<0xF4> STK5L;
    Register<0xF5> STK5H;
    Register<0xF6> STK4L;
    Register<0xF7> STK4H;
    Register<0xF8> STK3L;
    Register<0xF9> STK3H;
    Register<0xFA> STK2L;
    Register<0xFB> STK2H;
    Register<0xFC> STK1L;
    Register<0xFD> STK1H;
    Register<0xFE> STK0L;
    Register<0xFF> STK0H;

    //program counter
    Register<0xCE, 0, 14> PC;
    Register<0x83, 0, 14> YZ;
    //stack pointer
    Register<0xDF, 7, 1> GIE;
    Register<0xF0, 0, 14> STK7;
    Register<0xF2, 0, 14> STK6;
    Register<0xF4, 0, 14> STK5;
    Register<0xF6, 0, 14> STK4;
    Register<0xF8, 0, 14> STK3;
    Register<0xFA, 0, 14> STK2;
    Register<0xFC, 0, 14> STK1;
    Register<0xFE, 0, 14> STK0;
    //flags
    Register<0x86, 0, 1> FZ;
    Register<0x86, 1, 1> FDC;
    Register<0x86, 2, 1> FC;
    Register<0x86, 6, 1> FNPD;
    Register<0x86, 7, 1> FNT0;

    struct I2C
    {

    } i2c;

    struct Pin1
    {
        I2C & i2c;

        Pin1(I2C & i2c) : i2c(i2c) { }

        uint8_t MODE;
        uint8_t WAKE;
        uint8_t PULLUP;
        uint8_t OPENDRAIN;
        uint8_t VALUE;

        uint8_t getMode()
        {
            printf("Pin1: get mode\n");
            return MODE;
        }

        void setMode(uint8_t mode)
        {
            printf("Pin1: set mode to 0x%02x\n", mode);
            MODE = mode;
        }

        uint8_t getWake()
        {
            printf("Pin1: get wake\n");
            return WAKE;
        }

        void setWake(uint8_t wake)
        {
            printf("Pin1: set wake to 0x%02x\n", wake);
            WAKE = wake;
        }

        uint8_t getPullup()
        {
            printf("Pin1: get pull-up\n");
            return PULLUP;
        }

        void setPullup(uint8_t pullup)
        {
            printf("Pin1: set pull-up to 0x%02x\n", pullup);
            PULLUP = pullup;
        }

        uint8_t getOpendrain()
        {
            printf("Pin1: get open-drain\n");
            return OPENDRAIN;
        }

        void setOpendrain(uint8_t opendrain)
        {
            printf("Pin1: set open-drain to 0x%02x\n", opendrain);
            OPENDRAIN = opendrain;
        }

        uint8_t getValue()
        {
            return VALUE;
        }

        void setValue(uint8_t value)
        {
            VALUE = value;
        }
    } pin1;

    struct Pin2
    {
        I2C & i2c;

        Pin2(I2C & i2c) : i2c(i2c) { }
    } pin2; //TODO: add more pins?

    INTRQ1Data intrq1;
    INTEN1Data inten1;
    INTRQData intrq;
    INTENData inten;

    UsbData usb;

    TimerCounter tc0, tc1, tc2;

#define regWhitelist() \
    case decltype(STKP)::index: \
    case decltype(PFLAG)::index: \
    case decltype(PCL)::index: \
    case decltype(PCH)::index: \
    case decltype(R)::index: \
    case decltype(Y)::index: \
    case decltype(Z)::index: \
    case decltype(STK7L)::index: \
    case decltype(STK7H)::index: \
    case decltype(STK6L)::index: \
    case decltype(STK6H)::index: \
    case decltype(STK5L)::index: \
    case decltype(STK5H)::index: \
    case decltype(STK4L)::index: \
    case decltype(STK4H)::index: \
    case decltype(STK3L)::index: \
    case decltype(STK3H)::index: \
    case decltype(STK2L)::index: \
    case decltype(STK2H)::index: \
    case decltype(STK1L)::index: \
    case decltype(STK1H)::index: \
    case decltype(STK0L)::index: \
    case decltype(STK0H)::index: \
    case decltype(PEROM_L)::index: \
    case decltype(PEROM_H)::index: \
    case decltype(PERAM_L)::index: \
    case decltype(PERAM_CNT)::index:

    uint8_t ramRead(uint16_t index)
    {
        if(index >= 0 && index <= 0x7F) //BANK 0, General purpose area
        {
            return ramData[index];
        }
        else if(index >= 0x80 && index <= 0xFF) //BANK 0, System registers
        {
            switch(index)
            {
            //USB registers
            case decltype(UDA)::index:
                return usb.getUda();
            case decltype(USTATUS)::index:
                return usb.getUstatus();
            case decltype(EP_ACK)::index:
                return usb.getEp_ack();
            case decltype(UE0R)::index:
                return usb.getUe0r();
            case decltype(UE1R)::index:
                return usb.getUe1r();
            case decltype(UE2R)::index:
                return usb.getUe2r();
            case decltype(UDP0)::index:
                return usb.getUdp0();
            case decltype(UDR0_R)::index:
                return usb.getUdr0_r();
            case decltype(UDR0_W)::index:
                return usb.getUdr0_w();
            case decltype(EP0OUT_CNT)::index:
                return usb.getEp0out_cnt();
            case decltype(UE1R_C)::index:
                return usb.getUe1r_c();
            case decltype(UE2R_C)::index:
                return usb.getUe2r_c();
            case decltype(EP2FIFO_ADDR)::index:
                return usb.getEp2fifo_addr();
            case decltype(UPID)::index:
                return usb.getUpid();
            case decltype(USB_INT_EN)::index:
                return usb.getUsb_int_en();

                //TODO: interrupts
            case decltype(INTRQ1)::index:
                printf("TODO: INTRQ1 read (PC: 0x%04x)\n", PC.get());
                return intrq1.get();
            case decltype(INTEN1)::index:
                printf("TODO: INTEN1 read (PC: 0x%04x)\n", PC.get());
                return inten1.get();
            case decltype(INTRQ)::index:
                printf("TODO: INTRQ read (PC: 0x%04x)\n", PC.get());
                return intrq.get();
            case decltype(INTEN)::index:
                printf("TODO: INTEN read (PC: 0x%04x)\n", PC.get());
                return inten.get();

            //TODO: pins
            case decltype(P0M)::index:
            case decltype(P2M)::index:
            case decltype(P4M)::index:
            case decltype(P5M)::index:
            case decltype(P0UR)::index:
            case decltype(P2UR)::index:
            case decltype(P4UR)::index:
            case decltype(P5UR)::index:
            case decltype(P0)::index:
            case decltype(P2)::index:
            case decltype(P4)::index:
            case decltype(P5)::index:
                printf("TODO: pin 0x%02x read\n", index);
                return ramData[index];
            case 0xc3:
            case 0xd3:
            case 0xe3:
                printf("invalid P3-related read (doing NOOP)!\n");
                return 0;

            case decltype(P1M)::index:
                return pin1.getMode();
            case decltype(P1W)::index:
                return pin1.getWake();
            case decltype(P1UR)::index:
                return pin1.getPullup();
            case decltype(P1OC)::index:
                return pin1.getOpendrain();
            case decltype(P1)::index:
                return pin1.getValue();

            //Timers
            case decltype(TC0M)::index:
                return tc0.getMode();
            case decltype(TC1M)::index:
                return tc1.getMode();
            case decltype(TC2M)::index:
                return tc2.getMode();
            case decltype(TC0C)::index:
                return tc0.getCounter();
            case decltype(TC1C)::index:
                return tc1.getCounter();
            case decltype(TC2C)::index:
                return tc2.getCounter();
            case decltype(TC0R)::index:
                return tc0.getAutoload();
            case decltype(TC1R)::index:
                return tc1.getAutoload();
            case decltype(TC2R)::index:
                return tc2.getAutoload();

            case decltype(AT_YZ)::index:
                return ramRead(YZ.get()); //TODO: possible stack overflow

            case decltype(OSCM)::index:
                printf("TODO: oscillator control register read (PC: 0x%04x)\n", PC.get());
                return ramData[index];

            case decltype(RBANK)::index:
                return ramData[index];

                regWhitelist()
                        return ramData[index];

            default:
                printf("unknown register read at 0x%04x, PC: 0x%04x\n", index, PC.get());
            }
        }
        else if(index >= 0x100 && index <= 0x1FF) //BANK 1, General purpose area
        {
            return ramData[index];
        }
        else if(index >= 0x200 && index <= 0x27F) //BANK 2, General purpose area
        {
            return ramData[index];
        }
        printf("location: 0x%04x\n", index);
        throw std::out_of_range("invalid ram read");
    }

    void ramWrite(uint16_t index, uint8_t value)
    {
        if(index >= 0 && index <= 0x7F) //BANK 0, General purpose area
        {
            ramData[index] = value;
            return;
        }
        else if(index >= 0x80 && index <= 0xFF) //BANK 0, System registers
        {
            switch(index)
            {
            //USB registers
            case decltype(UDA)::index:
                usb.setUda(value);
                return;
            case decltype(USTATUS)::index:
                usb.setUstatus(value);
                return;
            case decltype(EP_ACK)::index:
                usb.setEp_ack(value);
                return;
            case decltype(UE0R)::index:
                usb.setUe0r(value);
                return;
            case decltype(UE1R)::index:
                usb.setUe1r(value);
                return;
            case decltype(UE2R)::index:
                usb.setUe2r(value);
                return;
            case decltype(UDP0)::index:
                usb.setUdp0(value);
                return;
            case decltype(UDR0_R)::index:
                usb.setUdr0_r(value);
                return;
            case decltype(UDR0_W)::index:
                usb.setUdr0_w(value);
                return;
            case decltype(EP0OUT_CNT)::index:
                usb.setEp0out_cnt(value);
                return;
            case decltype(UE1R_C)::index:
                usb.setUe1r_c(value);
                return;
            case decltype(UE2R_C)::index:
                usb.setUe2r_c(value);
                return;
            case decltype(EP2FIFO_ADDR)::index:
                usb.setEp2fifo_addr(value);
                return;
            case decltype(UPID)::index:
                usb.setUpid(value);
                return;
            case decltype(USB_INT_EN)::index:
                usb.setUsb_int_en(value);
                return;

                //TODO: interrupts
            case decltype(INTRQ1)::index:
                printf("TODO: INTRQ1 write, value: 0x%02x (PC: 0x%04x)\n", value, PC.get());
                intrq1.set(value);
                return;
            case decltype(INTEN1)::index:
                printf("TODO: INTEN1 write, value: 0x%02x (PC: 0x%04x)\n", value, PC.get());
                inten1.set(value);
                return;
            case decltype(INTRQ)::index:
                printf("TODO: INTRQ write, value: 0x%02x (PC: 0x%04x)\n", value, PC.get());
                intrq.set(value);
                return;
            case decltype(INTEN)::index:
                printf("TODO: INTEN write, value: 0x%02x (PC: 0x%04x)\n", value, PC.get());
                inten.set(value);
                return;

                //TODO: pins
            case decltype(P0M)::index:
            case decltype(P2M)::index:
            case decltype(P4M)::index:
            case decltype(P5M)::index:
            case decltype(P0UR)::index:
            case decltype(P2UR)::index:
            case decltype(P4UR)::index:
            case decltype(P5UR)::index:
            case decltype(P0)::index:
            case decltype(P2)::index:
            case decltype(P4)::index:
            case decltype(P5)::index:
                printf("TODO: pin 0x%02x write, value: 0x%02x\n", index, value);
                ramData[index] = value;
                return;
            case 0xc3:
            case 0xd3:
            case 0xe3:
                printf("invalid P3-related write, value 0x%02x (doing NOOP)!\n", value);
                return;

            case decltype(P1M)::index:
                pin1.setMode(value);
                return;
            case decltype(P1W)::index:
                pin1.setWake(value);
                return;
            case decltype(P1UR)::index:
                pin1.setPullup(value);
                return;
            case decltype(P1OC)::index:
                pin1.setOpendrain(value);
                return;
            case decltype(P1)::index:
                pin1.setValue(value);
                return;

                //Timers
            case decltype(TC0M)::index:
                tc0.setMode(value);
                return;
            case decltype(TC1M)::index:
                tc1.setMode(value);
                return;
            case decltype(TC2M)::index:
                tc2.setMode(value);
                return;
            case decltype(TC0C)::index:
                tc0.setCounter(value);
                return;
            case decltype(TC1C)::index:
                tc1.setCounter(value);
                return;
            case decltype(TC2C)::index:
                tc2.setCounter(value);
                return;
            case decltype(TC0R)::index:
                tc0.setAutoload(value);
                return;
            case decltype(TC1R)::index:
                tc1.setAutoload(value);
                return;
            case decltype(TC2R)::index:
                tc2.setAutoload(value);
                return;

            case decltype(AT_YZ)::index:
                ramWrite(YZ.get(), value);
                return;

            case decltype(WDTR)::index:
                if(value == 0x5a) // clear watchdog timer
                {
                    //TODO: probably pointless to emulate
                }
                return;

            case decltype(OSCM)::index:
                printf("TODO: oscillator control register write, value: 0x%02x (PC: 0x%04x)\n", value, PC.get());
                ramData[index] = value;
                return;

            case decltype(PECMD)::index:
            {
                auto PEROM = uint16_t(PEROM_H.get() << 8) | (PEROM_L.get() << 0);
                auto PERAMCNT = PERAM_CNT.get();
                auto PERAM = uint16_t(((PERAMCNT & 0x3) << 9) | (PERAM_L.get() << 0));
                PERAMCNT >>= 3;
                if(PERAM > 0x7f && PERAM < 0x100)
                {
                    printf("invalid RAM address 0x%04x for flash operation!\n", PERAM);
                    return;
                }

                if(value == 0x5a) // page program (32 words)
                {
                    if(PEROM % 32)
                    {
                        printf("misaligned ROM address 0x%04x for page program!\n", PEROM);
                        throw std::out_of_range("misaligned ROM address");
                    }
                    printf("performing 0x%x word page program, ROM address: 0x%04x, RAM address: 0x%04x\n", PERAMCNT + 1, PEROM, PERAM);
                    for(uint16_t i = 0, j = 0; i < PERAMCNT + 1; i++, j += 2)
                        rom.set(PEROM + i, ramRead(j + 1) << 8 | ramRead(j));
                }
                else if(value == 0xc3) // page erase (128 words)
                {
                    if(PEROM % 128)
                    {
                        printf("misaligned ROM address 0x%04x for page erase!\n", PEROM);
                        throw std::out_of_range("misaligned ROM address");
                    }
                    printf("performing page erase at 0x%04x\n", PEROM);
                    for(uint16_t i = 0; i < 128; i++)
                        rom.set(PEROM + i, 0);
                }
                else
                {
                    printf("invalid PECMD value 0x%02x\n", value);
                }
                return;
            }

            case decltype(RBANK)::index:
                if(value == 0)
                {
                    ramData[index] = value;
                    return;
                }
                break;

            regWhitelist()
                ramData[index] = value;
                return;

            default:
                printf("unknown register write at 0x%04x, PC: 0x%04x, value: 0x%02x\n", index, PC.get(), value);
            }
        }
        else if(index >= 0x100 && index <= 0x1FF) //BANK 1, General purpose area
        {
            ramData[index] = value;
            return;
        }
        else if(index >= 0x200 && index <= 0x27F) //BANK 2, General purpose area
        {
            ramData[index] = value;
            return;
        }
        printf("location: 0x%04x\n", index);
        throw std::out_of_range("invalid ram write");
    }

    bool flashRom(const std::vector<uint8_t> & data)
    {
        if(data.size() > sizeof(rom) || data.size() & 1)
            return false;
        for(size_t i = 0; i < rom.count; i++)
            rom.set(i, 0);
        for(size_t i = 0, j = 0; i < data.size(); i+= 2, j++)
            rom.set(j, data.at(i + 1) << 8 | data.at(i));
        //TODO: last 16 bytes of the ROM ???
        return true;
    }

    void reset(ResetType resetType = ResetPowerOn)
    {
        memset(ramData, 0, sizeof(ramData));
        ACC = 0;
        STKP.set(7);
        GIE.set(0);
        switch(resetType)
        {
            case ResetPowerOn:
                FNT0.set(1);
                FNPD.set(0);
                break;
            case ResetWatchdog:
                FNT0.set(0);
                FNPD.set(0);
                break;
            case ResetExternal:
                FNT0.set(1);
                FNPD.set(1);
                break;
        }
    }

    bool stackPush(uint16_t value)
    {
        auto stkp = STKP.get();
        switch(stkp)
        {
            case 0: //TODO: what happens with the stack pointer?
                STK7.set(value);
                break;
            case 1:
                STK6.set(value);
                break;
            case 2:
                STK5.set(value);
                break;
            case 3:
                STK4.set(value);
                break;
            case 4:
                STK3.set(value);
                break;
            case 5:
                STK2.set(value);
                break;
            case 6:
                STK1.set(value);
                break;
            case 7:
                STK0.set(value);
                break;
            default:
                //TODO: halt?
                return false;
        }
        STKP.set(stkp - 1);
        return true;
    }

    bool stackPop(uint16_t& value)
    {
        auto stkp = STKP.get();
        stkp++;
        if(stkp > 7) //overflow
            stkp = 0;
        switch(stkp)
        {
            case 0: //TODO: what happens with the stack pointer?
                value = STK7.get();
                break;
            case 1:
                value = STK6.get();
                break;
            case 2:
                value = STK5.get();
                break;
            case 3:
                value = STK4.get();
                break;
            case 4:
                value = STK3.get();
                break;
            case 5:
                value = STK2.get();
                break;
            case 6:
                value = STK1.get();
                break;
            case 7:
                value = STK0.get();
                break;
            default:
                //TODO: halt?
                return false;
        }
        STKP.set(stkp);
        return true;
    }

    bool step()
    {
        try
        {
            // Page 62 (whole chapter)
            if(GIE.get()) //interrupts are enabled and no interrupt is in progress
            {
                auto rq = intrq.get(), en = inten.get(), rq1 = intrq1.get(), en1 = inten1.get();
                if((rq & en) != 0 || (rq1 & en1) != 0)
                {
                    auto pc = PC.get();
                    printf("performing interrupt (PC: 0x%04x, INTRQ: 0x%02x, INTEN: 0x%02x, INTRQ1: 0x%02x, INTEN1: 0x%02x)\n",
                           pc,
                           rq,
                           en,
                           rq1,
                           en1);
                    GIE.set(0);
                    stackPush(pc);
                    PC.set(8);
                    return true;
                }
            }

            if(stepInternal())
            {
                auto stepTimer = [&](TimerCounter& tc, bool& irq)
                {
                    if(tc.step()) //timer overflow occurred
                        irq = true; //interrupt request bit is set regardless if the interrupt is enabled (Page 73)
                };
                stepTimer(tc0, intrq1.TC0IRQ);
                stepTimer(tc1, intrq1.TC1IRQ);
                stepTimer(tc2, intrq1.TC2IRQ);
                return true;
            }
            return false;
        }
        catch(std::exception & ex)
        {
            printf("exception during instruction step: %s\n", ex.what());
            return false;
        }
    }

    bool stepInternal()
    {
        /*struct Timer
        {
            unsigned long long time1;

            Timer()
            {
                unsigned int aux = 0;
                time1 = __rdtscp(&aux);
            }

            ~Timer()
            {
                unsigned int aux = 0;
                auto time2 = __rdtscp(&aux);
                printf("time: %llu\n", time2 - time1);
            }
        };*/
        //fetch instruction
        auto addr = *(uint16_t*)(ramData + 0xCE);
        auto instruction = rom.get(addr);
        //disassemble instruction
        auto bincode = uint8_t(instruction >> 8);
        uint8_t opcode_key = 0;
        bool is_bit = false;
        if(bincode >= 0x80)
        {
            opcode_key = uint8_t(bincode & 0xc0);
            is_bit = false;
        }
        else if(bincode >= 0x40)
        {
            opcode_key = uint8_t(bincode & 0xf8);
            is_bit = true;
        }
        else
        {
            opcode_key = bincode;
            is_bit = false;
        }

        //char opcode[256] = "";
        auto entry = entries[opcode_key];
        if(entry.mnemonic == nullptr)
        {
            printf("0x%04x: DW 0x%02x ; ILLEGAL OPCODE\n", addr, instruction);
            return false;
        }
        auto mask = entry.mask;
        auto opspace = entry.opSpace;
        //auto jump_action = entry.branchType;
        //auto mnemonic = entry.mnemonic;
        //auto caption = entry.operands;

        //strcat(opcode, mnemonic);

        uint16_t operand = 0;
        uint8_t bit_address = 0;

        if(opspace == NUL_SPACE)
        {
            //TODO: branch
        }
        else
        {
            //char operand_fmt[256] = "";
            operand = instruction & mask;
            //TODO: branch
            if(opspace == ROM_SPACE)
            {
                //strcpy(operand_fmt, "0x%04x");
                //assert not is_bit
            }
            else if(opspace == IMM_SPACE)
            {
                //strcpy(operand_fmt, "#0x%02x");
                //assert not is_bit
            }
            else // ZRO & RAM
            {
                //strcpy(operand_fmt, "0x%02x");
                if(is_bit)
                {
                    bit_address = uint8_t(bincode & 0x7);
                    //sprintf(operand_fmt + strlen(operand_fmt), ".%i", bit_address);
                }
            }

            //char symbol[256] = "";
            //sprintf(symbol, operand_fmt, operand);
            //strcat(opcode, " ");
            //sprintf(opcode + strlen(opcode), caption, symbol);
        }

        auto next = [&]()
        {
            PC.set(addr + 1);
            return true;
        };
        auto skip = [&]()
        {
            PC.set(addr + 2);
            return true;
        };
        auto trbank = [&](uint16_t index)
        {
            return uint16_t(index + 0x100 * RBANK.get());
        };

        switch(entry.type)
        {
        case INSTR_NUL_NOP:
        {
            return next();
        }

        case INSTR_ROM_JMP_OP:
        {
            PC.set(operand);
            return true;
        }

        case INSTR_ZRO_B0BCLR_OP:
        {
            ram.setBit(operand, bit_address, 0);
            return next();
        }

        case INSTR_ZRO_B0BSET_OP:
        {
            ram.setBit(operand, bit_address, 1);
            return next();
        }

        case INSTR_IMM_B0MOV_Y_OP:
        {
            Y.set(operand);
            return next();
        }

        case INSTR_IMM_B0MOV_Z_OP:
        {
            Z.set(operand);
            return next();
        }

        case INSTR_NUL_MOVC:
        {
            auto yz = YZ.get();
            auto data = rom.get(yz);
            ACC = data & 0xFF;
            R.set(data >> 8);
            return next();
        }

        case INSTR_IMM_CMPRS_A_OP:
        {
            auto result = int(ACC) - int(operand);
            auto zero = result == 0;
            FZ.set(zero);
            FC.set(result < 0);
            return zero ? skip() : next();
        }

        case INSTR_RAM_INCMS_OP:
        {
            auto result = uint8_t(ram.read(operand) + 1);
            ram.write(operand, result);
            return result == 0 ? skip() : next();
        }

        case INSTR_ZRO_B0MOV_A_OP:
        {
            auto data = ram.read(operand);
            ACC = data;
            FZ.set(data == 0);
            return next();
        }

        case INSTR_IMM_MOV_A_OP:
        {
            ACC = operand;
            return next();
        }

        case INSTR_IMM_B0MOV_R_OP:
        {
            R.set(operand);
            return next();
        }

        case INSTR_RAM_B0MOV_OP_A:
        {
            ram.write(operand, ACC);
            return next();
        }

        case INSTR_IMM_B0MOV_RBANK_OP:
        {
            RBANK.set(operand);
            return next();
        }

        case INSTR_RAM_CLR_OP:
        {
            ram.write(operand, 0);
            return next();
        }

        case INSTR_RAM_DECMS_OP:
        {
            auto data = ram.read(operand);
            data--;
            ram.write(operand, data);
            return data == 0 ? skip() : next();
        }

        case INSTR_ZRO_B0BTS0_OP:
        {
            auto data = ram.getBit(operand, bit_address);
            return data == 0 ? skip() : next();
        }

        case INSTR_RAM_BTS0_OP:
        {
            auto data = ram.getBit(operand, bit_address);
            return data == 0 ? skip() : next();
        }

        case INSTR_ZRO_B0BTS1_OP:
        {
            auto data = ram.getBit(operand, bit_address);
            return data == 1 ? skip() : next();
        }

        case INSTR_RAM_BTS1_OP:
        {
            auto data = ram.getBit(operand, bit_address);
            return data == 1 ? skip() : next();
        }

        case INSTR_RAM_MOV_A_OP:
        {
            auto data = ram.read(operand);
            if(data == 0) //TODO: unclear wording on page 10
                FZ.set(1);
            ACC = data;
            return next();
        }

        case INSTR_RAM_MOV_OP_A:
        {
            ram.write(operand, ACC);
            return next();
        }

        case INSTR_IMM_AND_A_OP:
        {
            auto data = ACC;
            data &= operand;
            FZ.set(data == 0);
            ACC = data;
            return next();
        }

        case INSTR_IMM_XOR_A_OP:
        {
            auto data = ACC;
            data ^= operand;
            FZ.set(data == 0);
            ACC = data;
            return next();
        }

        case INSTR_RAM_BCLR_OP:
        {
            ram.setBit(trbank(operand), bit_address, 0);
            return next();
        }

        case INSTR_RAM_BSET_OP:
        {
            ram.setBit(trbank(operand), bit_address, 1);
            return next();
        }

        case INSTR_ROM_CALL_OP:
        {
            if(!stackPush(addr + 1))
                return false;
            PC.set(operand);
            return true;
        }

        case INSTR_NUL_RET:
        {
            uint16_t newpc = 0;
            if(!stackPop(newpc))
                return false;
            PC.set(newpc);
            return true;
        }

        case INSTR_NUL_RETI:
        {
            uint16_t newpc = 0;
            if(!stackPop(newpc))
                return false;
            PC.set(newpc);
            GIE.set(1);
            return true;
        }

        case INSTR_NUL_PUSH: //Page 68
        {
            ACC_push = ACC;
            PFLAG_push = PFLAG.get();
            return next();
        }

        case INSTR_NUL_POP:
        {
            ACC = ACC_push;
            PFLAG.set(PFLAG_push);
            return next();
        }

        case INSTR_ZRO_B0ADD_OP_A:
        {
            auto data = ram.read(operand);
            auto result = data + ACC;
            FZ.set(result == 0);
            FC.set(result > 255);
            FDC.set((data & 0xf) + (ACC & 0xf) > 15); //https://stackoverflow.com/a/4513781
            ram.write(operand, uint8_t(data & 0xff));
            return next();
        }

        case INSTR_RAM_RLCM:
        {
            auto data = ram.read(trbank(operand));
            auto result = uint8_t(data << 1);
            result |= FC.get();
            FC.set(data >> 7);
            ram.write(trbank(operand), result);
            return next();
        }

        default:
            break;
        }

        printf("0x%04x: instruction not implemented: %s //%s\n", addr, instrTypeToString(entry.type), disassemble(addr, instruction).c_str());
        return false;
    }
};
