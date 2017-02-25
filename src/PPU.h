#ifndef PPU_H
#define PPU_H


class PPU
{
public:
    PPU();
    ~PPU();

    uint8_t getControlReg1();
    void setControlReg1(uint8_t value);

    uint8_t getControlReg2();
    void setControlReg2(uint8_t value);

    uint8_t getStatusReg();
    void setStatusReg(uint8_t value);

    uint8_t getSramAddrReg();
    void setSramAddrReg(uint8_t value);

    uint8_t getSramIOReg();
    void setSramIOReg(uint8_t value);

    uint8_t getVramAddrReg1();
    void setVramAddrReg1(uint8_t value);

    uint8_t getVramAddrReg2();
    void setVramAddrReg2(uint8_t value);

    uint8_t getVramIOReg();
    void setVramIOReg(uint8_t value);

private:
    uint8_t controlReg1;
    uint8_t controlReg2;
    uint8_t statusReg;
    uint8_t sramAddrReg;
    uint8_t sramIOReg;
    uint8_t vramAddrReg1;
    uint8_t vramAddrReg2;
    uint8_t vramIOReg;
};

#endif // PPU_H
