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
    CPU(PPU *ppu, char *prgRom, int prgRomSize);
    ~CPU();

    void debug();
    void reset();
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

    void setDebug(bool val);

private:
    PPU *ppu;

    char *prgRom;
    int prgRomBanks;

    uint8_t cpuRam[2048];
    uint8_t sRam[8192];

    uint16_t pc;
    uint8_t sp;
    uint8_t status;
    uint8_t accumulator;
    uint8_t regX;
    uint8_t regY;
    uint8_t opcode;
    uint16_t operand;

    bool isDebug;

    bool getBit(unsigned char input, int bit);
    bool getStatusBit(int bit);
    void setStatusBit(int bit, bool val);

    void storeMemory(uint16_t address, uint8_t value);
    uint8_t readMemory(uint16_t address);

    void pushStack(uint8_t value);
    uint8_t popStack();

    uint16_t branch(uint16_t pc, uint8_t operand);

    uint16_t resolveAddress(uint16_t address);

    void AAX();
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

    void DCP();
    void DEC();
    void DEX();
    void DEY();
    void DOP();

    void EOR();

    void INC();
    void INX();
    void INY();
    void ISC();

    void JMP();
    void JSR();

    void LAX();
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

    void RLA();
    void ROL();
    void ROR();
    void RRA();
    void RTI();
    void RTS();

    void SBC();
    void SEC();
    void SED();
    void SEI();
    void SLO();
    void SRE();
    void STA();
    void STX();
    void STY();

    void TAX();
    void TAY();
    void TOP();
    void TSX();
    void TXA();
    void TXS();
    void TYA();

};

#endif
