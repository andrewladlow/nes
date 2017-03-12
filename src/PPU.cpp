#include <iostream>
#include "PPU.h"

using namespace std;

PPU::PPU() {
    cout << "New PPU created" << endl;

    ppuCtrl = 0;
    ppuMask = 0;
    ppuStatus = 0;
    oamAddr = 0;
    oamData = 0;
    ppuScroll = 0;
    ppuAddr = 0;
    ppuData = 0;

    cntFV = 0;
    cntV = 0;
    cntH = 0;
    cntVT = 0;
    cntHT = 0;

    written = false;

	for (int i = 0; i < 4096; i++) {
		vRam[i] = 0;
	}

	for (int i = 0; i < 256; i++) {
		sprRam[i] = 0;
	}
}

PPU::~PPU() {

}

uint8_t PPU::readMemory(uint16_t address) {
	if (address >= 0x4000) {
		// locations 4000 - 10000 are mirrors of 0000 - 3fff
		address &= 0x3FFF;
	}

	if (address >= 0x3F20) {
		// locations 3f20 - 4000 are mirrors of 3f00 - 3f1f
		address &= 0x3F1F;
	}

	if (address >= 0x3F00) {
		// every 4th location is a mirror of 3f00
		if (!(address & 0x03)) {
			address &= 0x3F00;
		}
	}

	if (address >= 0x3000) {
		address &= 0x2EFF;
	}

	if (address >= 0x2000) {
		// use vertical mirroring for initial testing (mario)
		address &= 0x27FF;
	}

	return vRam[address];
}

void PPU::renderScanlines() {
	// for each scanline
	for (int i = 0; i < 261; i++) {
		if (i <= 19) {
			// ?
		} else if (i == 20) {
			// update scroll counters
		} else if (i <= 260) {
			// render
		} else {
			setVBlank(1);
		}
	}
}

bool PPU::getVBlank() {
	return ppuStatus & 0x80;
}

void PPU::setVBlank(bool value) {
	if (value) {
		ppuStatus |= 0x80;
	} else {
		ppuStatus -= 0x80;
	}
}


void PPU::setPPUCtrl(uint8_t value) { // 2000
	//ppuCtrl = value;
	regV = value & 0x02;
	regH = value & 0x01;
	regS = value & 0x10;
}

void PPU::setPPUMask(uint8_t value) { // 2001
	ppuMask = value;
}

uint8_t PPU::getPPUStatus() { // 2002
	setVBlank(0);
	written = false; // reset PPUSCROLL / PPUADDR latch
	return ppuStatus;
}

void PPU::setPPUStatus(uint8_t value) { // 2002
	ppuStatus = value;
}

void PPU::setOAMAddr(uint8_t value) { // 2003
	oamAddr = value;
}

uint8_t PPU::getOAMData() { // 2004
	return oamData;
}

void PPU::setOAMData(uint8_t value) { // 2004
	//oamData = value;
	sprRam[oamAddr] = value;
	oamAddr++;
}

void PPU::setPPUScroll(uint8_t value) { // 2005
	//ppuScroll = value;

	if (!written) {
		regHT = value & 0xF8;
		regFH = value & 0x07;
	} else {
		regFV = value & 0x07;
		regVT = value & 0xF8;
	}

	written = !written;
}

void PPU::setPPUAddr(uint8_t value) { // 2006
	!ppuAddr = value;

	if (!written) {
		regFV = value & 0x30;
		regFV &= 0xBF;
		regV = value & 0x0F;
		regH = value & 0x04;
		regVT = value & 0x03;
	} else {
		regVT = value & 0xE0;
		regHT = value & 0x1F;

		updateCounters();
	}

	written = !written;
}

uint8_t PPU::getPPUData() { // 2007
	return ppuData;
}

void PPU::setPPUData(uint8_t value) { // 2007
	ppuData = value;
}

// load counters with values of their latches
void PPU::updateCounters() {
	cntFV = regFV;
	cntV = regV;
	cntH = regH;
	cntVT = regVT;
	cntHT = regHT;
}
