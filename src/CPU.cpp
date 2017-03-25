#include "CPU.h"

using namespace std;

CPU::CPU(PPU *ppu, char *prgRom, int prgRomBanks) {
	this->ppu = ppu;
	this->prgRom = prgRom;
	this->prgRomBanks = prgRomBanks;
	//cout << "New CPU created" << endl;
	reset();
}

CPU::~CPU() {
}

void CPU::debug() {
	if (!isDebug) {
		isDebug = true;
	}
	cout << "PC:" << left << setw(4) << hex << +pc <<
			" A:" << setw(2) << +accumulator <<
			" X:" << setw(2) << +regX <<
			" Y:" << setw(2) << +regY <<
			" SP:" << setw(2) << +sp << dec <<
			" P:" << bitset<8>(status) << " " << "(" << hex << +status << ") ";
}

void CPU::reset() {
	//cout << "CPU reset" << endl;
	//Reset interrupts are triggered when the system first starts and when the user presses the
	//reset button. When a reset occurs the system jumps to the address located at $FFFC and
	//$FFFD.
	//pc = resolveAddress(0xFFFC);
	pc = 0xC000;
	//pc = 0x8000;
	sp = 0xFD;
	status = 0x24;
	accumulator = 0;
	regX = 0;
	regY = 0;
	opcode = 0;
	operand = 0;
	for (int i = 0; i < 8192; i++) {
		cpuRam[i] = 0;
	}

	for (int i = 0; i < 8192; i++) {
		sRam[i] = 0;
	}

	isDebug = false;
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
			ppu->setppuCtrl(value);
			break;
		case 1:
			ppu->setppuMask(value);
			break;
		case 3:
			ppu->setoamAddr(value);
			break;
		case 4:
			ppu->setoamData(value);
			break;
		case 5:
			ppu->setppuScroll(value);
			break;
		case 6:
			ppu->setppuAddr(value);
			break;
		case 7:
			ppu->setppuData(value);
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
		// TODO memory mapper support
		if (prgRomBanks == 1) {
			return prgRom[address - 0xc000];
		} else {
			return prgRom[address - 0x8000];
		}
	} else if (address >= 0x6000) { // SRAM TODO
		cout << "SRAM" << endl;
		return sRam[address - 0x6000];
	} else if (address >= 0x4020) { // Expansion ROM TODO
		cout << "Expansion ROM" << endl;
		return 1;
	} else if (address >= 0x2000) {
		switch (address & 0x007) { // PPU registers mirrored every 8 bytes
		case 2:
			return ppu->getppuStatus();
			break;
		case 4:
			return ppu->getoamData();
			break;
		case 7:
			return ppu->getVramAddr();
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

void CPU::NMI() {
	cout << "Debug: NMI occured" << endl;
	// push pc and status regs onto stack
	pushStack(pc >> 8);
	pushStack(pc);
	pushStack(status);
	//cout << hex << +status << endl;
	//SEI();
	// load NMI val
	pc = resolveAddress(0xFFFA);
	//RTI();
}

void CPU::cycle() {

	//cout << "CPU Cycle started" << endl;
	opcode = readMemory(pc);
	//cout << "Current opcode: 0x" << hex << +opcode << dec << endl;
	if (isDebug) {
		debug();
		cout << "$" << hex << pc << ":" << +opcode << " ";
	}

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
    case 0xA4: case 0x04: case 0x44: case 0x64:
    case 0xA7: case 0x87: case 0xC7: case 0xE7:
    case 0x07: case 0x27: case 0x47: case 0x67:
        operand = readMemory(pc + 1);
        pc += 2;
        opCount = 1;
        break;
    // Zero page, X
    case 0x75: case 0x35: case 0x16: case 0xD5:
    case 0xD6: case 0x55: case 0xF6: case 0xB5:
    case 0xB4: case 0x56: case 0x15: case 0x36:
    case 0x76: case 0xF5: case 0x95: case 0x94:
    case 0x14: case 0x34: case 0x54: case 0x74:
    case 0xD4: case 0xF4: case 0xD7: case 0xF7:
    case 0x17: case 0x37: case 0x57: case 0x77:
        operand = (readMemory(pc + 1) + regX) & 0xFF; // wraparound to remain in zero page
        pc += 2;
        opCount = 1;
        break;
    // Zero page, Y
    case 0xB6: case 0x96: case 0xB7: case 0x97:
        operand = (readMemory(pc + 1) + regY) & 0xFF;
        pc += 2;
        opCount = 1;
        break;
    // Absolute
    case 0x6D: case 0x2D: case 0x6F: case 0x0E:
    case 0x2C: case 0xCD: case 0xEC: case 0xCC:
    case 0xCE: case 0x4D: case 0xEE: case 0x4C:
    case 0x20: case 0xAD: case 0xAE: case 0xAC:
    case 0x4E: case 0x0D: case 0x2E: case 0x6E:
    case 0xED: case 0x8D: case 0x8E: case 0x8C:
    case 0x0C: case 0xAF: case 0x8F: case 0xCF:
    case 0xEF: case 0x0F: case 0x2F: case 0x4F:
        operand = resolveAddress(pc + 1);
        pc += 3;
        // TODO revise fix here
        if (opcode == 0x4C || opcode == 0x20) {
        	opCount = 3;
        } else {
        	opCount = 2;
        }
        break;
    // Absolute, X
    case 0x7D: case 0x3D: case 0xDD: case 0xDE:
    case 0x5D: case 0xFE: case 0xBD: case 0xBC:
    case 0x1D: case 0x3E: case 0x7E: case 0xFD:
    case 0x9D: case 0x1C: case 0x3C: case 0x5C:
    case 0x7C: case 0xDC: case 0xFC: case 0xDF:
    case 0xFF: case 0x1F: case 0x3F: case 0x5F:
    case 0x7F: case 0x5E: case 0x1E:
        operand = resolveAddress(pc + 1) + regX;
        pc += 3;
        opCount = 2;
        break;
    // Absolute, Y
    case 0x79: case 0x39: case 0xD9: case 0x59:
    case 0xB9: case 0xBE: case 0x7B: case 0x19:
    case 0xF9: case 0x99: case 0xBF: case 0xDB:
    case 0xFB: case 0x1B: case 0x3B: case 0x5B:
        operand = resolveAddress(pc + 1) + regY;
        pc += 3;
        opCount = 2;
        break;
    // Indirect (JMP only)
    case 0x6C: {
        temp = resolveAddress(pc + 1);
        // pc set to data in memory address pointed to by operands
        // handle page boundary (if low byte is FF, high byte is not increased)
        if (temp & 0x00FF == 0xFF) {
        	operand = ((uint16_t)readMemory(temp & 0xFF00) << 8) + readMemory(temp);
        }
        else {
            operand = resolveAddress(temp);
        }
        opCount = 1;
        break;
    }
    // Implied
    case 0x18: case 0x38: case 0x58: case 0x78:
    case 0xB8: case 0xD8: case 0xF8: case 0xAA:
    case 0x8A: case 0xCA: case 0xE8: case 0xA8:
    case 0x98: case 0x88: case 0xC8: case 0x9A:
    case 0xBA: case 0x48: case 0x68: case 0x08:
    case 0x28: case 0x60: case 0xEA: case 0x40:
    case 0x1A: case 0x3A: case 0x5A: case 0x7A:
    case 0xDA: case 0xFA:

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
    case 0xA0: case 0x09: case 0xE9: case 0x80:
    case 0x82: case 0x89: case 0xC2: case 0xE2:
    case 0xEB:
        operand = pc + 1;
        pc += 2;
    	opCount = 1;
        break;
    // Relative
    case 0x10: case 0x30: case 0x50: case 0x70:
    case 0x90: case 0xB0: case 0xD0: case 0xF0:
        operand = pc + 1;
        pc += 2;
    	opCount = 4;
    	temp = pc + (readMemory(operand) ^ 0x80) - 0x80;
        break;
    // Indirect, X (Pre-indexed)
    case 0x61: case 0x21: case 0xC1: case 0x41:
    case 0xA1: case 0x01: case 0xE1: case 0x81:
    case 0xA3: case 0x83: case 0xC3: case 0xE3:
    case 0x03: case 0x23: case 0x43: case 0x63:
    	temp = readMemory(pc + 1);
    	temp = (temp + regX) & 0xFF;
    	if (temp == 0xFF) {
        	operand = ((uint16_t)readMemory(0x0000) << 8) + readMemory(0x00FF);
    	} else {
    		operand = resolveAddress(temp);
    	}
        pc += 2;
    	opCount = 1;
        break;
    // Indirect, Y (Post-indexed)
    case 0x71: case 0x31: case 0xD1: case 0x51:
    case 0xB1: case 0x11: case 0xF1: case 0x91:
    case 0xB3: case 0xD3: case 0xF3: case 0x13:
    case 0x33: case 0x53: case 0x73:
    	temp = readMemory(pc + 1);
    	if (temp == 0xFF) {
    		temp = 	((uint16_t)readMemory(0x0000) << 8) + readMemory(0x00FF);
    	} else {
    		temp = resolveAddress(temp);
    	}
    	operand = temp + regY;
        pc += 2;
    	opCount = 1;
        break;
    default:
        cout << "Address mode: Unknown opcode: 0x" << hex << +opcode << " at PC: 0x" << +pc << endl;
        break;
    }

    // Select instruction
    switch (opcode) {
    case 0x87: case 0x97: case 0x83: case 0x8F:
    	instrName="*AAX";
    	AAX();
    	break;
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
    case 0xC7: case 0xD7: case 0xCF: case 0xDF:
    case 0xDB: case 0xC3: case 0xD3:
    	instrName = "*DCP";
    	DCP();
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
    case 0x04: case 0x14: case 0x34: case 0x44:
    case 0x54: case 0x64: case 0x74: case 0x80:
    case 0x82: case 0x89: case 0xC2: case 0xD4:
    case 0xE2: case 0xF4:
    	instrName = "*NOP";
    	DOP();
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
    case 0xE7: case 0xF7: case 0xEF: case 0xFF:
    case 0xFB: case 0xE3: case 0xF3:
    	instrName = "*ISC";
    	ISC();
    	break;
    case 0x6C: case 0x4C:
    	instrName = "JMP";
        JMP();
        break;
    case 0x20:
    	instrName = "JSR";
        JSR();
        break;
    case 0xA7: case 0xB7: case 0xAF: case 0xBF:
    case 0xA3: case 0xB3:
    	instrName = "*LAX";
    	LAX();
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
    case 0xEA: case 0x1A: case 0x3A: case 0x5A:
    case 0x7A: case 0xDA: case 0xFA:
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
    case 0x27: case 0x37: case 0x2F: case 0x3F:
    case 0x3B: case 0x23: case 0x33:
    	instrName = "*RLA";
    	RLA();
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
    case 0x67: case 0x77: case 0x6F: case 0x7F:
    case 0x7B: case 0x63: case 0x73:
    	instrName = "*RRA";
    	RRA();
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
    case 0xEB:
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
    case 0x07: case 0x17: case 0x0F: case 0x1F:
    case 0x1B: case 0x03: case 0x13:
    	instrName = "*SLO";
    	SLO();
    	break;
    case 0x47: case 0x57: case 0x4F: case 0x5F:
    case 0x5B: case 0x43: case 0x53:
    	instrName = "*SRE";
    	SRE();
    	break;
    case 0x85: case 0x95: case 0x8D: case 0x9D:
    case 0x99: case 0x81: case 0x91:
    	instrName = "STA";
        STA();
        break;
    case 0x86: case 0x96: case 0x8E:
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
    case 0x0C: case 0x1C: case 0x3C: case 0x5C:
    case 0x7C: case 0xDC: case 0xFC:
    	instrName = "*NOP";
    	TOP();
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

    if (isDebug) {
		cout << instrName << " ";
		if (opCount == 4) {
			cout << temp; // TODO more elegant solution for this...
		} else if (opCount == 3) {
			cout << hex << operand;
		} else if (opCount == 2) {
			cout << hex << operand << " = " << +readMemory(operand);
		} else if (opCount == 1) {
			cout << hex << +readMemory(operand);
		}
		cout << endl;
    }
}

void CPU::AAX() {
	uint8_t temp = accumulator & regX;
//	setZeroFlag(temp);
//	setNegativeFlag(temp);
	storeMemory(operand, temp);
}

void CPU::ADC() {
	uint16_t temp = readMemory(operand) + accumulator + getCarryFlag();
	setCarryFlag(temp > 0xFF);
	setZeroFlag(temp & 0xFF);
	// overflow calc from http://nesdev.com/6502.txt
	//setOverflowFlag((getBit(temp, 6) + getBit(temp, 7)) ^ (getBit(temp, 7) + getBit(status,0)));
	setOverflowFlag(!((accumulator ^ readMemory(operand)) & 0x80) && ((accumulator ^ temp) & 0x80));
	setNegativeFlag(temp);
	accumulator = temp & 0xFF;
}

void CPU::AND() {
	accumulator &= readMemory(operand);
	setZeroFlag(accumulator);
	setNegativeFlag(accumulator);
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
	setZeroFlag(temp);
	setNegativeFlag(temp);
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
	uint8_t temp = readMemory(operand);
	setZeroFlag(accumulator & temp);
	// set overflow and negative (6,7) to memory value (6,7)
	// TODO possible refactor here - no abstraction present
	//status[6] = getBit(memory[operand], 6);
	setOverflowFlag(0x40 & temp);
	setNegativeFlag(temp);
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
		//cout << "TEST1: " << pc << endl;
		//cout << "TEST2: " << (readMemory(operand) ^ 0x80) - 0x80 << endl;
		//cout << "TEST3: " << pc + (readMemory(operand) ^ 0x80) - 0x80 << endl;
		pc += (readMemory(operand) ^ 0x80) - 0x80;
	}
}

void CPU::BRK() {
	// push pc and status regs onto stack
	pushStack(pc >> 8);
	pushStack(pc);
	setBreakFlag(1);
	pushStack(status);
	SEI();
	// load IRQ val
	pc = resolveAddress(0xFFFE);
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
	setCarryFlag(temp < 0x100);
	setZeroFlag(temp);
	// negative flag = result[7]
	setNegativeFlag(temp);
}

void CPU::CPX() {
	uint16_t temp = regX - readMemory(operand);
	setCarryFlag(temp < 0x100);
	setZeroFlag(temp);
	setNegativeFlag(temp);
}

void CPU::CPY() {
	uint16_t temp = regY - readMemory(operand);
	setCarryFlag(temp < 0x100);
	setZeroFlag(temp);
	setNegativeFlag(temp);
}

void CPU::DCP() {
	DEC();
	CMP();
}

void CPU::DEC() {
	uint8_t temp = readMemory(operand) - 1;
	setZeroFlag(temp);
	setNegativeFlag(temp);
	storeMemory(operand, temp);
}

void CPU::DEX() {
	regX--;
	setZeroFlag(regX);
	setNegativeFlag(regX);
}

void CPU::DEY() {
	regY--;
	setZeroFlag(regY);
	setNegativeFlag(regY);
}

void CPU::DOP() {
	NOP();
}

void CPU::EOR() {
	accumulator ^= readMemory(operand);
	setZeroFlag(accumulator);
	setNegativeFlag(accumulator);
}

void CPU::INC() {
	uint16_t temp = readMemory(operand) + 1;
	setZeroFlag(temp);
	setNegativeFlag(temp);
	storeMemory(operand, temp);
}

void CPU::INX() {
	regX++;
	setZeroFlag(regX);
	setNegativeFlag(regX);
}

void CPU::INY() {
	regY++;
	setZeroFlag(regY);
	setNegativeFlag(regY);
}

void CPU::ISC() {
	INC();
	SBC();
}

void CPU::JMP() {
	// TODO possible JMP problem with indirect on page boundary
	pc = operand;
}

void CPU::JSR() {
	pc--;
	pushStack(pc >> 8);
	pushStack(pc);
	pc = operand;
}

void CPU::LAX() {
	uint8_t temp = readMemory(operand);
	setZeroFlag(temp);
	setNegativeFlag(temp);
	accumulator = temp;
	regX = temp;
}

void CPU::LDA() {
	accumulator = readMemory(operand);
	setZeroFlag(accumulator);
	//cout << "TEST: " << +accumulator << " " << (accumulator & 0x80) << endl;
	setNegativeFlag(accumulator);
}

void CPU::LDX() {
	regX = readMemory(operand);
	setZeroFlag(regX);
	setNegativeFlag(regX);
}

void CPU::LDY() {
	regY = readMemory(operand);
	setZeroFlag(regY);
	setNegativeFlag(regY);
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
	setZeroFlag(temp);
	setNegativeFlag(temp);
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
	//cout <<"TEST1: " << hex << +operand << endl;
	accumulator |= readMemory(operand);
	setZeroFlag(accumulator);
	setNegativeFlag(accumulator);
}

void CPU::PHA() {
	pushStack(accumulator);
}

void CPU::PHP() {
	pushStack(status);
}

void CPU::PLA() {
	accumulator = popStack();
	setZeroFlag(accumulator);
	setNegativeFlag(accumulator);
}

void CPU::PLP() {
	status = popStack();
}

void CPU::RLA() {
	ROL();
	AND();
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

	setZeroFlag(temp);
	setNegativeFlag(temp);
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

	setZeroFlag(temp);
	setNegativeFlag(temp);
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

void CPU::RRA() {
	ROR();
	ADC();
}

void CPU::RTI() {
	status = popStack();
	//pc = stack[sp - 1] << 8 | stack[sp - 2];
	pc = popStack();
	pc |= (uint16_t)popStack() << 8;
	// TODO test, possibly incorrect
}

void CPU::RTS() {
	pc = popStack();
	pc |= (uint16_t)popStack() << 8;
	pc++;
}

void CPU::SBC() {
	uint16_t temp = accumulator - readMemory(operand) - (1 - getCarryFlag());
	setCarryFlag(temp < 0x100);
	setZeroFlag(temp & 0xFF);
	setOverflowFlag(((accumulator ^ temp) & 0x80) && ((accumulator ^ readMemory(operand)) & 0x80));
	setNegativeFlag(temp);
	accumulator = temp & 0xFF;

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

void CPU::SLO() {
	ASL();
	ORA();
}

void CPU::SRE() {
	LSR();
	EOR();
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
	setZeroFlag(regX);
	setNegativeFlag(regX);
}

void CPU::TAY() {
	regY = accumulator;
	setZeroFlag(regY);
	setNegativeFlag(regY);
}

void CPU::TOP() {
	NOP();
}

void CPU::TSX() {
	regX = sp;
	//regX = popStack();
	setZeroFlag(regX);
	setNegativeFlag(regX);
}

void CPU::TXA() {
	accumulator = regX;
	setZeroFlag(accumulator);
	setNegativeFlag(accumulator);
}

void CPU::TXS() {
	sp = regX;
	//pushStack(regX);
}

void CPU::TYA() {
	accumulator = regY;
	setZeroFlag(accumulator);
	setNegativeFlag(accumulator);
}

bool CPU::getNegativeFlag() {
	return getBit(status, 7);
}

void CPU::setNegativeFlag(uint8_t input) {
	setStatusBit(7, input & 0x80);
}

bool CPU::getOverflowFlag() {
	return getBit(status, 6);
}

//void CPU::setOverflowFlag(char input) {
//    status[6] = (getBit(input, 6) + getBit(input, 7)) ^ (getBit(input, 7) + status[0]);
//}

void CPU::setOverflowFlag(uint8_t input) {
	setStatusBit(6, input);
}

bool CPU::getBreakFlag() {
	return getBit(status, 4);
}

void CPU::setBreakFlag(uint8_t input) {
	setStatusBit(4, input);
}

bool CPU::getDecimalModeFlag() {
	return getBit(status, 3);
}

void CPU::setDecimalModeFlag(uint8_t input) {
	setStatusBit(3, input);
}

bool CPU::getInterruptDisableFlag() {
	return getBit(status, 2);
}

void CPU::setInterruptDisableFlag(uint8_t input) {
	setStatusBit(2, input);
}

bool CPU::getZeroFlag() {
	return getBit(status, 1);
}

void CPU::setZeroFlag(uint8_t input) {
	setStatusBit(1, !input);
}

bool CPU::getCarryFlag() {
	return getBit(status, 0);
}

void CPU::setCarryFlag(uint8_t input) {
	setStatusBit(0, input);
}

bool CPU::getBit(unsigned char input, int bit) {
	return (input >> bit) & 1;
}

void CPU::setStatusBit(int bit, bool val) {
	status ^= (-val ^ status) & (1 << bit);
}

void CPU::setDebug(bool val) {
	isDebug = val;
}
