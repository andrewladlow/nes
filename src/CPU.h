#ifndef CPU_H_
#define CPU_H_

#include <bitset>
#include <array>
#include <string>
#include <iostream>
#include <iomanip>
#include "PPU.h"

class CPU {
public:
    CPU(PPU *ppu, char *prgRom);
    ~CPU();

    void debug();
    void reset();
    void loadROM();
    void cycle();

    bool getNegativeFlag();
    void setNegativeFlag(bool input);

    bool getOverflowFlag();
    void setOverflowFlag(bool input);

    bool getBreakFlag();
    void setBreakFlag(bool input);

    bool getDecimalModeFlag();
    void setDecimalModeFlag(bool input);

    bool getInterruptDisableFlag();
    void setInterruptDisableFlag(bool input);

    bool getZeroFlag();
    void setZeroFlag(bool input);

    bool getCarryFlag();
    void setCarryFlag(bool input);

    void NMI();

private:
    PPU *ppu;
    char *prgRom;

    char cpuRam[2048];
    char sRam[8192];

    uint16_t pc;
    uint8_t sp;
    std::bitset<8> status; // have status as uint8 instead? modify each bit via shifting?
    // bitset cleaner solution unless other limitations?
    uint8_t accumulator;
    uint8_t regX;
    uint8_t regY;
    uint8_t opcode;
    uint16_t operand;

    bool getBit(unsigned char input, int n);

    void storeMemory(uint16_t address, uint8_t value);
    uint8_t readMemory(uint16_t address);

    void pushStack(uint8_t value);
    uint8_t popStack();

    uint16_t branch(uint16_t pc, uint8_t operand);

    uint16_t resolveAddress(uint16_t address);

    void ADC();
    void AND();
    void ASL();

    void BCC();
    void BCS();
    void BEQ();
    void BIT();
    void BMI();
    void BNE();
    void BPL();
    void BRK();
    void BVC();
    void BVS();

    void CLC();
    void CLD();
    void CLI();
    void CLV();
    void CMP();
    void CPX();
    void CPY();

    void DEC();
    void DEX();
    void DEY();

    void EOR();

    void INC();
    void INX();
    void INY();

    void JMP();
    void JSR();

    void LDA();
    void LDX();
    void LDY();
    void LSR();

    void NOP();

    void ORA();

    void PHA();
    void PHP();
    void PLA();
    void PLP();

    void ROL();
    void ROR();
    void RTI();
    void RTS();

    void SBC();
    void SEC();
    void SED();
    void SEI();
    void STA();
    void STX();
    void STY();

    void TAX();
    void TAY();
    void TSX();
    void TXA();
    void TXS();
    void TYA();

};

#endif
