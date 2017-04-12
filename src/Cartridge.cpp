#include "Cartridge.h"

#include <fstream>
#include <vector>

using namespace std;

Cartridge::Cartridge(string filePath) {
    ifstream file(filePath, ios::binary);
    if (file.good()) {
        file.read(header, 16);
        string headerName = string() + header[0] + header[1] + header[2];
        if (headerName != "NES") {
            cout << "Invalid ROM loaded" << endl;
            file.close();
        } else {
        	// store rom control byte 1 to handle ppu mirroring
        	romControlByte1 = header[8];

            if (header[6] & 0x4) { // check trainer presence
                cout << "trainer present" << endl;
            }

            prgRomSize = header[4] * 16384; // 16kb
            chrRomSize = header[5] * 8192; // 8kb
            ramSize = header[8] * 8192; // 8kb

            // handle case where size of 0 indicates 1 8kb page
            if (!ramSize) {
                ramSize = 8192;
            }
            prgRom = new char[prgRomSize];
            file.read(prgRom, prgRomSize);

            chrRom = new char[chrRomSize];
            file.read(chrRom, chrRomSize);

            ram = new char[ramSize];
            file.read(ram, ramSize);

            file.close();
        }
    }
}

char *Cartridge::getprgRom() {
    return prgRom;
}

int Cartridge::getprgRomBanks() {
	return (int) header[4];
}

char *Cartridge::getchrRom() {
    return chrRom;
}

char *Cartridge::getram() {
    return ram;
}

uint8_t Cartridge::getromControlByte1() {
	return romControlByte1;
}

void Cartridge::debug() {
    cout << "Header name: " << header[0] << header[1] << header[2] << endl;
    cout << "File format: " << hex << (int) header[3] << dec << endl;
    cout << "Number of PRG-ROM banks: " << (int) header[4] << endl;
    cout << "Size of PRG-ROM: " << prgRomSize << "kb" << endl;
    cout << "Number of CHR-ROM banks: " << (int) header[5] << endl;
    cout << "Size of CHR-ROM: " << chrRomSize << "kb" << endl;
    cout << "ROM control byte 1: " << bitset<8>(header[6]) << endl;
    cout << "ROM control byte 2: " << bitset<8>(header[7]) << endl;
    // assume 1 page of ram if result is 0
    cout << "Number of RAM banks: " << (!header[8] ? 1 : header[8]) << endl;
    cout << "Size of RAM: " << ramSize << "kb" << endl;
    cout << "Reserved: ";
    for (int i = 9; i < 15; i++) {
        cout << (int) header[i];
    }
    cout << endl;
}

Cartridge::~Cartridge() {

}
