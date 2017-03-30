
#ifndef PPU_H_
#define PPU_H_
#include <iostream>
#include <array>
#include <SFML/Graphics/Color.hpp>

using namespace std;

template <typename T, size_t M, size_t N> using array2d = array<array<T, N>, M>;

typedef struct colour {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};



class PPU {
public:
    PPU(char *chrRom);
    ~PPU();

    array2d<colour, 256, 240> getpixelBuffer();
    uint8_t getppuCtrl();
    void setppuCtrl(uint8_t value);
    uint8_t getppuMask();
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
    bool isNMI();

    void renderScanline(int scanline);

    void updateScrollCounters();

    uint16_t getVramAddr();
    void incVramAddr();
    void updateVramAddr(uint16_t value);

    void cycle(int scanline);

    void incrementVerticalScrollCounters();

    void incrementHorizontalScrollCounters();
    void initPalette();

    void setppuStatus(int bit, bool val);
    void setvBlank(bool val);

    uint8_t readMemory(uint16_t address);
    void storeMemory(uint16_t address, uint8_t word);
    uint16_t resolveAddress(uint16_t address);

private:

    array<colour, 64> palette;
    array2d<colour, 256, 240> pixelBuffer;

    char vRam[0x4000];
    char sprRam[0x100];
    char *chrRom;

    uint8_t vRamBuffer;

    uint8_t ppuStatus;
    uint8_t oamAddr;
    uint8_t oamData;

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
    bool generateNMI;

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
};

#endif
