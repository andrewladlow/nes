#include "CPU.h"

using namespace std;

CPU::CPU(PPU *ppu, char *prgRom) {
	this->ppu = ppu;
	this->prgRom = prgRom;
	cout << "New CPU created" << endl;
}

CPU::~CPU() {
}

void CPU::debug() {
	// having to trick cout by prepending + to variables, due to use of uint8_t - interpreted as char?
	//cout << "PC            A        X       Y       SP     Status" << endl;
	//cout << hex << +pc << "          " << +accumulator << "       " << +regX << "       " << +regY << "       " << +sp << dec << "     " << status << endl;
	cout << "PC:" << left << setw(4) << hex << +pc <<
			" A:" << setw(2) << +accumulator <<
			" X:" << setw(2) << +regX <<
			" Y:" << setw(2) << +regY <<
			" SP:" << setw(2) << +sp << dec <<
			" P:" << status << " ";
}

void CPU::reset() {
	cout << "CPU reset" << endl;
	//pc = 0;
	//Reset interrupts are triggered when the system first starts and when the user presses the
	//reset button. When a reset occurs the system jumps to the address located at $FFFC and
	//$FFFD.
	pc = resolveAddress(0xFFFC);
	//pc = 0x8000;
	sp = 0xFF;
	status.reset();
	accumulator = 0;
	regX = 0;
	regY = 0;
	opcode = 0;
	operand = 0;
	for (int i = 0; i < 3; i++) {
		cpuRam[i] = 0;
	}

	for (int i = 0; i < 8192; i++) {
		sRam[i] = 0;
	}
	//debug();
}

void CPU::loadROM() {

}

uint16_t CPU::resolveAddress(uint16_t address) {
	return ((uint16_t)readMemory(address + 1) << 8) + readMemory(address);
}

void CPU::storeMemory(uint16_t address, uint8_t value) {
	if (address >= 0x6000) {
		sRam[address - 0x6000] = value;
	} else if (address >= 0x2000) {
		switch(address & 0x007) {
		case 0:
			ppu->setControlReg1(value);
			break;
		case 1:
			ppu->setControlReg2(value);
			break;
		case 3:
			ppu->setSramAddrReg(value);
			break;
		case 4:
			ppu->setSramIOReg(value);
			break;
		case 5:
			ppu->setVramAddrReg1(value);
			break;
		case 6:
			ppu->setVramAddrReg2(value);
			break;
		case 7:
			ppu->setVramIOReg(value);
			break;
		}
	} else {
		cpuRam[address & 0x07FF] = value;
	}
}

uint8_t CPU::readMemory(uint16_t address) {
	//cout << "Read memory address: 0x" << hex << address << dec << endl;
	if (address >= 0x8000) { // PRG ROM starting at address 0x8000
		//cout << "PRGROM: " << hex << address - 0x8000 << dec << " = " << hex << +prgRom[address - 0x8000] << dec << endl;
		//cout << "TEST: " << prgRom[0] << endl;
		return prgRom[address - 0x8000];
	} else if (address >= 0x6000) { // SRAM TODO
		cout << "SRAM" << endl;
		return sRam[address - 0x6000];
	} else if (address >= 0x4020) { // Expansion ROM TODO
		cout << "Expansion ROM" << endl;
		return 1;
	} else if (address >= 0x2000) { // PPU TODO
		switch (address & 0x007) { // PPU registers mirrored every 8 bytes
		// control registers write only?
//		case 0:
//			return ppu->getControlReg1();
//			break;
//		case 1:
//			return ppu->getControlReg2();
//			break;
		case 2:
			return ppu->getStatusReg();
			break;
		case 4:
			return ppu->getSramIOReg();
			break;
		case 7:
			return ppu->getVramIOReg();
			break;
		}
	} else {
		return cpuRam[address & 0x07FF]; // contents of 0x000 - 0x07FF mirrored up to 0x2000
	}

	return 0;
}

void CPU::pushStack(uint8_t value) {
	storeMemory(0x100 + sp, value);
	sp--;
}

uint8_t CPU::popStack() {
	sp++;
	uint8_t value = readMemory(0x100 + sp);
	return value;
}

void CPU::cycle() {
	//cout << "CPU Cycle started" << endl;
	debug();
	opcode = readMemory(pc);
	//cout << "Current opcode: 0x" << hex << +opcode << dec << endl;
	cout << "$" << hex << pc << ":" << +opcode << " ";
	uint16_t temp = 0; // store memory locations

	// variables used to display assembly in a more readable format
	string instrName;
	int opCount = 0;

    // Select address mode
    switch (opcode) {
    // Zero page
    case 0x65: case 0x25: case 0x06: case 0x24:
    case 0xC5: case 0xE4: case 0xC4: case 0xC6:
    case 0x45: case 0xE6: case 0xA5: case 0xA6:
    case 0x46: case 0x05: case 0x26: case 0x66:
    case 0xE5: case 0x85: case 0x86: case 0x84:
        operand = readMemory(pc + 1);
        pc += 2;
        opCount = 1;
        break;
    // Zero page, X
    case 0x75: case 0x35: case 0x16: case 0xD5:
    case 0xD6: case 0x55: case 0xF6: case 0xB5:
    case 0xB4: case 0x56: case 0x15: case 0x36:
    case 0x76: case 0xF5: case 0x95: case 0x94:
        operand = (readMemory(pc + 1) + regX) & 0xFF; // wraparound to remain in zero page
        pc += 2;
        opCount = 1;
        break;
    // Zero page, Y
    case 0xB6: case 0x96:
        operand = (readMemory(pc + 1) + regY) & 0xFF;
        pc += 2;
        opCount = 1;
        break;
    // Absolute
    case 0x6D: case 0x2D: case 0x1E: case 0x0E:
    case 0x2C: case 0xCD: case 0xEC: case 0xCC:
    case 0xCE: case 0x4D: case 0xEE: case 0x4C:
    case 0x20: case 0xAD: case 0xAE: case 0xAC:
    case 0x4E: case 0x0D: case 0x2E: case 0x6E:
    case 0xED: case 0x8D: case 0x8E: case 0x8C:
        operand = resolveAddress(pc + 1);
        pc += 3;
        opCount = 2;
        break;
    // Absolute, X
    case 0x7D: case 0x3D: case 0xDD: case 0xDE:
    case 0x5D: case 0xFE: case 0xBD: case 0xBC:
    case 0x1D: case 0x3E: case 0x7E: case 0xFD:
    case 0x9D:
        operand = resolveAddress(pc + 1) + regX;
        pc += 3;
        opCount = 2;
        break;
    // Absolute, Y
    case 0x79: case 0x39: case 0xD9: case 0x59:
    case 0xB9: case 0xBE: case 0x5E: case 0x19:
    case 0xF9: case 0x99:
        operand = resolveAddress(pc + 1) + regY;
        pc += 3;
        opCount = 2;
        break;
    // Indirect (JMP only)
    case 0x6C: {
        temp = resolveAddress(pc + 1);
        // pc set to data in memory address pointed to by operands
        operand = resolveAddress(temp);
        opCount = 2;
        break;
    }
    // Implied
    case 0x18: case 0x38: case 0x58: case 0x78:
    case 0xB8: case 0xD8: case 0xF8: case 0xAA:
    case 0x8A: case 0xCA: case 0xE8: case 0xA8:
    case 0x98: case 0x88: case 0xC8: case 0x9A:
    case 0xBA: case 0x48: case 0x68: case 0x08:
    case 0x28:
    	pc++;
        // no operand
    	opCount = 0;
        break;
    // Accumulator
    case 0x0A: case 0x4A: case 0x2A: case 0x6A:
        operand = accumulator;
        pc++;
    	opCount = 0;
        break;
    // Immediate
    case 0x69: case 0x29: case 0xC9: case 0xE0:
    case 0xC0: case 0x49: case 0xA9: case 0xA2:
    case 0xA0: case 0x09: case 0xE9:
        operand = pc + 1;
        pc += 2;
    	opCount = 1;
        break;
    // Relative
    case 0x10: case 0x30: case 0x50: case 0x70:
    case 0x90: case 0xB0: case 0xD0: case 0xF0:
        operand = pc + 1;
        pc += 2;
    	opCount = 1;
        break;
    // Indirect, X (Pre-indexed)
    case 0x61: case 0x21: case 0xC1: case 0x41:
    case 0xA1: case 0x01: case 0xE1: case 0x81:
    	temp = readMemory(pc + 1);
    	temp = (temp + regX) & 0xFF;
    	operand = resolveAddress(temp);

    	//temp = resolveAddress(pc + 1) + regX;
    	//operand = resolveAddress(temp);

        //temp = pc + 1 + regX;
        //operand = readMemory(temp + 1 << 8 | temp);
        pc += 2;
    	opCount = 1;
        break;
    // Indirect, Y (Post-indexed)
    case 0x71: case 0x31: case 0xD1: case 0x51:
    case 0xB1: case 0x11: case 0xF1: case 0x91:
    	temp = readMemory(pc + 1);
    	temp = resolveAddress(temp);
    	operand = temp + regY; // no wrap around here as we're adding Y to an address that isn't limited to zero page

        //temp = pc + 1;
        //operand = memory[memory[(memory[temp + 1] << 8 | memory[temp]) + regY]];
        //operand = readMemory((temp + 1 << 8 | temp) + regY);
        pc += 2;
    	opCount = 1;
        break;
    default:
        cout << "Address mode: Unknown opcode: 0x" << hex << opcode << " at PC: 0x" << +pc << endl;
        break;
    }

    // Select instruction
    switch (opcode) {
    case 0x69: case 0x65: case 0x75: case 0x6D:
    case 0x7D: case 0x79: case 0x61: case 0x71:
    	instrName = "ADC";
        ADC();
        break;
    case 0x29: case 0x25: case 0x35: case 0x2D:
    case 0x3D: case 0x39: case 0x21: case 0x31:
    	instrName = "AND";
        AND();
        break;
    case 0x0A: case 0x06: case 0x16: case 0x0E:
    case 0x1E:
    	instrName = "ASL";
        ASL();
        break;
    case 0x90:
    	instrName = "BCC";
        BCC();
        break;
    case 0xB0:
    	instrName = "BCS";
        BCS();
        break;
    case 0xF0:
    	instrName = "BEQ";
        BEQ();
        break;
    case 0x24: case 0x2C:
    	instrName = "BIT";
        BIT();
        break;
    case 0x30:
    	instrName = "BMI";
        BMI();
        break;
    case 0xD0:
    	instrName = "BNE";
        BNE();
        break;
	case 0x10:
		instrName = "BPL";
		BPL();
		break;
	case 0x00:
		instrName = "BRK";
		BRK();
		break;
	case 0x50:
    	instrName = "BVC";
        BVC();
        break;
    case 0x70:
    	instrName = "BVS";
        BVS();
        break;
    case 0x18:
    	instrName = "CLC";
        CLC();
        break;
    case 0xD8:
    	instrName = "CLD";
        CLD();
        break;
    case 0x58:
    	instrName = "CLI";
        CLI();
        break;
    case 0xB8:
    	instrName = "CLV";
        CLV();
        break;
    case 0xC9: case 0xC5: case 0xD5: case 0xCD:
    case 0xDD: case 0xD9: case 0xC1: case 0xD1:
    	instrName = "CMP";
        CMP();
        break;
    case 0xE0: case 0xE4: case 0xEC:
    	instrName = "CPX";
        CPX();
        break;
    case 0xC0: case 0xC4: case 0xCC:
    	instrName = "CPY";
    	CPY();
        break;
    case 0xC6: case 0xD6: case 0xCE: case 0xDE:
    	instrName = "DEC";
        DEC();
        break;
    case 0xCA:
    	instrName = "DEX";
        DEX();
        break;
    case 0x88:
    	instrName = "DEY";
        DEY();
        break;
    case 0x49: case 0x45: case 0x55: case 0x4D:
    case 0x5D: case 0x59: case 0x41: case 0x51:
    	instrName = "EOR";
    	EOR();
        break;
    case 0xE6: case 0xF6: case 0xEE: case 0xFE:
    	instrName = "INC";
    	INC();
        break;
    case 0xE8:
    	instrName = "INX";
        INX();
        break;
    case 0xC8:
    	instrName = "INY";
        INY();
        break;
    case 0x6C: case 0x4C:
    	instrName = "JMP";
        JMP();
        break;
    case 0x20:
    	instrName = "JSR";
        JSR();
        break;
    case 0xA9: case 0xA5: case 0xB5: case 0xAD:
    case 0xBD: case 0xB9: case 0xA1: case 0xB1:
    	instrName = "LDA";
        LDA();
        break;
    case 0xA6: case 0xB6: case 0xAE: case 0xBE:
    case 0xA2:
    	instrName = "LDX";
        LDX();
        break;
    case 0xA0: case 0xA4: case 0xB4: case 0xAC:
    case 0xBC:
    	instrName = "LDY";
        LDY();
        break;
    case 0x4A: case 0x46: case 0x56: case 0x4E:
    case 0x5E:
    	instrName = "LSR";
        LSR();
        break;
    case 0xEA:
    	instrName = "NOP";
        NOP();
        break;
    case 0x09: case 0x05: case 0x15: case 0x0D:
    case 0x1D: case 0x19: case 0x01: case 0x11:
    	instrName = "ORA";
        ORA();
        break;
    case 0x48:
    	instrName = "PHA";
        PHA();
        break;
    case 0x08:
    	instrName = "PHP";
        PHP();
        break;
    case 0x68:
    	instrName = "PLA";
        PLA();
        break;
    case 0x28:
    	instrName = "PLP";
        PLP();
        break;
    case 0x2A: case 0x26: case 0x36: case 0x2E:
    case 0x3E:
    	instrName = "ROL";
        ROL();
        break;
    case 0x6A: case 0x66: case 0x76: case 0x6E:
    case 0x7E:
    	instrName = "ROR";
        ROR();
        break;
    case 0x40:
    	instrName = "RTI";
        RTI();
        break;
    case 0x60:
    	instrName = "RTS";
        RTS();
        break;
    case 0xE9: case 0xE5: case 0xF5: case 0xED:
    case 0xFD: case 0xF9: case 0xE1: case 0xF1:
    	instrName = "SBC";
        SBC();
        break;
    case 0x38:
    	instrName = "SEC";
        SEC();
        break;
    case 0xF8:
    	instrName = "SED";
        SED();
        break;
    case 0x78:
    	instrName = "SEI";
        SEI();
        break;
    case 0x85: case 0x95: case 0x8D: case 0x9D:
    case 0x99: case 0x81: case 0x91:
    	instrName = "STA";
        STA();
        break;
    case 0x86: case 0x8E:
    	instrName = "STX";
        STX();
        break;
    case 0x84: case 0x94: case 0x8C:
    	instrName = "STY";
        STY();
        break;
    case 0xAA:
    	instrName = "TAX";
        TAX();
        break;
    case 0xA8:
    	instrName = "TAY";
        TAY();
        break;
    case 0xBA:
    	instrName = "TSX";
        TSX();
        break;
    case 0x8A:
    	instrName = "TXA";
        TXA();
        break;
    case 0x9A:
    	instrName = "TXS";
        TXS();
        break;
    case 0x98:
    	instrName = "TYA";
        TYA();
        break;
    default:
        cout << "Instruction select: Unknown opcode: 0x" << +opcode << " at PC: 0x" << +pc << endl;
        break;
    }

    cout << instrName << " ";
    if (opCount == 2) {
		cout << hex << operand << " = " << +readMemory(operand);
	} else if (opCount == 1) {
		cout << hex << +readMemory(operand);
	}
    cout << endl;
}

void CPU::ADC() {
	uint16_t temp = readMemory(operand) + accumulator + getCarryFlag();
	setCarryFlag(temp > 0xFF);
	setZeroFlag(temp == 0);
	// TODO test and fix this overflow calc
	setOverflowFlag((getBit(temp, 6) + getBit(temp, 7)) ^ (getBit(temp, 7) + status[0]));
	setNegativeFlag(temp & 0x80);
	accumulator = temp;
}

void CPU::AND() {
	accumulator &= readMemory(operand);
	setZeroFlag(accumulator == 0);
	setNegativeFlag(accumulator & 0x80);
}

void CPU::ASL() {
	uint8_t temp;
	if (opcode != 0x0A) {
		temp = readMemory(operand);
	} else {
		temp = operand;
	}
	setCarryFlag(temp & 0x80);
	temp <<= 1;
	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
	if (opcode != 0x0A) {
		storeMemory(operand, temp);
	} else {
		accumulator = temp;
	}
}

void CPU::BCC() {
	if (!getCarryFlag()) {
		//pc += readMemory(operand);
		// operand interpreted as a signed byte, therefore ranges from -128 to 127
		// mask with 7th bit and subsequently subtract 128
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BCS() {
	if (getCarryFlag()) {
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BEQ() {
	if (getZeroFlag()) {
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BIT() {
	setZeroFlag(accumulator & readMemory(operand));
	// set overflow and negative (6,7) to memory value (6,7)
	// TODO possible refactor here - no abstraction present
	//status[6] = getBit(memory[operand], 6);
	setOverflowFlag(getBit(readMemory(operand), 6));
	setNegativeFlag(getBit(readMemory(operand), 7));
}

void CPU::BMI() {
	if (getNegativeFlag()) {
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BNE() {
	if (!getZeroFlag()) {
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BPL() {
	if (!getNegativeFlag()) {
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BRK() {
	// push pc and status regs onto stack
	pushStack(pc & 0xFF00); // MSB first
	pushStack(pc & 0x00FF);
	pushStack((unsigned char) status.to_ulong());
	// load IRQ val
	pc = resolveAddress(0xFFFE);
	setBreakFlag(1);
}

void CPU::BVC() {
	if (!getOverflowFlag()) {
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BVS() {
	if (getOverflowFlag()) {
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::CLC() {
	setCarryFlag(0);
}

void CPU::CLD() {
	setDecimalModeFlag(0);
}

void CPU::CLI() {
	setInterruptDisableFlag(0);
}

void CPU::CLV() {
	setOverflowFlag(0);
}

void CPU::CMP() {
	uint16_t temp = accumulator - readMemory(operand);
	setCarryFlag(temp >= 0);
	setZeroFlag(temp == 0);
	// negative flag = result[7]
	setNegativeFlag(temp & 0x80);
}

void CPU::CPX() {
	uint16_t temp = regX - readMemory(operand);
	setCarryFlag(temp >= 0);
	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
}

void CPU::CPY() {
	uint16_t temp = regY - readMemory(operand);
	setCarryFlag(temp >= 0);
	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
}

void CPU::DEC() {
	uint16_t temp = readMemory(operand) - 1;
	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
	storeMemory(operand, temp);
}

void CPU::DEX() {
	regX--;
	setZeroFlag(regX == 0);
	setNegativeFlag(regX & 0x80);
}

void CPU::DEY() {
	regY--;
	setZeroFlag(regY == 0);
	setNegativeFlag(regY & 0x80);
}

void CPU::EOR() {
	accumulator ^= readMemory(operand);
	setZeroFlag(accumulator == 0);
	setNegativeFlag(accumulator & 0x80);
}

void CPU::INC() {
	uint16_t temp = readMemory(operand) + 1;
	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
	storeMemory(operand, temp);
}

void CPU::INX() {
	regX++;
	setZeroFlag(regX == 0);
	setNegativeFlag(regX & 0x80);
}

void CPU::INY() {
	regY++;
	setZeroFlag(regY == 0);
	setNegativeFlag(regY & 0x80);
}

void CPU::JMP() {
	// TODO possible JMP problem with indirect on page boundary
	pc = readMemory(operand);
}

void CPU::JSR() {
	pc--;
	pushStack(pc & 0xFF00);
	pushStack(pc & 0x00FF);
	pc = readMemory(operand);
}

void CPU::LDA() {
	accumulator = readMemory(operand);
	setZeroFlag(accumulator == 0);
	setNegativeFlag(accumulator & 0x80);
}

void CPU::LDX() {
	regX = readMemory(operand);
	setZeroFlag(regX == 0);
	setNegativeFlag(regX & 0x80);
}

void CPU::LDY() {
	regY = readMemory(operand);
	setZeroFlag(regY == 0);
	setNegativeFlag(regY & 0x80);
}

void CPU::LSR() {
	uint8_t temp;
	if (opcode != 0x4A) {
		temp = readMemory(operand);
	} else {
		temp = operand;
	}
	setCarryFlag(temp & 0x01);
	temp >>= 1;
	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
	if (opcode != 0x4A) {
		storeMemory(operand, temp);
	} else {
		accumulator = temp;
	}
/*	uint16_t temp;
	if (opcode )
	int result = operand >> 1;
	setCarryFlag(getBit(operand, 0));
	setZeroFlag(result == 0);
	setNegativeFlag(result & 0x80);
	accumulator = (result & 0xFF);*/
}

void CPU::NOP() {

}

void CPU::ORA() {
	accumulator |= readMemory(operand);
	setZeroFlag(accumulator == 0);
	setNegativeFlag(accumulator & 0x80);
}

void CPU::PHA() {
	pushStack(accumulator);
}

void CPU::PHP() {
	pushStack((unsigned char) status.to_ulong());
}

void CPU::PLA() {
	accumulator = popStack();
	setZeroFlag(accumulator == 0);
	setNegativeFlag(accumulator & 0x80);
}

void CPU::PLP() {
	// TODO test this function
	string temp = to_string(popStack());
	for (int i = 0; i < 7; i++) {
		status[i] = temp[i];
	}
}

void CPU::ROL() {
	uint8_t temp;
	if (opcode != 0x2A) {
		temp = readMemory(operand);
	} else {
		temp = operand;
	}
	// rotate 1 byte left, bit 0 becomes old carry, carry becomes old bit 7
	uint8_t carry = getCarryFlag();
	setCarryFlag(temp & 0x80);
	temp <<= 1;
	temp |= carry;

	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
	if (opcode != 0x2A) {
		storeMemory(operand, temp);
	} else {
		accumulator = temp;
	}
//	int result = operand << 1;
//	setCarryFlag(getBit(operand, 7));
//	setZeroFlag(result);
//	setNegativeFlag(result & 0x80);
//	accumulator = (result & 0xFF);
}

void CPU::ROR() {
	uint8_t temp;
	if (opcode != 0x6A) {
		temp = readMemory(operand);
	} else {
		temp = operand;
	}
	// rotate 1 byte right, bit 7 becomes old carry, carry becomes old bit 0
	uint8_t carry = getCarryFlag();
	// move carry to 7th bit as we're replacing bit 7, rather than bit 0 as with ROL
	carry <<= 7;
	setCarryFlag(temp & 0x01);
	temp >>= 1;
	temp |= carry;

	setZeroFlag(temp == 0);
	setNegativeFlag(temp & 0x80);
	if (opcode != 0x6A) {
		storeMemory(operand, temp);
	} else {
		accumulator = temp;
	}
//	int result = operand >> 1;
//	setCarryFlag(getBit(operand, 0));
//	setZeroFlag(result);
//	setNegativeFlag(result & 0x80);
//	accumulator = (result & 0xFF);
}

void CPU::RTI() {
	status = popStack();
	//pc = stack[sp - 1] << 8 | stack[sp - 2];
	pc = popStack();
	pc |= (uint16_t) popStack() << 8;
	// TODO test, possibly incorrect
}

void CPU::RTS() {
	pc = popStack();
	pc |= (uint16_t) popStack() << 8;
	pc++;
}

void CPU::SBC() {
	uint16_t temp = accumulator - readMemory(operand) - (1 - getCarryFlag());
	setCarryFlag(temp > 0xFF);
	setZeroFlag(temp == 0);
	// TODO test and fix this overflow calc
	setOverflowFlag((getBit(temp, 6) + getBit(temp, 7)) ^ (getBit(temp, 7) + status[0]));
	setNegativeFlag(temp & 0x80);
	accumulator = temp;

//	uint16_t temp = accumulator - readMemory(operand) - (1 - getCarryFlag());
//	setCarryFlag(result > 0xFF);
//	setZeroFlag(accumulator == 0);
//	// TODO test and fix this overflow calc
//	setOverflowFlag(!((getBit(result, 6) + getBit(result, 7)) ^ (getBit(result, 7) + status[0])));
//	setNegativeFlag(result & 0x80);
//	accumulator = (result & 0xFF);
}

void CPU::SEC() {
	setCarryFlag(1);
}

void CPU::SED() {
	setDecimalModeFlag(1);
}

void CPU::SEI() {
	setInterruptDisableFlag(1);
}

void CPU::STA() {
	storeMemory(operand, accumulator);
	//operand = accumulator;
}

void CPU::STX() {
	storeMemory(operand, regX);
	//operand = regX;
}

void CPU::STY() {
	storeMemory(operand, regY);
	//operand = regY;
}

void CPU::TAX() {
	regX = accumulator;
	setZeroFlag(regX == 0);
	setNegativeFlag(regX & 0x80);
}

void CPU::TAY() {
	regY = accumulator;
	setZeroFlag(regY == 0);
	setNegativeFlag(regY & 0x80);
}

void CPU::TSX() {
	regX = sp;
	//regX = popStack();
	setZeroFlag(regX == 0);
	setNegativeFlag(regX & 0x80);
}

void CPU::TXA() {
	accumulator = regX;
	setZeroFlag(accumulator == 0);
	setNegativeFlag(accumulator & 0x80);
}

void CPU::TXS() {
	sp = regX;
	//pushStack(regX);
}

void CPU::TYA() {
	accumulator = regY;
	setZeroFlag(accumulator == 0);
	setNegativeFlag(accumulator & 0x80);
}

bool CPU::getNegativeFlag() {
	return status[7];
}

void CPU::setNegativeFlag(bool input) {
	status[7] = input;
}

bool CPU::getOverflowFlag() {
	return status[6];
}

//void CPU::setOverflowFlag(char input) {
//    status[6] = (getBit(input, 6) + getBit(input, 7)) ^ (getBit(input, 7) + status[0]);
//}

void CPU::setOverflowFlag(bool input) {
	status[6] = input;
}

bool CPU::getBreakFlag() {
	return status[4];
}

void CPU::setBreakFlag(bool input) {
	status[4] = input;
}

bool CPU::getDecimalModeFlag() {
	return status[3];
}

void CPU::setDecimalModeFlag(bool input) {
	status[3] = input;
}

bool CPU::getInterruptDisableFlag() {
	return status[2];
}

void CPU::setInterruptDisableFlag(bool input) {
	status[2] = input;
}

bool CPU::getZeroFlag() {
	return status[1];
}

void CPU::setZeroFlag(bool input) {
	status[1] = input;
}

bool CPU::getCarryFlag() {
	return status[0];
}

void CPU::setCarryFlag(bool input) {
	status[0] = input;
}

bool CPU::getBit(unsigned char input, int n) {
	return (input >> n) & 1;
}