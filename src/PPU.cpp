#include "PPU.h"

using namespace std;

PPU::PPU() {
    //cout << "New PPU created" << endl;

    //this->display = display;

    vRamBuffer = NULL;

    ppuStatus = 0;
    oamAddr = 0;
    oamData = 0;

    cntFV = 0;
    cntV = 0;
    cntH = 0;
    cntVT = 0;
    cntHT = 0;

    vramInc32 = false;
    sprTable = false;
    sprSize = false;
    generateNMI = false;

    sprOverflow = false;
    spr0Hit = false;

    written = false;

    currentScanline = 0;
    currentCycle = 0;
    currentScanlineCycle = 0;

	for (int i = 0; i < 0x4000; i++) {
		vRam[i] = 0;
	}

	for (int i = 0; i < 0x100; i++) {
		sprRam[i] = 0;
	}

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 240; j++) {
			pixelBuffer[i][j] = sf::Color();
		}
	}

	initPalette();
}

PPU::~PPU() {

}

array2d<sf::Color, 256, 240> PPU::getpixelBuffer() {
	return pixelBuffer;
}

uint8_t PPU::readMemory(uint16_t address) {
	if (address >= 0x4000) {
		// locations 4000 - 10000 are mirrors of 0000 - 3fff
		address &= 0x3FFF;
	} else if (address >= 0x3F20) {
		// locations 3f20 - 4000 are mirrors of 3f00 - 3f1f
		address &= 0x3F1F;
	} else if (address >= 0x3F00) {
		// every 4th location is a mirror of 3f00
		if (!(address & 0x03)) {
			address &= 0x3F00;
		}
	} else if (address >= 0x3000) {
		address &= 0x2EFF;
	} else if (address >= 0x2000) {
		// use vertical mirroring for initial testing (mario)
		address &= 0x27FF;
		// horizontal
		//address &= 0x23FF;
	}

	return vRam[address];
}

void PPU::cycle() {
	cout << "Scanline: " << dec << currentScanline << endl;
	//if (vBlank)
	//cout << vBlank << endl;
	if (currentScanline == 0) {
		vBlank = 0;
		spr0Hit = 0;
		// TODO according to scrolling skinny this is correct?
		// this is also suggested by http://forums.nesdev.com/viewtopic.php?t=664 "at frame start" loopyV = loopyT
		updateScrollCounters();
	} else if (currentScanline == 20) {
		// TODO increment scroll counters during render
		updateScrollCounters();
	} else if (currentScanline >= 21 && currentScanline <= 260) {
		// one of the two must be active to enable PPU rendering
		//if (showBG || showSpr) {
			cout << "debug: render" << endl;
			// TODO consider custom screen width
			if (currentScanlineCycle >= 0 && currentScanlineCycle < 256) {

				uint8_t pixelX = currentScanlineCycle;
				// render scanlines range from 21 - 260
				uint8_t pixelY = currentScanline - 21;

				// TODO screen width
				while (pixelX < 256) {
					uint16_t vRamAddr = getVramAddr();
					cout << "vram: " << hex << +vRamAddr << endl;

					uint16_t nameTableAddr = 0x2000 | (vRamAddr & 0x0FFF);
					cout << "name table addr: " << hex << +nameTableAddr << endl;
					uint8_t tile = readMemory(nameTableAddr);
					cout << "tile: " << hex << +tile << endl;

					uint16_t attributeTableAddr = 0x23C0 | (vRamAddr & 0x0C00) | ((vRamAddr >> 4) & 0x38) | ((vRamAddr >> 2) & 0x07);

					uint16_t patternTableAddr = regS << 8;
					// each tile occupies 16 bytes (1 set of 8 bytes for each of the 2 lower colour bits), hence shift left 4
					// cntFV determining which row of the pattern table here?
					patternTableAddr |= (tile << 4) | (vRamAddr >> 12);

					// fetch lower colour bits from pattern table
					// according to qeed nes the lower bits are from table entries [0] and [7] rather than [8]?
					uint8_t lowerPixelIndex = ((readMemory(patternTableAddr) & 0x80) >> 7) | ((readMemory(patternTableAddr + 8) & 0x80) >> 6);

					// fetch upper colour bits from attribute table
					uint8_t attributeTableByte = readMemory(attributeTableAddr);
					// http://forums.nesdev.com/viewtopic.php?f=3&t=14795&start=15
					uint8_t quadrant = (cntVT & 0x02) << 1 | (cntHT & 0x02);

					uint8_t upperPixelIndex = (attributeTableByte >> quadrant) & 0x03;

					// combine lower and upper to form pixel index
					uint8_t pixelIndex = (upperPixelIndex << 2) | lowerPixelIndex;


					pixelIndex += 0x3F00;
					sf::Color pixelColour = palette[readMemory(pixelIndex)];

					// assign pixel colour to buffer
					pixelBuffer[pixelX][pixelY].r = pixelColour.r;
					pixelBuffer[pixelX][pixelY].g = pixelColour.g;
					pixelBuffer[pixelX][pixelY].b = pixelColour.b;
					pixelBuffer[pixelX][pixelY].a = 255;

					pixelX++;

					// check for new tile
					// "The HT counter is then clocked every 8 pixel dot clocks"
					if ((pixelX + regFH) % 8 == 0) {
					//if (pixelX % 8 == 0) {
						incrementHorizontalScrollCounters();
					}
				}
			}
		//}

		// after a scanline is rendered, reset X scroll, inc Y
		// H and HT are updated at hblank whilst rendering is active
		// reload loopyT into loopyV
		cntH = regH;
		cntHT = regHT;

		incrementVerticalScrollCounters();
	} else {
		//cout << "PPU:261" << endl;
		//cout << "generateNMI: " << generateNMI << endl;
		//vBlank = 1;
	}

	currentCycle++;
	currentScanlineCycle++;

	// 341 ppu cycles per scanline, 1 pixel per scanline
	if (currentCycle % 341 == 0) {
		currentScanline++;
		currentScanlineCycle = 0;

		// 261 scanlines per frame, checking bounds after above increment
		if (currentScanline % 262 == 0) {
			currentCycle = 0;
			currentScanline = 0;

			vBlank = 1;
		}
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
	//cout << "vBlank: (P) " << vBlank << endl;
	//cout << "generateNMI: " << generateNMI << endl;
	return (vBlank && generateNMI);
}

uint8_t PPU::getppuCtrl() { // 2000
	uint8_t temp = 0;
	temp |= regH;
	temp |= regV << 1;
	temp |= vramInc32 << 2;
	temp |= sprTable << 3;
	temp |= regS << 4;
	temp |= sprSize << 5;
	temp |= generateNMI << 7;
	return temp;
}

void PPU::setppuCtrl(uint8_t value) { // 2000
	//ppuCtrl = value;
	regH = value & 0x01;
	regV = value & 0x02;
	vramInc32 = value & 0x04;
	sprTable = value & 0x08;
	regS = value & 0x10;
	sprSize = value & 0x20;
	generateNMI = value & 0x80;
}

uint8_t PPU::getppuMask() { // 2001
	uint8_t temp = 0;
	temp |= greyscale;
	temp |= showBGLeft << 1;
	temp |= showSprLeft << 2;
	temp |= showBG << 3;
	temp |= showSpr << 4;
	temp |= emphR << 5;
	temp |= emphG << 6;
	temp |= emphB << 7;
	return temp;
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
	// TODO not concerned with lower 5 bits here?
	ppuStatus = 0;
	ppuStatus |= sprOverflow << 5;
	ppuStatus |= spr0Hit << 6;
	ppuStatus |= vBlank << 7;
//	setppuStatus(5, sprOverflow);
//	setppuStatus(6, spr0Hit);
//	setppuStatus(7, vBlank);

	//cout << "PPU Status: " << hex << +ppuStatus << endl;

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
		cout << "fv: " << hex << +regFV << endl;
		//regFV &= 0xBF; not required?
		regV = value & 0x08;
		cout << "v: " << hex << +regV << endl;
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
	result |= (uint16_t) cntHT;
	result |= ((uint16_t) cntVT) << 5;
	result |= ((uint16_t) cntH) << 10;
	result |= ((uint16_t) cntV) << 11;
	result |= ((uint16_t) cntFV) << 12;
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

void PPU::initPalette() {
	// std::array itself is a struct with 1 element being the actual array element? hence double braces
	// can also remove braces and have a single list representing all elements?
	// see https://social.msdn.microsoft.com/Forums/vstudio/en-US/e5ad8fa5-c9e8-4328-a7fa-af7a47ce2492/initialising-a-stdarray-of-structs?forum=vclanguage
	// palette = { {1,2,3}, {4,5,6} }; does not work
	// palette = {{ {1,2,3}, {4,5,6} }}; works
	// palette = {1,2,3,4,5,6}; as does this
	// TODO any alternative solution here?
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

void PPU::setppuStatus(int bit, bool val) {
	ppuStatus ^= (-val ^ ppuStatus) & (1 << bit);
}

bool PPU::getvBlank() {
	return vBlank;
}

void PPU::setvBlank(bool val) {
	vBlank = val;
}
