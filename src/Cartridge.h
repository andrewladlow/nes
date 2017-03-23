#ifndef CARTRIDGE_H_
#define CARTRIDGE_H_

#include <iostream>
#include <bitset>

class Cartridge {
public:
    Cartridge(std::string filePath);
    ~Cartridge();

    void debug();

    char *getprgRom();
    int getprgRomBanks();
    char *getchrRom();
    char *getram();
private:
    char header[16];

    char *prgRom;
    char *chrRom;
    char *ram;

    int prgRomSize;
    int chrRomSize;
    int ramSize;
};

#endif
