#include <iostream>
#include "PPU.h"

using namespace std;

PPU::PPU() {
    cout << "New PPU created" << endl;

    vRamBuffer = NULL;

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

    vramInc32 = false;
    sprTable = false;
    sprSize = false;

    sprOverflow = false;
    spr0Hit = false;

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
			updateScrollCounters();
			// TODO increment scroll counters during render
		} else if (i <= 260) {
			// render
		} else {
			vBlank = 1;
		}
	}
}

bool PPU::getvBlank() {
	return vBlank;
}

void PPU::setppuCtrl(uint8_t value) { // 2000
	//ppuCtrl = value;
	regH = value & 0x01;
	regV = value & 0x02;
	vramInc32 = value & 0x04;
	sprTable = value & 0x08;
	regS = value & 0x10;
	sprSize = value & 0x20;
	vBlank = value & 0x80;
}

void PPU::setppuMask(uint8_t value) { // 2001
	//ppuMask = value;
	greyscale = value & 0x01;
	showBGLeft = value & 0x02;
	showSprLeft = value & 0x04;
	showBG = value & 0x08;
	showSpr = value & 0x10;
	emphR = value & 0x20;
	emphG = value & 0x40;
	emphB = value & 0x80;
}

uint8_t PPU::getppuStatus() { // 2002
	uint8_t status = 0;
	// TODO not concerned with lower 5 bits here?
	status &= sprOverflow << 5;
	status &= spr0Hit << 6;
	status &= vBlank << 7;

	vBlank = 0;
	written = false; // reset PPUSCROLL / PPUADDR latch
	return ppuStatus;
}


void PPU::setoamAddr(uint8_t value) { // 2003
	oamAddr = value;
	// TODO consider value during rendering
}

uint8_t PPU::getoamData() { // 2004
	return sprRam[oamAddr];
}

void PPU::setoamData(uint8_t value) { // 2004
	//oamData = value;
	sprRam[oamAddr] = value;
	oamAddr++;
}

void PPU::setppuScroll(uint8_t value) { // 2005
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

void PPU::setppuAddr(uint8_t value) { // 2006
	// ppuAddr = value;

	if (!written) {
		regFV = value & 0x30;
		//regFV &= 0xBF; not required?
		regV = value & 0x08;
		regH = value & 0x04;
		regVT = value & 0x03;
	} else {
		regVT = value & 0xE0;
		regHT = value & 0x1F;

		updateScrollCounters();
	}

	written = !written;
}

uint8_t PPU::getppuData() { // 2007
	//return ppuData;
	uint16_t vRamAddr = getVramAddr();
	uint8_t result = 0;
	//
	if (vRamAddr <= 0x3EFF && vRamBuffer != NULL) {
		result = vRamBuffer;
	} else {
		result = vRam[vRamAddr];
	}

	vRamBuffer = vRam[vRamAddr];
	incVramAddr(vRamAddr);
	return result;
}

void PPU::setppuData(uint8_t value) { // 2007
	//ppuData = value;
	uint16_t vRamAddr = getVramAddr();
	vRam[vRamAddr] = value;
	incVramAddr(vRamAddr);
}

// load counters with values of their latches
void PPU::updateScrollCounters() {
	cntFV = regFV;
	cntV = regV;
	cntH = regH;
	cntVT = regVT;
	cntHT = regHT;
}

// Retrieve vram address from daisy-chained counters
uint16_t PPU::getVramAddr() {
	uint16_t result = 0;
	result = cntHT;
	result &= cntVT << 5;
	result &= cntH << 10;
	result &= cntV << 11;
	result &= cntFV << 12;
	return result;
}

// Increment address supplied by getVramAddr() above
void PPU::incVramAddr(uint16_t value) {
	uint8_t updateAmount = 0;
	uint16_t vramAddr = value;
	if (vramInc32) {
		updateAmount = 32;
	} else {
		updateAmount = 1;
	}

	vramAddr += updateAmount & 0x7FFF; // all 5 counters total 15 bits rather than 16
	updateVramAddr(vramAddr);
}

// Update counters with new address after it has been incremented
void PPU::updateVramAddr(uint16_t value) {
	cntFV = (value >> 12) & 0x03;
	cntV = (value >> 11) & 0x01;
	cntH = (value >> 10) & 0x01;
	cntVT = (value >> 5) & 0x1F;
	cntHT = value & 0x1F;
}

