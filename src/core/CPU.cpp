#include <stdexcept>
#include "core/CPU.hpp"
#include "core/Utils.hpp"

using namespace core;

CPU::CPU()
{
    pc = sp = a = x = y = p = 0;
}

bool CPU::getFlag(Flag flag)
{
    return p & (flag != 0);
}

void CPU::setFlag(Flag flag, bool value)
{
    if(value) {
        p |= flag;
    } else {
        p &= !flag;
    }
}

void CPU::setFlagsZeroNegative(Byte value)
{
    setFlag(Z, value == 0);
    setFlag(N, (value & 1 << 7) != 0);
}

void CPU::setFlagsCarryOverflow(Byte m, Byte n, Word value)
{
    setFlag(C, value > 0xFF);
    setFlag(V, (m ^ value) & (n ^ value) & (0x80 != 0));
}

Byte CPU::carry()
{
    return p & C;
}

Byte CPU::nextByte()
{
    Byte data = pc;
    pc++;
    return bus.readByte(data);
}

Word CPU::nextWord()
{
    Byte data = pc;
    pc += 2;
    return bus.readByte(data);
}

Word CPU::getAddr(Mode mode)
{
    switch (mode)
    {
        case ZP0:
            return nextByte();
        case ZPX:
            bus.tick();
            return Utils::lowByte(Utils::offset(nextByte(), x));
        case ZPY:
            bus.tick();
            return Utils::lowByte(Utils::offset(nextByte(), y));
        case ABS:
            return nextWord();
        case ABX:
        {
            Word data = nextWord();
            if (Utils::cross(data, x)) bus.tick();
            
            return Utils::offset(data, x);
        }
        case ABY:
        {
            Word data = nextWord();
            if (Utils::cross(data, y)) bus.tick();

            return Utils::offset(data, y);
        }
        case IND:
        {
            Word i = nextWord();
            
            return bus.readNoncontinuousWord(i, Utils::highByte(i) | Utils::lowByte(i + 1));

        }
        case IZX:
        {
            bus.tick();
            Word i = Utils::offset(nextByte(), x);
            return bus.readNoncontinuousWord(i, Utils::lowByte(i) | Utils::lowByte(i + 1));;
        }
        case IZY:
        {
            Word i = nextWord();
            Word data = bus.readNoncontinuousWord(i, Utils::lowByte(i + 1));

            if (Utils::cross(data, y)) bus.tick();

            return Utils::offset(data, y);
        }
        case IMM:
        {
            Byte i = pc;
            pc++;
            return i;
        }
        default:
            throw std::runtime_error("No addressing mode");
    }
}

void CPU::runNextInstruction()
{
    runInstruction(nextByte());
}

void CPU::runInstruction(Byte opcode)
{
    switch (opcode)
    {

    // Load Accumulator
    case 0xa9:
        return LDA(IMM);
    case 0xa5:
        return LDA(ZP0);
    case 0xb5:
        return LDA(ZPX);
    case 0xad:
        return LDA(ABS);
    case 0xbd:
        return LDA(ABX);
    case 0xb9:
        return LDA(ABY);
    case 0xa1:
        return LDA(IZX);
    case 0xb1:
        return LDA(IZY);

    // Load X Register
    case 0xa2:
        return LDX(IMM);
    case 0xa6:
        return LDX(ZP0);
    case 0xb6:
        return LDX(ZPY);
    case 0xae:
        return LDX(ABS);
    case 0xbe:
        return LDX(ABY);

    // Load Y Register
    case 0xa0:
        return LDY(IMM);
    case 0xa4:
        return LDY(ZP0);
    case 0xb4:
        return LDY(ZPX);
    case 0xac:
        return LDY(ABS);
    case 0xbc:
        return LDY(ABX);
        
    default:
        pc++;
    }
}

void CPU::LDA(Mode mode)
{
    Byte data = getAddr(mode);
    setFlagsZeroNegative(data);
    a = data;
}

void CPU::LDX(Mode mode)
{
    Byte data = getAddr(mode);
    setFlagsZeroNegative(data);
    x = data;
}

void CPU::LDY(Mode mode)
{
    Byte data = getAddr(mode);
    setFlagsZeroNegative(data);
    y = data;
}

void CPU::STA(Mode mode)
{
    Byte data = getAddr(mode);
    bus.writeByte(data, a);
}

void CPU::STX(Mode mode)
{
    Byte data = getAddr(mode);
    bus.writeByte(data, x);
}

void CPU::STY(Mode mode)
{
    Byte data = getAddr(mode);
    bus.writeByte(data, y);
}
