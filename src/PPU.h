#ifndef PPU_H_
#define PPU_H_

#include <iostream>

class PPU {
public:
    PPU();
    ~PPU();

    void setppuCtrl(uint8_t value);

    void setppuMask(uint8_t value);

    uint8_t getppuStatus();
    void setppuStatus(uint8_t value);

    void setoamAddr(uint8_t value);

    uint8_t getoamData();
    void setoamData(uint8_t value);

    void setppuScroll(uint8_t value);

    void setppuAddr(uint8_t value);

    uint8_t getppuData();
    void setppuData(uint8_t value);

    bool getvBlank();

    void renderScanlines();

    void updateScrollCounters();

    uint16_t getVramAddr();

    void incVramAddr(uint16_t value);
    void updateVramAddr(uint16_t value);

private:


    char vRam[4096];
    char sprRam[256];

    uint8_t vRamBuffer;

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

    bool vramInc32;
    bool sprTable;
    bool sprSize;

	bool greyscale;
	bool showBGLeft;
	bool showSprLeft;
	bool showBG;
	bool showSpr;
	bool emphR;
	bool emphG;
	bool emphB;

	bool sprOverflow;
	bool spr0Hit;

	bool vBlank;

    // determine first or second write for PPUSCROLL and PPUADDR
    bool written;

    uint8_t readMemory(uint16_t address);
};

#endif
