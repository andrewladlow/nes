#include <iostream>
#include "PPU.h"

using namespace std;

PPU::PPU() {
    cout << "New PPU created" << endl;

    controlReg1 = 0;
    controlReg2 = 0;
    statusReg = 0;
    sramAddrReg = 0;
    sramIOReg = 0;
    vramAddrReg1 = 0;
    vramAddrReg2 = 0;
    vramIOReg = 0;
}

PPU::~PPU() {

}

uint8_t PPU::getControlReg1() {
	return controlReg1;
}

void PPU::setControlReg1(uint8_t value) {
	controlReg1 = value;
}

uint8_t PPU::getControlReg2() {
	return controlReg2;
}

void PPU::setControlReg2(uint8_t value) {
	controlReg2 = value;
}

uint8_t PPU::getStatusReg() {
	return statusReg;
}

void PPU::setStatusReg(uint8_t value) {
	statusReg = value;
}

uint8_t PPU::getSramAddrReg() {
	return sramAddrReg;
}

void PPU::setSramAddrReg(uint8_t value) {
	sramAddrReg = value;
}

uint8_t PPU::getSramIOReg() {
	return sramIOReg;
}

void PPU::setSramIOReg(uint8_t value) {
	sramIOReg = value;
}

uint8_t PPU::getVramAddrReg1() {
	return vramAddrReg1;
}

void PPU::setVramAddrReg1(uint8_t value) {
	vramAddrReg1 = value;
}

uint8_t PPU::getVramAddrReg2() {
	return vramAddrReg2;
}

void PPU::setVramAddrReg2(uint8_t value ) {
	vramAddrReg2 = value;
}

uint8_t PPU::getVramIOReg() {
	return vramIOReg;
}

void PPU::setVramIOReg(uint8_t value) {
	vramIOReg = value;
}
