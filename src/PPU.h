#ifndef PPU_H
#define PPU_H


class PPU
{
public:
    PPU();
    ~PPU();

    void setPPUCtrl(uint8_t value);

    void setPPUMask(uint8_t value);

    uint8_t getPPUStatus();
    void setPPUStatus(uint8_t value);

    void setOAMAddr(uint8_t value);

    uint8_t getOAMData();
    void setOAMData(uint8_t value);

    void setPPUScroll(uint8_t value);

    void setPPUAddr(uint8_t value);

    uint8_t getPPUData();
    void setPPUData(uint8_t value);

    bool getVBlank();
    void setVBlank(bool value);

    void renderScanlines();

private:
    char vRam[4096];
    char sprRam[256];

    uint8_t ppuCtrl;
    uint8_t ppuMask;
    uint8_t ppuStatus;
    uint8_t oamAddr;
    uint8_t oamData;
    uint8_t ppuScroll;
    uint8_t ppuAddr;
    uint8_t ppuData;

    uint8_t regFV;
    uint8_t regFH;
    uint8_t regVT;
    uint8_t regHT;
    uint8_t regV;
    uint8_t regH;
    uint8_t regS;

    uint8_t cntFV;
    uint8_t cntV;
    uint8_t cntH;
    uint8_t cntVT;
    uint8_t cntHT;


    // determine first or second write for PPUSCROLL and PPUADDR
    bool written;

    uint8_t readMemory(uint16_t address);
};

#endif // PPU_H
