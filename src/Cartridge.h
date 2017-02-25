#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <iostream>
#include <bitset>

class Cartridge {
public:
    Cartridge(std::string filePath);
    ~Cartridge();

    void debug();

    char *getPrgRom();
    char *getChrRom();
    char *getRam();
private:
    char header[16];

    char *prgRom;
    char *chrRom;
    char *ram;

    int prgRomSize;
    int chrRomSize;
    int ramSize;
};

#endif // CARTRIDGE_H
