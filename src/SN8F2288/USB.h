#pragma once

struct UsbData
{
    BaseArray<uint8_t, 136> & fifo;
    Register<0xCE, 0, 14> & PC;

    UsbData(BaseArray<uint8_t, 136> & fifo, Register<0xCE, 0, 14> & PC) : fifo(fifo), PC(PC) { }

    bool UDE = false;
    uint8_t UDA = 0;
    bool CRCERR = false;
    bool PKTERR = false;
    bool SOF = false;
    bool BUS_RST = false;
    bool SUSPEND = false;
    bool EP0SETUP = false;
    bool EP0IN = false;
    bool EP0OUT = false;

    bool EP_ACK[4] = { false, false, false, false };
    bool EP_NAK[4] = { false, false, false, false };

    bool REG_EN = true;
    bool DP_UP_EN = false;
    bool SOF_INT_EN = false;
    bool EP_NAK_INT_EN[4] = { false, false, false, false };

    enum EndpointMode
    {
        MODE_NAK = 0,
        MODE_ACK,
        MODE_STALL1,
        MODE_STALL2
    };

    EndpointMode UE0M = MODE_NAK;
    uint8_t UE0C = 0;
    bool UE1E = false;
    EndpointMode UE1M = MODE_NAK;
    uint8_t UE1C = 0;
    bool UE2E = false;
    EndpointMode UE2M = MODE_NAK;
    uint8_t UE2C = 0;

    uint8_t UDP0 = 0;

    uint8_t EP0OUT_CNT = 0;
    uint8_t EP2FIFO_ADDR = 0;

    bool UBDE = false;
    bool DDP = false;
    bool DDN = false;


    void setUda(uint8_t uda)
    {
        UDE = (uda >> 7) != 0;
        UDA = uint8_t(uda & 0x7f);
        printf("USB: set UDA to 0x%02x\n", uda);
    }

    uint8_t getUda() const
    {
        printf("USB: get UDA\n");
        return uint8_t((UDE << 7) | (UDA & 0x7f));
    }

    void setUstatus(uint8_t ustatus)
    {
        CRCERR = GETBIT(ustatus, 7);
        PKTERR = GETBIT(ustatus, 6);
        SOF = GETBIT(ustatus, 5);
        BUS_RST = GETBIT(ustatus, 4);
        SUSPEND = GETBIT(ustatus, 3);
        EP0SETUP = GETBIT(ustatus, 2);
        EP0IN = GETBIT(ustatus, 1);
        EP0OUT = GETBIT(ustatus, 0);
        printf("USB: set USTATUS to 0x%02x\n", ustatus);
    }

    uint8_t getUstatus() //const
    {
        //printf("USB: get USTATUS\n");
        //puts("USB: sleeping 200ms...");
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        //SUSPEND = !SUSPEND; //TODO: figure out how this actually works
        /*static int count = 0;
        if(count++ > 3)
        {
            SUSPEND = !SUSPEND;
            count = 0;
            throw exception();
        }*/
        return uint8_t((CRCERR << 7) | (PKTERR << 6) | (SOF << 5) | (BUS_RST << 4) | (SUSPEND << 3) | (EP0SETUP << 2) | (EP0IN << 1) | (EP0OUT << 0));
    }

    void setEp_ack(uint8_t ep_ack)
    {
        EP_ACK[0] = GETBIT(ep_ack, 0);
        EP_ACK[1] = GETBIT(ep_ack, 1);
        EP_ACK[2] = GETBIT(ep_ack, 2);
        EP_ACK[3] = GETBIT(ep_ack, 3);
        printf("USB: set EP_ACK to 0x%02x\n", ep_ack);
    }

    uint8_t getEp_ack() const
    {
        printf("USB: get EP_ACK\n");
        return uint8_t((EP_ACK[0] << 0) | (EP_ACK[1] << 1) | (EP_ACK[2] << 2) | (EP_ACK[3] << 3));
    }

    void setUe0r(uint8_t ue0r)
    {
        printf("USB: set UE0R to 0x%02x\n", ue0r);
        UE0M = EndpointMode((ue0r >> 5) & 0x3);
        UE0C = uint8_t(ue0r & 0xf);
    }

    uint8_t getUe0r() const
    {
        printf("USB: get UE0R\n");
        return uint8_t((UE0M << 5) | (UE0C << 0));
    }

    void setUe1r(uint8_t ue1r)
    {
        printf("USB: set UE1R to 0x%02x\n", ue1r);
        UE1E = GETBIT(ue1r, 7);
        UE1M = EndpointMode((ue1r >> 5) & 0x3);
    }

    uint8_t getUe1r() const
    {
        printf("USB: get UE1R\n");
        return uint8_t((UE1E << 7) | (UE1M << 5));
    }

    void setUe2r(uint8_t ue2r)
    {
        printf("USB: set UE2R to 0x%02x\n", ue2r);
        UE2E = GETBIT(ue2r, 7);
        UE2M = EndpointMode((ue2r >> 5) & 0x3);
    }

    uint8_t getUe2r() const
    {
        printf("USB: get UE2R\n");
        return uint8_t((UE2E << 7) | (UE2M << 5));
    }

    void setUdp0(uint8_t udp0)
    {
        printf("USB: set UDP0 to 0x%02x\n", udp0);
        UDP0 = udp0;
    }

    uint8_t getUdp0() const
    {
        printf("USB: get UDP0\n");
        return UDP0;
    }

    void setUdr0_r(uint8_t udr0_r)
    {
        printf("USB: set UDR0_R to 0x%02x (UDP0: 0x%02x, is this a NOOP?)\n", udr0_r, UDP0);
    }

    uint8_t getUdr0_r() const
    {
        printf("USB: get UDR0_R (UDP0: 0x%02x)\n", UDP0);
        if(UDP0 > 136)
            printf("USB: invalid USB FIFO access at 0x%02x\n", UDP0);
        return fifo.get(UDP0);
    }

    void setUdr0_w(uint8_t udr0_w)
    {
        printf("USB: set UDR0_W to 0x%02x (UDP0: 0x%02x)\n", udr0_w, UDP0);
    }

    uint8_t getUdr0_w() const
    {
        printf("USB: get UDR0_W (UDP0: 0x%02x, is this a NOOP?)\n", UDP0);
        return 0;
    }

    void setEp0out_cnt(uint8_t ep0out_cnt)
    {
        printf("USB: set EP0OUT_CNT to 0x%02x\n", ep0out_cnt);
        EP0OUT_CNT = uint8_t(ep0out_cnt & 0x1f);
    }

    uint8_t getEp0out_cnt() const
    {
        printf("USB: get EP0OUT_CNT\n");
        return EP0OUT_CNT;
    }

    void setUe1r_c(uint8_t ue1r_c)
    {
        printf("USB: set UE1R_C to 0x%02x\n", ue1r_c);
        UE1C = uint8_t(ue1r_c & 0x7f);
    }

    uint8_t getUe1r_c() const
    {
        printf("USB: get UE1R_C\n");
        return UE1C;
    }

    void setUe2r_c(uint8_t ue2r_c)
    {
        printf("USB: set UE2R_C to 0x%02x\n", ue2r_c);
        UE2C = uint8_t(ue2r_c & 0x7f);
    }

    uint8_t getUe2r_c() const
    {
        printf("gUSB: et UE2R_C\n");
        return UE1C;
    }

    void setEp2fifo_addr(uint8_t ep2fifo_addr)
    {
        printf("USB: set EP2FIFO_ADDR to 0x%02x\n", ep2fifo_addr);
        EP2FIFO_ADDR = ep2fifo_addr;
    }

    uint8_t getEp2fifo_addr() const
    {
        printf("USB: get EP2FIFO_ADDR\n");
        return EP2FIFO_ADDR;
    }

    void setUpid(uint8_t upid)
    {
        printf("USB: set UPID to 0x%02x\n", upid);
        UBDE = GETBIT(upid, 2);
        DDP = GETBIT(upid, 1);
        DDN = GETBIT(upid, 0);
    }

    uint8_t getUpid() const
    {
        printf("USB: get UPID\n");
        return uint8_t((UBDE << 2) | (DDP << 1) | (DDN << 0));
    }

    void setUsb_int_en(uint8_t usb_int_en)
    {
        printf("USB: set USB_INT_EN to 0x%02x\n", usb_int_en);
        REG_EN = GETBIT(usb_int_en, 7);
        DP_UP_EN = GETBIT(usb_int_en, 6);
        SOF_INT_EN = GETBIT(usb_int_en, 5);
        EP_NAK_INT_EN[3] = GETBIT(usb_int_en, 3);
        EP_NAK_INT_EN[2] = GETBIT(usb_int_en, 2);
        EP_NAK_INT_EN[1] = GETBIT(usb_int_en, 1);
        EP_NAK_INT_EN[0] = GETBIT(usb_int_en, 0);
    }

    uint8_t getUsb_int_en() const
    {
        printf("USB: get USB_INT_EN\n");
        return uint8_t((REG_EN << 7) | (DP_UP_EN << 6) | (SOF_INT_EN << 5) | (EP_NAK_INT_EN[3] << 3) | (EP_NAK_INT_EN[2] << 2) | (EP_NAK_INT_EN[1] << 1) | (EP_NAK_INT_EN[0] << 0));
    }

    void debug() const
    {
        printf("UDE: %d, UDA: 0x%02x\n", UDE, UDA);
        printf("CRCERR: %d, PKTERR: %d, SOF: %d, BUS_RST: %d\n", CRCERR, PKTERR, SOF, BUS_RST);
        printf("SUSPEND: %d, EP0SETUP: %d, EP0IN: %d, EP0OUT: %d\n", SUSPEND, EP0SETUP, EP0IN, EP0OUT);
    }
};
