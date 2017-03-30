#include "PPU.h"

using namespace std;

PPU::PPU(char *chrRom) {
	this->chrRom = chrRom;
    //cout << "New PPU created" << endl;

    oamAddr = 0;
    oamData = 0;

    vramInc32 = false;
    sprTable = false;
    sprSize = false;
    generateNMI = false;

    sprOverflow = false;
    spr0Hit = false;

    written = false;

	for (int i = 0; i < 0x4000; i++) {
		vRam[i] = 0;
	}

	memcpy(vRam, chrRom, 0x2000);

	for (int i = 0; i < 0x100; i++) {
		sprRam[i] = 0;
	}

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 240; j++) {
			pixelBuffer[i][j] = {0,0,0};
		}
	}

	initPalette();
}

PPU::~PPU() {

}

array2d<colour, 256, 240> PPU::getpixelBuffer() {
	return pixelBuffer;
}


uint8_t PPU::readMemory(uint16_t address) {
    return vRam[resolveAddress(address)];
}

void PPU::storeMemory(uint16_t address, uint8_t word) {
    vRam[resolveAddress(address)] = word;
}

uint16_t PPU::resolveAddress(uint16_t address) {
    // all data >= 0x4000 is a mirror of 0x0 - 0x3FFF
    address &= 0x3FFF;

    if (address < 0x2000) {
        return address;
    } else if (address < 0x3000) {
        return address & 0x23FF;
    } else if (address < 0x3F00) {
        // mirror of above
        return address & 0x23FF;
    } else {
    	// locations 3f20 - 4000 are mirrors of 3f00 - 3f1f
        address &= 0x3F1F;

        // every 4th location is a mirror
        if ((address & 0x03) == 0) {
            address &= 0x3F0F;
        }
        return address;
    }
}


void PPU::cycle(int currentScanline) {
	cout << "showBG: " << showBG << endl;
	cout << "Scanline: " << dec << currentScanline << endl;
	if (currentScanline == 0) {
		spr0Hit = 0;
	} else if (currentScanline == 20) {
		if (showBG || showSpr) {
			// TODO according to scrolling skinny this is correct?
			// this is also suggested by http://forums.nesdev.com/viewtopic.php?t=664 "at frame start" loopyV = loopyT
			updateScrollCounters();
			//vBlank = 0;
		}
	} else if (currentScanline >= 21 && currentScanline <= 260) {
		if (showBG || showSpr) {
			cout << "debug: render" << endl;
			int pixelX = 0;
			int pixelY = currentScanline;
			while (pixelX < 256) {
				uint16_t nameTableAddr = 0x2000 | (getVramAddr() & 0x0FFF);
				uint8_t tile = readMemory(nameTableAddr);
				uint16_t attributeTableAddr = 0x23C0 | (getVramAddr() & 0x0C00) | ((getVramAddr() >> 4) & 0x38) | ((getVramAddr() >> 2) & 0x07);

				uint8_t attributeTableByte = readMemory(attributeTableAddr);
				uint8_t quadrant = (cntVT & 0x02) << 1 | (cntHT & 0x02);
				uint8_t upperPixelIndex = (attributeTableByte >> quadrant) & 0x03;

				uint16_t patternTableAddr = (uint16_t) regS << 12;
				patternTableAddr |= (uint16_t)tile << 4;
				patternTableAddr |= cntFV;

				uint8_t lowerPatternByte = readMemory(patternTableAddr);
				uint8_t upperPatternByte = readMemory(patternTableAddr + 8);

				int x = (pixelX + regFH) % 8;
				int patternBit = 7 - x;

				uint8_t lowerPatternBit = (lowerPatternByte >> patternBit) & 1;
				uint8_t upperPatternBit = ((upperPatternByte >> patternBit) & 1);

				uint16_t paletteIndex = 0x3F00;

				uint8_t lowerPixelIndex = (uint8_t) (lowerPatternBit | (upperPatternBit << 1));

				uint8_t pixelIndex = lowerPixelIndex | (upperPixelIndex << 2);;
				paletteIndex += pixelIndex;

				pixelBuffer[pixelX][pixelY] = palette[readMemory(paletteIndex)];

				// check for new tile
				// "The HT counter is then clocked every 8 pixel dot clocks"
				pixelX++;
				if ((pixelX + regFH) % 8 == 0) {
					incrementHorizontalScrollCounters();
				}
			}
		}

		incrementVerticalScrollCounters();

		// after a scanline is rendered, reset X scroll, inc Y
		// H and HT are updated at hblank whilst rendering is active
		// reload loopyT into loopyV
		cntH = regH;
		cntHT = regHT;

	} else if (currentScanline == 261) {
		vBlank = 1;
	}
}

//http://wiki.nesdev.com/w/index.php/PPU_scrolling#Wrapping_around
void PPU::incrementVerticalScrollCounters() {
	cntFV++;
	if (cntFV == 8) {
		cntFV = 0;
		cntVT++;
		// coarse Y cut off after 29
		if (cntVT == 30) {
			cntVT = 0;
			// switch vertical nametable
			cntV = !cntV;
		} else if (cntVT == 31) {
			cntVT = 0;
			// if coarse Y set out of bounds, wrap to 0 but dont switch vertical nametable
		}
	}
}

//http://wiki.nesdev.com/w/index.php/PPU_scrolling#Wrapping_around
void PPU::incrementHorizontalScrollCounters() {
	cntHT++;
	if (cntHT == 32) {
		cntHT = 0;
		// switch horizontal nametable
		cntH = !cntH;
	}

}

bool PPU::isNMI() {
	return generateNMI && vBlank;
}

uint8_t PPU::getppuCtrl() { // 2000
	uint8_t temp = 0;
	temp |= (regH);
	temp |= (regV << 1);
	temp |= (vramInc32 << 2);
	temp |= (sprTable << 3);
	temp |= (regS << 4);
	temp |= (sprSize << 5);
	temp |= (generateNMI << 7);
	return temp;
}

void PPU::setppuCtrl(uint8_t value) { // 2000
	regH = value & 0x01;
	regV = (value & 0x02) >> 1;
	vramInc32 = (value & 0x04) >> 2;
	sprTable = (value & 0x08) >> 3;
	regS = (value & 0x10) >> 4;
	sprSize = (value & 0x20) >> 5;
	generateNMI = (value & 0x80) >> 7;
}

uint8_t PPU::getppuMask() { // 2001
	uint8_t temp = 0;
	temp |= (greyscale);
	temp |= (showBGLeft << 1);
	temp |= (showSprLeft << 2);
	temp |= (showBG << 3);
	temp |= (showSpr << 4);
	temp |= (emphR << 5);
	temp |= (emphG << 6);
	temp |= (emphB << 7);
	return temp;
}

void PPU::setppuMask(uint8_t value) { // 2001
	//ppuMask = value;
	greyscale = value & 0x01;
	showBGLeft =  (value & 0x02) >> 1;
	showSprLeft = (value & 0x04) >> 2;
	showBG = (value & 0x08) >> 3;
	showSpr = (value & 0x10) >> 4;
	emphR = (value & 0x20) >> 5;
	emphG = (value & 0x40) >> 6;
	emphB = (value & 0x80) >> 7;
}

uint8_t PPU::getppuStatus() { // 2002
	// not concerned with lower 5 bits here?
	ppuStatus = 0;
	ppuStatus |= (sprOverflow << 5);
	ppuStatus |= (spr0Hit << 6);
	ppuStatus |= (vBlank << 7);

	vBlank = 0;
	written = false; // reset PPUSCROLL / PPUADDR latch
	return ppuStatus;
}

void PPU::setoamAddr(uint8_t value) { // 2003
	oamAddr = value;
	// TODO consider value during rendering
}

uint8_t PPU::getoamData() { // 2004
	return sprRam[oamAddr++];
}

void PPU::setoamData(uint8_t value) { // 2004
	//oamData = value;
	sprRam[oamAddr++] = value;
}

void PPU::setppuScroll(uint8_t value) { // 2005
	if (!written) {
		regHT = (value & 0xF8) >> 3;
		regFH = value & 0x07;
	} else {
		regFV = value & 0x07;
		regVT = (value & 0xF8) >> 3;
	}

	written = !written;
}

void PPU::setppuAddr(uint8_t value) { // 2006
    if (!written) {
        regFV = (value & 0x30) >> 4;
        regV = (value & 0x08) >> 3;
        regH = (value & 0x04) >> 2;
        // get upper 2 bits from write here
        regVT = ((value & 0x03) << 3) | (regVT & 0x07);
    } else {
    	// then lower 3 bits for full 5 bit value
        regVT = ((value & 0xE0) >> 5) | (regVT & 0x18);
        regHT = (value & 0x1F);

        updateScrollCounters();
    }

    written = !written;
}

uint8_t PPU::getppuData() { // 2007
	uint8_t result = 0;

	if (getVramAddr() <= 0x3EFF) {
		result = vRamBuffer;
	} else {
		result = vRam[getVramAddr()];
	}

	vRamBuffer = readMemory(getVramAddr());
	incVramAddr();
	return result;


}

void PPU::setppuData(uint8_t value) { // 2007
	storeMemory(getVramAddr(), value);
	incVramAddr();
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
	result |= (uint16_t) cntHT;
	result |= ((uint16_t) cntVT) << 5;
	result |= ((uint16_t) cntH) << 10;
	result |= ((uint16_t) cntV) << 11;
	result |= ((uint16_t) cntFV) << 12;
	return result;
}

void PPU::incVramAddr() {
	uint8_t updateAmount = 0;
	uint16_t vRamAddr = getVramAddr();
	if (vramInc32) {
		updateAmount = 32;
	} else {
		updateAmount = 1;
	}
	vRamAddr = (vRamAddr + updateAmount) & 0x7FFF; // all 5 counters total 15 bits rather than 16
	updateVramAddr(vRamAddr);
}

// Update counters with new address after it has been incremented
void PPU::updateVramAddr(uint16_t value) {
	cntFV = (value >> 12) & 0x03;
	cntV = (value >> 11) & 0x01;
	cntH = (value >> 10) & 0x01;
	cntVT = (value >> 5) & 0x1F;
	cntHT = value & 0x1F;
}

void PPU::initPalette() {
	palette = {{
		{124,124,124}, {0,0,252}, {0,0,188}, {68,40,188}, {148,0,132}, {168,0,32}, {168,16,0}, {136,20,0},
		{80,48,0}, {0,120,0}, {0,104,0},{0,88,0},{0,64,88},{0,0,0},{0,0,0},{0,0,0},
		{188,188,188},{0,120,248},{0,88,248},{104,68,252},{216,0,204},{228,0,88},{248,56,0},{228,92,16},
		{172,124,0},{0,184,0},{0,168,0},{0,168,68},{0,136,136},{0,0,0},{0,0,0},{0,0,0},
		{248,248,248},{60,188,252},{104,136,252},{152,120,248},{248,120,248},{248,88,152},{248,120,88},{252,160,68},
		{248,184,0},{184,248,24},{88,216,84},{88,248,152},{0,232,216},{120,120,120},{0,0,0},{0,0,0},
		{252,252,252},{164,228,252},{184,184,248},{216,184,248},{248,184,248},{248,164,192},{240,208,176},{252,224,168},
		{248,216,120},{216,248,120},{184,248,184},{184,248,216},{0,252,252},{248,216,248},{0,0,0},{0,0,0}
	}};
}

bool PPU::getvBlank() {
	return vBlank;
}

void PPU::setvBlank(bool val) {
	vBlank = val;
}
