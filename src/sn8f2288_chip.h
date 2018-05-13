#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <vector>
#include <cstdint>
#include <functional>
#include <algorithm>
#include <string>
#include <thread>
#include <chrono>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif //_MSC_VER

using namespace std;

static vector<uint8_t> readFile(const char* filename)
{
    auto fp = fopen(filename, "rb");
    fseek(fp, 0, SEEK_END);
    auto size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    vector<uint8_t> data;
    data.resize(size);
    fread(data.data(), size, 1, fp);
    return data;
}

enum OpSpaceType
{
    NUL_SPACE,
    ZRO_SPACE,
    RAM_SPACE,
    ROM_SPACE,
    IMM_SPACE
};

enum BranchType
{
    NONXT, //There is no fixed next-instruction address (RET & RETI).
    NEXTI, //Always execute next instruction (normal instructions).
    BRNCH, //Execute next instruction or the one after that (conditional branches).
    JUMPI, //Jump to some far address (unconditional branches).
    CALLI //Call some address (call instructions).
};

enum InstrType
{
    INSTR_ILLEGAL,
    INSTR_NUL_NOP,
    INSTR_ZRO_B0XCH_A_OP,
    INSTR_ZRO_B0ADD_OP_A,
    INSTR_NUL_PUSH,
    INSTR_NUL_POP,
    INSTR_IMM_CMPRS_A_OP,
    INSTR_RAM_CMPRS_A_OP,
    INSTR_RAM_RRC_OP,
    INSTR_RAM_RRCM_OP,
    INSTR_RAM_RLC_OP,
    INSTR_RAM_RLCM,
    INSTR_NUL_MOVC,
    INSTR_NUL_RET,
    INSTR_NUL_RETI,
    INSTR_RAM_ADC_A_OP,
    INSTR_RAM_ADC_OP_A,
    INSTR_RAM_ADD_A_OP,
    INSTR_RAM_ADD_OP_A,
    INSTR_IMM_ADD_A_OP,
    INSTR_RAM_INCS_OP,
    INSTR_RAM_INCMS_OP,
    INSTR_RAM_SWAP_OP,
    INSTR_RAM_OR_A_OP,
    INSTR_RAM_OR_OP_A,
    INSTR_IMM_OR_A_OP,
    INSTR_RAM_XOR_A_OP,
    INSTR_RAM_XOR_OP_A,
    INSTR_IMM_XOR_A_OP,
    INSTR_RAM_MOV_A_OP,
    INSTR_RAM_MOV_OP_A,
    INSTR_RAM_SBC_A_OP,
    INSTR_RAM_SBC_OP_A,
    INSTR_RAM_SUB_A_OP,
    INSTR_RAM_SUB_OP_A,
    INSTR_IMM_SUB_A_OP,
    INSTR_RAM_DECS_OP,
    INSTR_RAM_DECMS_OP,
    INSTR_RAM_SWAPM_OP,
    INSTR_RAM_AND_A_OP,
    INSTR_RAM_AND_OP_A,
    INSTR_IMM_AND_A_OP,
    INSTR_RAM_CLR_OP,
    INSTR_RAM_XCH_A_OP,
    INSTR_IMM_MOV_A_OP,
    INSTR_ZRO_B0MOV_A_OP,
    INSTR_RAM_B0MOV_OP_A,
    INSTR_IMM_B0MOV_R_OP,
    INSTR_IMM_B0MOV_Z_OP,
    INSTR_IMM_B0MOV_Y_OP,
    INSTR_IMM_B0MOV_PFLAG_OP,
    INSTR_IMM_B0MOV_RBANK_OP,
    INSTR_RAM_BCLR_OP,
    INSTR_RAM_BSET_OP,
    INSTR_RAM_BTS0_OP,
    INSTR_RAM_BTS1_OP,
    INSTR_ZRO_B0BCLR_OP,
    INSTR_ZRO_B0BSET_OP,
    INSTR_ZRO_B0BTS0_OP,
    INSTR_ZRO_B0BTS1_OP,
    INSTR_ROM_JMP_OP,
    INSTR_ROM_CALL_OP,
};

static const char* instrTypeToString(InstrType type)
{
    switch(type)
    {
        case INSTR_ILLEGAL: return "INSTR_ILLEGAL";
        case INSTR_NUL_NOP: return "INSTR_NUL_NOP";
        case INSTR_ZRO_B0XCH_A_OP: return "INSTR_ZRO_B0XCH_A_OP";
        case INSTR_ZRO_B0ADD_OP_A: return "INSTR_ZRO_B0ADD_OP_A";
        case INSTR_NUL_PUSH: return "INSTR_NUL_PUSH";
        case INSTR_NUL_POP: return "INSTR_NUL_POP";
        case INSTR_IMM_CMPRS_A_OP: return "INSTR_IMM_CMPRS_A_OP";
        case INSTR_RAM_CMPRS_A_OP: return "INSTR_RAM_CMPRS_A_OP";
        case INSTR_RAM_RRC_OP: return "INSTR_RAM_RRC_OP";
        case INSTR_RAM_RRCM_OP: return "INSTR_RAM_RRCM_OP";
        case INSTR_RAM_RLC_OP: return "INSTR_RAM_RLC_OP";
        case INSTR_RAM_RLCM: return "INSTR_RAM_RLCM";
        case INSTR_NUL_MOVC: return "INSTR_NUL_MOVC";
        case INSTR_NUL_RET: return "INSTR_NUL_RET";
        case INSTR_NUL_RETI: return "INSTR_NUL_RETI";
        case INSTR_RAM_ADC_A_OP: return "INSTR_RAM_ADC_A_OP";
        case INSTR_RAM_ADC_OP_A: return "INSTR_RAM_ADC_OP_A";
        case INSTR_RAM_ADD_A_OP: return "INSTR_RAM_ADD_A_OP";
        case INSTR_RAM_ADD_OP_A: return "INSTR_RAM_ADD_OP_A";
        case INSTR_IMM_ADD_A_OP: return "INSTR_IMM_ADD_A_OP";
        case INSTR_RAM_INCS_OP: return "INSTR_RAM_INCS_OP";
        case INSTR_RAM_INCMS_OP: return "INSTR_RAM_INCMS_OP";
        case INSTR_RAM_SWAP_OP: return "INSTR_RAM_SWAP_OP";
        case INSTR_RAM_OR_A_OP: return "INSTR_RAM_OR_A_OP";
        case INSTR_RAM_OR_OP_A: return "INSTR_RAM_OR_OP_A";
        case INSTR_IMM_OR_A_OP: return "INSTR_IMM_OR_A_OP";
        case INSTR_RAM_XOR_A_OP: return "INSTR_RAM_XOR_A_OP";
        case INSTR_RAM_XOR_OP_A: return "INSTR_RAM_XOR_OP_A";
        case INSTR_IMM_XOR_A_OP: return "INSTR_IMM_XOR_A_OP";
        case INSTR_RAM_MOV_A_OP: return "INSTR_RAM_MOV_A_OP";
        case INSTR_RAM_MOV_OP_A: return "INSTR_RAM_MOV_OP_A";
        case INSTR_RAM_SBC_A_OP: return "INSTR_RAM_SBC_A_OP";
        case INSTR_RAM_SBC_OP_A: return "INSTR_RAM_SBC_OP_A";
        case INSTR_RAM_SUB_A_OP: return "INSTR_RAM_SUB_A_OP";
        case INSTR_RAM_SUB_OP_A: return "INSTR_RAM_SUB_OP_A";
        case INSTR_IMM_SUB_A_OP: return "INSTR_IMM_SUB_A_OP";
        case INSTR_RAM_DECS_OP: return "INSTR_RAM_DECS_OP";
        case INSTR_RAM_DECMS_OP: return "INSTR_RAM_DECMS_OP";
        case INSTR_RAM_SWAPM_OP: return "INSTR_RAM_SWAPM_OP";
        case INSTR_RAM_AND_A_OP: return "INSTR_RAM_AND_A_OP";
        case INSTR_RAM_AND_OP_A: return "INSTR_RAM_AND_OP_A";
        case INSTR_IMM_AND_A_OP: return "INSTR_IMM_AND_A_OP";
        case INSTR_RAM_CLR_OP: return "INSTR_RAM_CLR_OP";
        case INSTR_RAM_XCH_A_OP: return "INSTR_RAM_XCH_A_OP";
        case INSTR_IMM_MOV_A_OP: return "INSTR_IMM_MOV_A_OP";
        case INSTR_ZRO_B0MOV_A_OP: return "INSTR_ZRO_B0MOV_A_OP";
        case INSTR_RAM_B0MOV_OP_A: return "INSTR_RAM_B0MOV_OP_A";
        case INSTR_IMM_B0MOV_R_OP: return "INSTR_IMM_B0MOV_R_OP";
        case INSTR_IMM_B0MOV_Z_OP: return "INSTR_IMM_B0MOV_Z_OP";
        case INSTR_IMM_B0MOV_Y_OP: return "INSTR_IMM_B0MOV_Y_OP";
        case INSTR_IMM_B0MOV_PFLAG_OP: return "INSTR_IMM_B0MOV_PFLAG_OP";
        case INSTR_IMM_B0MOV_RBANK_OP: return "INSTR_IMM_B0MOV_RBANK_OP";
        case INSTR_RAM_BCLR_OP: return "INSTR_RAM_BCLR_OP";
        case INSTR_RAM_BSET_OP: return "INSTR_RAM_BSET_OP";
        case INSTR_RAM_BTS0_OP: return "INSTR_RAM_BTS0_OP";
        case INSTR_RAM_BTS1_OP: return "INSTR_RAM_BTS1_OP";
        case INSTR_ZRO_B0BCLR_OP: return "INSTR_ZRO_B0BCLR_OP";
        case INSTR_ZRO_B0BSET_OP: return "INSTR_ZRO_B0BSET_OP";
        case INSTR_ZRO_B0BTS0_OP: return "INSTR_ZRO_B0BTS0_OP";
        case INSTR_ZRO_B0BTS1_OP: return "INSTR_ZRO_B0BTS1_OP";
        case INSTR_ROM_JMP_OP: return "INSTR_ROM_JMP_OP";
        case INSTR_ROM_CALL_OP: return "INSTR_ROM_CALL_OP";
        default: return "default";
    }
}

struct InstrEntry
{
    uint16_t mask;
    OpSpaceType opSpace;
    BranchType branchType;
    const char* mnemonic;
    const char* operands;
    InstrType type;

    InstrEntry(uint16_t mask, OpSpaceType opSpace, BranchType branchType, const char* mnemonic, const char* operands, InstrType type)
            : mask(mask), opSpace(opSpace), branchType(branchType), mnemonic(mnemonic), operands(operands), type(type) { }

    InstrEntry() : InstrEntry(0, NUL_SPACE, NONXT, nullptr, nullptr, INSTR_ILLEGAL) { }
};

static InstrEntry entries[256];

static void initializeInstructions()
{
    for(size_t i = 0; i < 256; i++)
        entries[i] = InstrEntry();

    entries[0x00] = InstrEntry(0x0000, NUL_SPACE, NEXTI, "NOP", nullptr, INSTR_NUL_NOP);
    entries[0x02] = InstrEntry(0x00ff, ZRO_SPACE, NEXTI, "B0XCH", "A, %s", INSTR_ZRO_B0XCH_A_OP);
    entries[0x03] = InstrEntry(0x00ff, ZRO_SPACE, NEXTI, "B0ADD", "%s, A", INSTR_ZRO_B0ADD_OP_A);
    entries[0x04] = InstrEntry(0x0000, NUL_SPACE, NEXTI, "PUSH", nullptr, INSTR_NUL_PUSH);
    entries[0x05] = InstrEntry(0x0000, NUL_SPACE, NEXTI, "POP", nullptr, INSTR_NUL_POP);
    entries[0x06] = InstrEntry(0x00ff, IMM_SPACE, BRNCH, "CMPRS", "A, %s", INSTR_IMM_CMPRS_A_OP);
    entries[0x07] = InstrEntry(0x00ff, RAM_SPACE, BRNCH, "CMPRS", "A, %s", INSTR_RAM_CMPRS_A_OP);
    entries[0x08] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "RRC", "%s", INSTR_RAM_RRC_OP);
    entries[0x09] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "RRCM", "%s", INSTR_RAM_RRCM_OP);
    entries[0x0a] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "RLC", "%s", INSTR_RAM_RLC_OP);
    entries[0x0b] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "RLCM", "%s", INSTR_RAM_RLCM);
    entries[0x0d] = InstrEntry(0x0000, NUL_SPACE, NEXTI, "MOVC", nullptr, INSTR_NUL_MOVC);
    entries[0x0e] = InstrEntry(0x0000, NUL_SPACE, NONXT, "RET", nullptr, INSTR_NUL_RET);
    entries[0x0f] = InstrEntry(0x0000, NUL_SPACE, NONXT, "RETI", nullptr, INSTR_NUL_RETI);
    entries[0x10] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "ADC", "A, %s", INSTR_RAM_ADC_A_OP);
    entries[0x11] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "ADC", "%s, A", INSTR_RAM_ADC_OP_A);
    entries[0x12] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "ADD", "A, %s", INSTR_RAM_ADD_A_OP);
    entries[0x13] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "ADD", "%s, A", INSTR_RAM_ADD_OP_A);
    entries[0x14] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "ADD", "A, %s", INSTR_IMM_ADD_A_OP);
    entries[0x15] = InstrEntry(0x00ff, RAM_SPACE, BRNCH, "INCS", "%s", INSTR_RAM_INCS_OP);
    entries[0x16] = InstrEntry(0x00ff, RAM_SPACE, BRNCH, "INCMS", "%s", INSTR_RAM_INCMS_OP);
    entries[0x17] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "SWAP", "%s", INSTR_RAM_SWAP_OP);
    entries[0x18] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "OR", "A, %s", INSTR_RAM_OR_A_OP);
    entries[0x19] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "OR", "%s, A", INSTR_RAM_OR_OP_A);
    entries[0x1a] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "OR", "A, %s", INSTR_IMM_OR_A_OP);
    entries[0x1b] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "XOR", "A, %s", INSTR_RAM_XOR_A_OP);
    entries[0x1c] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "XOR", "%s, A", INSTR_RAM_XOR_OP_A);
    entries[0x1d] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "XOR", "A, %s", INSTR_IMM_XOR_A_OP);
    entries[0x1e] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "MOV", "A, %s", INSTR_RAM_MOV_A_OP);
    entries[0x1f] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "MOV", "%s, A", INSTR_RAM_MOV_OP_A);
    entries[0x20] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "SBC", "A, %s", INSTR_RAM_SBC_A_OP);
    entries[0x21] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "SBC", "%s, A", INSTR_RAM_SBC_OP_A);
    entries[0x22] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "SUB", "A, %s", INSTR_RAM_SUB_A_OP);
    entries[0x23] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "SUB", "%s, A", INSTR_RAM_SUB_OP_A);
    entries[0x24] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "SUB", "A, %s", INSTR_IMM_SUB_A_OP);
    entries[0x25] = InstrEntry(0x00ff, RAM_SPACE, BRNCH, "DECS", "%s", INSTR_RAM_DECS_OP);
    entries[0x26] = InstrEntry(0x00ff, RAM_SPACE, BRNCH, "DECMS", "%s", INSTR_RAM_DECMS_OP);
    entries[0x27] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "SWAPM", "%s", INSTR_RAM_SWAPM_OP);
    entries[0x28] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "AND", "A, %s", INSTR_RAM_AND_A_OP);
    entries[0x29] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "AND", "%s, A", INSTR_RAM_AND_OP_A);
    entries[0x2a] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "AND", "A, %s", INSTR_IMM_AND_A_OP);
    entries[0x2b] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "CLR", "%s", INSTR_RAM_CLR_OP);
    entries[0x2c] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "XCH", "A, %s", INSTR_RAM_XCH_A_OP);
    entries[0x2d] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "MOV", "A, %s", INSTR_IMM_MOV_A_OP);
    entries[0x2e] = InstrEntry(0x00ff, ZRO_SPACE, NEXTI, "B0MOV", "A, %s", INSTR_ZRO_B0MOV_A_OP);
    entries[0x2f] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "B0MOV", "%s, A", INSTR_RAM_B0MOV_OP_A);
    entries[0x32] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "B0MOV", "R, %s", INSTR_IMM_B0MOV_R_OP);
    entries[0x33] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "B0MOV", "Z, %s", INSTR_IMM_B0MOV_Z_OP);
    entries[0x34] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "B0MOV", "Y, %s", INSTR_IMM_B0MOV_Y_OP);
    entries[0x35] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "B0MOV", "PFLAG, %s", INSTR_IMM_B0MOV_PFLAG_OP);
    entries[0x37] = InstrEntry(0x00ff, IMM_SPACE, NEXTI, "B0MOV", "RBANK, %s", INSTR_IMM_B0MOV_RBANK_OP);
    entries[0x40] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "BCLR", "%s", INSTR_RAM_BCLR_OP);
    entries[0x48] = InstrEntry(0x00ff, RAM_SPACE, NEXTI, "BSET", "%s", INSTR_RAM_BSET_OP);
    entries[0x50] = InstrEntry(0x00ff, RAM_SPACE, BRNCH, "BTS0", "%s", INSTR_RAM_BTS0_OP);
    entries[0x58] = InstrEntry(0x00ff, RAM_SPACE, BRNCH, "BTS1", "%s", INSTR_RAM_BTS1_OP);
    entries[0x60] = InstrEntry(0x00ff, ZRO_SPACE, NEXTI, "B0BCLR", "%s", INSTR_ZRO_B0BCLR_OP);
    entries[0x68] = InstrEntry(0x00ff, ZRO_SPACE, NEXTI, "B0BSET", "%s", INSTR_ZRO_B0BSET_OP);
    entries[0x70] = InstrEntry(0x00ff, ZRO_SPACE, BRNCH, "B0BTS0", "%s", INSTR_ZRO_B0BTS0_OP);
    entries[0x78] = InstrEntry(0x00ff, ZRO_SPACE, BRNCH, "B0BTS1", "%s", INSTR_ZRO_B0BTS1_OP);
    entries[0x80] = InstrEntry(0x3fff, ROM_SPACE, JUMPI, "JMP", "%s", INSTR_ROM_JMP_OP);
    entries[0xc0] = InstrEntry(0x3fff, ROM_SPACE, CALLI, "CALL", "%s", INSTR_ROM_CALL_OP);
}

static string disassemble(uint16_t address, uint16_t instruction)
{
    auto bincode = uint8_t(instruction >> 8);
    uint8_t opcode_key = 0;
    bool is_bit = false;
    if(bincode >= 0x80)
    {
        opcode_key = uint8_t(bincode & 0xc0);
        is_bit = false;
    }
    else if(bincode >= 0x40)
    {
        opcode_key = uint8_t(bincode & 0xf8);
        is_bit = true;
    }
    else
    {
        opcode_key = bincode;
        is_bit = false;
    }

    char opcode[256] = "";
    auto entry = entries[opcode_key];
    if(entry.mnemonic == nullptr)
    {
        //TODO: branch
        sprintf(opcode, "DW 0x%02x ; ILLEGAL OPCODE", instruction);
        return string(opcode);
    }
    auto mask = entry.mask;
    auto opspace = entry.opSpace;
    auto jump_action = entry.branchType;
    auto mnemonic = entry.mnemonic;
    auto caption = entry.operands;

    strcat(opcode, mnemonic);

    if(opspace == NUL_SPACE)
    {
        //TODO: branch
    }
    else
    {
        char operand_fmt[256] = "";
        auto operand = instruction & mask;
        //TODO: branch
        if(opspace == ROM_SPACE)
        {
            strcpy(operand_fmt, "0x%04x");
            //assert not is_bit
        }
        else if(opspace == IMM_SPACE)
        {
            strcpy(operand_fmt, "#0x%02x");
            //assert not is_bit
        }
        else // ZRO & RAM
        {
            strcpy(operand_fmt, "0x%02x");
            if(is_bit)
            {
                auto bit_address = bincode & 0x7;
                sprintf(operand_fmt + strlen(operand_fmt), ".%i", bit_address);
            }
        }

        char symbol[256] = "";
        sprintf(symbol, operand_fmt, operand);
        strcat(opcode, " ");
        sprintf(opcode + strlen(opcode), caption, symbol);
    }
    return string(opcode);
}

enum LogType
{
    LogNone,
    LogAcc,
    LogRam,
    LogRom,
    LogFifo
};

class LogEntry
{
    LogEntry(LogType type, uint16_t addr, bool write, uint16_t oldValue, uint16_t newValue, bool bit = false, uint8_t bitpos = 0)
            : type(type), addr(addr), write(write), oldValue(oldValue), newValue(newValue), bit(bit), bitpos(bitpos) { }

public:
    LogType type = LogNone;
    uint16_t addr = 0;
    bool write = false;
    uint16_t oldValue = 0;
    uint16_t newValue = 0;
    bool bit = false;
    uint8_t bitpos = 0;

    LogEntry() : LogEntry(LogNone, 0, false, 0, 0) { }

    static LogEntry ReadAcc(uint16_t value)
    {
        return {LogAcc, 0, false, value, 0};
    }

    static LogEntry WriteAcc(uint16_t oldValue, uint16_t newValue)
    {
        return {LogAcc, 0, true, oldValue, newValue};
    }

    static LogEntry ReadRam(uint16_t addr, uint16_t value)
    {
        return {LogRam, addr, false, value, 0};
    }

    static LogEntry ReadBitRam(uint16_t addr, uint8_t bitpos, bool value)
    {
        return {LogRam, addr, false, value, 0, true, bitpos};
    }

    static LogEntry WriteRam(uint16_t addr, uint16_t oldValue, uint16_t newValue)
    {
        return {LogRam, addr, true, oldValue, newValue};
    }

    static LogEntry WriteBitRam(uint16_t addr, uint8_t bitpos, bool oldValue, bool newValue)
    {
        return {LogRam, addr, true, oldValue, newValue, true, bitpos};
    }

    static LogEntry ReadRom(uint16_t addr, uint16_t value)
    {
        return {LogRom, addr, false, value, 0};
    }

    static LogEntry WriteRom(uint16_t addr, uint16_t oldValue, uint16_t newValue)
    {
        return {LogRom, addr, true, oldValue, newValue};
    }

    static LogEntry ReadBuffer(LogType type, uint16_t addr, uint16_t value)
    {
        return {type, addr, false, value, 0};
    }

    static LogEntry ReadBufferBit(LogType type, uint16_t addr, uint8_t bitindex, uint16_t value)
    {
        return {type, addr, false, value, 0, true, bitindex};
    }

    static LogEntry WriteBuffer(LogType type, uint16_t addr, uint16_t oldValue, uint16_t newValue)
    {
        return {type, addr, true, oldValue, newValue};
    }

    static LogEntry WriteBufferBit(LogType type, uint16_t addr, uint8_t bitindex, uint16_t oldValue, uint16_t newValue)
    {
        return {type, addr, true, oldValue, newValue, true, bitindex};
    }

    void print(bool newline = true) const
    {
        printf(write ? "write" : "read");
        printf(" ");
        printf(bit ? "bit" : (type == LogRom ? "word" : "byte"));
        printf(" ");
        switch(type)
        {
            case LogNone:
                printf("INVALID");
                break;
            case LogAcc:
                printf("ACC");
                break;
            case LogRam:
                printf("RAM 0x%02x", addr);
                break;
            case LogRom:
                printf("ROM 0x%04x", addr);
                break;
            case LogFifo:
                printf("FIFO 0x%02x", addr);
                break;
        }
        if(write)
        {
            if(type == LogRom)
                printf(" (old: 0x%04x, new: 0x%04x)", oldValue, newValue);
            else
                printf(" (old: 0x%02x, new: 0x%02x)", oldValue, newValue);
        }
        else
        {
            if(type == LogRom)
                printf(" (value: 0x%04x)", oldValue);
            else
                printf(" (value: 0x%02x)", oldValue);
        }
        if(newline)
            puts("");
    }
};

template<class T>
struct ArrayFunctions
{
    using ReadFunc = function<T(uint16_t)>;
    using WriteFunc = function<void(uint16_t, T)>;

    ReadFunc read;
    WriteFunc write;

    T getBit(uint16_t index, uint8_t bitpos)
    {
        auto value = read(index);
        return value >> bitpos & 1;
    }

    void setBit(uint16_t index, uint8_t bitpos, T value)
    {
        auto mask = uint8_t(1 << bitpos);
        value <<= bitpos;
        value &= mask;
        auto newValue = read(index);
        newValue &= mask;
        newValue |= value;
        write(index, newValue);
    }

    ArrayFunctions(ReadFunc read, WriteFunc write) : read(read), write(write) { }
};

template<class T, size_t Count>
class BaseArray
{
    T data[Count];

public:
    BaseArray()
    {
        memset(data, 0, sizeof(data));
    }

    virtual T get(uint16_t index)
    {
        return data[index];
    }

    virtual void set(uint16_t index, T value)
    {
        data[index] = value;
    }
};

template<class T, size_t Count>
struct BitBaseArray : public BaseArray<T, Count>
{
    virtual T getBit(uint16_t index, uint8_t bitpos)
    {
        auto value = BaseArray<T, Count>::get(index);
        return value >> bitpos & 1;
    }

    virtual void setBit(uint16_t index, uint8_t bitpos, T value)
    {
        auto mask = uint8_t(1 << bitpos);
        value <<= bitpos;
        value &= mask;
        auto newValue = BaseArray<T, Count>::get(index);
        newValue &= mask;
        newValue |= value;
        BaseArray<T, Count>::set(index, newValue);
    }
};

template<class T, size_t Count, LogType Type>
struct LogBuffer : public BaseArray<T, Count>
{
    vector<LogEntry>* log;

    explicit LogBuffer(vector<LogEntry>* log) : log(log)  { }

    T getNoLog(uint16_t index)
    {
        return BaseArray<T, Count>::get(index);
    }

    void setNoLog(uint16_t index, T value)
    {
        return BaseArray<T, Count>::set(index, value);
    }

    T get(uint16_t index) override
    {
        auto value = getNoLog(index);
        log->push_back(LogEntry::ReadBuffer(Type, index, value));
        return value;
    }

    void set(uint16_t index, T value) override
    {
        auto old = getNoLog(index);
        log->push_back(LogEntry::WriteBuffer(Type, index, old, value));
        setNoLog(index, value);
    }

    bool getBitNoLog(uint16_t index, uint8_t bitindex)
    {
        auto value = getNoLog(index);
        return (value >> bitindex) & 1;
    }

    bool getBit(uint16_t index, uint8_t bitindex)
    {
        auto value = getBitNoLog(index, bitindex);
        log->push_back(LogEntry::ReadBufferBit(Type, index, bitindex, value));
        return value;
    }

    void setBitNoLog(uint16_t index, uint8_t bitindex, uint8_t value)
    {
        auto mask = uint8_t(1 << bitindex);
        value <<= bitindex;
        value &= mask;
        auto newValue = getNoLog(index);
        newValue &= mask;
        newValue |= value;
        setNoLog(index, newValue);
    }

    void setBit(uint16_t index, uint8_t bitindex, uint8_t value)
    {
        auto oldValue = getBitNoLog(index, bitindex);
        log->push_back(LogEntry::WriteBufferBit(Type, index, bitindex, oldValue, value));
        setBitNoLog(index, bitindex, value);
    }

    static const size_t count = Count;
};

template<uint16_t ByteIndex, uint8_t BitIndex = 0, uint8_t SizeInBits = 8>
class Register
{
    ArrayFunctions<uint8_t> & ram;

public:
    explicit Register(ArrayFunctions<uint8_t> & ram) : ram(ram) { }

    uint16_t get(ArrayFunctions<uint8_t> & ram) const
    {
        uint16_t result = 0;
        if(BitIndex + SizeInBits > 8)
            result = ram.read(ByteIndex) | ram.read(ByteIndex + 1) << 8;
        else
            result = ram.read(ByteIndex);
        result >>= BitIndex;
        auto mask = uint16_t(~uint16_t(int16_t(0x8000) >> (15 - SizeInBits)));
        result &= mask;
        return result;
    }

    uint16_t get() const
    {
        return get(ram);
    }

    void set(ArrayFunctions<uint8_t> & ram, uint16_t value)
    {
        uint16_t result = 0;
        if(BitIndex + SizeInBits > 8)
            result = ram.read(ByteIndex) | ram.read(ByteIndex + 1) << 8;
        else
            result = ram.read(ByteIndex);
        auto mask = uint16_t(~uint16_t(int16_t(0x8000) >> (15 - SizeInBits)));
        value &= mask;
        value <<= BitIndex;
        mask = ~(mask << BitIndex);
        result &= mask;
        result |= value;
        ram.write(ByteIndex, uint8_t(result & 0xFF));
        if(BitIndex + SizeInBits > 8)
            ram.write(ByteIndex + 1, uint8_t(result >> 8));
    }

    void set(uint16_t value)
    {
        set(ram, value);
    }

    const static uint16_t index = ByteIndex;
};

enum ResetType
{
    ResetPowerOn,
    ResetWatchdog,
    ResetExternal
};

struct SN8F2288
{
    SN8F2288()
        : rom(&log), ram([this](uint16_t index) { return ramRead(index); }, [this](uint16_t index, uint8_t value) { ramWrite(index, value); }), fifo(&log),
          usb(fifo),
          ACC(0),
          R(ram), Z(ram), Y(ram), PFLAG(ram), RBANK(ram),
          TC0M(ram), TC0C(ram), TC0R(ram),
          TC1M(ram), TC1C(ram), TC1R(ram),
          TC2M(ram), TC2C(ram), TC2R(ram),
          UDA(ram), USTATUS(ram), EP0OUT_CNT(ram), USB_INT_EN(ram), EP_ACK(ram), EP_NAK(ram), UE0R(ram), UE1R(ram), UE1R_C(ram), UE2R(ram), UE2R_C(ram), UE3R(ram), UR3R_C(ram), UE4R(ram), UE4R_C(ram), EP2FIFO_ADDR(ram), EP3FIFO_ADDR(ram), EP4FIFO_ADDR(ram), UDP0(ram), UDR0_R(ram), UDR0_W(ram), UPID(ram), UTOGGLE(ram),
          URTX(ram), URRX(ram), URBRC(ram), URTXD1(ram), URTXD2(ram), URRXD1(ram), URRXD2(ram),
          SIOM(ram), SIOR(ram), SIOB(ram), P0M(ram), ADM(ram), ADB(ram), ADR(ram), P4CON(ram), PECMD(ram), PEROM_L(ram), PEROM_H(ram), PERAM_L(ram), PERAM_CNT(ram), PEDGE(ram), P1W(ram), P1M(ram), P2M(ram), P4M(ram), P5M(ram), INTRQ1(ram), INTEN1(ram), INTRQ(ram), INTEN(ram), OSCM(ram), WDTR(ram), PCL(ram), PCH(ram), P0(ram), P1(ram), P2(ram), P4(ram), P5(ram), T0M(ram), T0C(ram), T1M(ram), T1CL(ram), T1CH(ram), STKP(ram), P0UR(ram), P1UR(ram), P2UR(ram), P4UR(ram), P5UR(ram), AT_YZ(ram), P1OC(ram), MSPSTAT(ram), MSPM1(ram), MSPM2(ram), MSPBUF(ram), MSPADR(ram), STK7L(ram), STK7H(ram), STK6L(ram), STK6H(ram), STK5L(ram), STK5H(ram), STK4L(ram), STK4H(ram), STK3L(ram), STK3H(ram), STK2L(ram), STK2H(ram), STK1L(ram), STK1H(ram), STK0L(ram), STK0H(ram),
          PC(ram), YZ(ram),
          GIE(ram), STK7(ram), STK6(ram), STK5(ram), STK4(ram), STK3(ram), STK2(ram), STK1(ram), STK0(ram),
          FZ(ram), FDC(ram), FC(ram), FNPD(ram), FNT0(ram) { }

    vector<LogEntry> log;
    //12K words ROM (page 16)
    LogBuffer<uint16_t, 0x3000, LogRom> rom;
    //512 X 8-bit RAM (page 27), actually 640 bytes?
    uint8_t ramData[0x280];
    ArrayFunctions<uint8_t> ram;
    //136 x 8-bit RAM for USB DATA FIFO
    LogBuffer<uint8_t, 136, LogFifo> fifo;
    uint8_t ACC; //not in RAM (TODO: logging)

    Register<0x82> R;
    Register<0x83> Z;
    Register<0x84> Y;
    Register<0x86> PFLAG;
    Register<0x87, 0, 2> RBANK;
    Register<0x88> TC0M;
    Register<0x89> TC0C;
    Register<0x8A> TC0R; //W
    Register<0x8B> TC1M;
    Register<0x8C> TC1C;
    Register<0x8D> TC1R; //W
    Register<0x8E> TC2M;
    Register<0x8F> TC2C;
    Register<0x90> TC2R; //W
    //USB
    Register<0x91> UDA;
    Register<0x92> USTATUS;
    Register<0x93> EP0OUT_CNT;
    Register<0x94> USB_INT_EN;
    Register<0x95> EP_ACK;
    Register<0x96> EP_NAK;
    Register<0x97> UE0R;
    Register<0x98> UE1R;
    Register<0x99> UE1R_C;
    Register<0x9A> UE2R;
    Register<0x9B> UE2R_C;
    Register<0x9C> UE3R;
    Register<0x9D> UR3R_C;
    Register<0x9E> UE4R;
    Register<0x9F> UE4R_C;
    Register<0xA0> EP2FIFO_ADDR;
    Register<0xA1> EP3FIFO_ADDR;
    Register<0xA2> EP4FIFO_ADDR;
    Register<0xA3> UDP0;
    Register<0xA5> UDR0_R;
    Register<0xA6> UDR0_W;
    Register<0xA7> UPID;
    Register<0xA8> UTOGGLE;

    Register<0xA9> URTX;
    Register<0xAA> URRX;
    Register<0xAB> URBRC;
    Register<0xAC> URTXD1;
    Register<0xAD> URTXD2;
    Register<0xAE> URRXD1; //R
    Register<0xAF> URRXD2; //R
    Register<0xB0> SIOM;
    Register<0xB1> SIOR; //W
    Register<0xB2> SIOB;
    Register<0xB5> P0M;
    Register<0xB6> ADM;
    Register<0xB7> ADB; //R
    Register<0xB8> ADR;
    Register<0xB9> P4CON; //W
    Register<0xBA> PECMD; //W
    Register<0xBB> PEROM_L;
    Register<0xBC> PEROM_H;
    Register<0xBD> PERAM_L;
    Register<0xBE> PERAM_CNT;
    Register<0xBF> PEDGE;
    Register<0xC0> P1W;
    Register<0xC1> P1M;
    Register<0xC2> P2M;
    Register<0xC4> P4M;
    Register<0xC5> P5M;
    Register<0xC6> INTRQ1;
    Register<0xC7> INTEN1;
    Register<0xC8> INTRQ;
    Register<0xC9> INTEN;
    Register<0xCA> OSCM;
    Register<0xCC> WDTR; //W
    Register<0xCE> PCL;
    Register<0xCF> PCH;
    Register<0xD0> P0;
    Register<0xD1> P1;
    Register<0xD2> P2;
    Register<0xD4> P4;
    Register<0xD5> P5;
    Register<0xD8> T0M;
    Register<0xD9> T0C;
    Register<0xDA> T1M;
    Register<0xDB> T1CL;
    Register<0xDC> T1CH;
    Register<0xDF, 0, 3> STKP;
    Register<0xE0> P0UR; //W
    Register<0xE1> P1UR; //W
    Register<0xE2> P2UR; //W
    Register<0xE4> P4UR; //W
    Register<0xE5> P5UR; //W
    Register<0xE7> AT_YZ; //TODO
    Register<0xE9> P1OC; //W
    Register<0xEA> MSPSTAT; //R
    Register<0xEB> MSPM1;
    Register<0xEC> MSPM2;
    Register<0xED> MSPBUF;
    Register<0xEE> MSPADR;
    Register<0xF0> STK7L;
    Register<0xF1> STK7H;
    Register<0xF2> STK6L;
    Register<0xF3> STK6H;
    Register<0xF4> STK5L;
    Register<0xF5> STK5H;
    Register<0xF6> STK4L;
    Register<0xF7> STK4H;
    Register<0xF8> STK3L;
    Register<0xF9> STK3H;
    Register<0xFA> STK2L;
    Register<0xFB> STK2H;
    Register<0xFC> STK1L;
    Register<0xFD> STK1H;
    Register<0xFE> STK0L;
    Register<0xFF> STK0H;

    //program counter
    Register<0xCE, 0, 14> PC;
    Register<0x83, 0, 14> YZ;
    //stack pointer
    Register<0xDF, 7, 1> GIE;
    Register<0xF0, 0, 14> STK7;
    Register<0xF2, 0, 14> STK6;
    Register<0xF4, 0, 14> STK5;
    Register<0xF6, 0, 14> STK4;
    Register<0xF8, 0, 14> STK3;
    Register<0xFA, 0, 14> STK2;
    Register<0xFC, 0, 14> STK1;
    Register<0xFE, 0, 14> STK0;
    //flags
    Register<0x86, 0, 1> FZ;
    Register<0x86, 1, 1> FDC;
    Register<0x86, 2, 1> FC;
    Register<0x86, 6, 1> FNPD;
    Register<0x86, 7, 1> FNT0;

    struct UsbData
    {
        BaseArray<uint8_t, 136> & fifo;

        UsbData(BaseArray<uint8_t, 136> & fifo) : fifo(fifo) { }

#define GETBIT(value, n) (((value >> n) & 1) != 0)

        bool UDE = false;
        uint8_t UDA = 0;
        bool CRCERR = false;
        bool PKTERR = false;
        bool SOF = false;
        bool BUS_RST = false;
        bool SUSPEND = false;
        bool EP0SETUP = false;
        bool EP0IN = false;
        bool EP0OUT = false;

        bool EP_ACK[4] = { false, false, false, false };
        bool EP_NAK[4] = { false, false, false, false };

        bool REG_EN = true;
        bool DP_UP_EN = false;
        bool SOF_INT_EN = false;
        bool EP_NAK_INT_EN[4] = { false, false, false, false };

        enum EndpointMode
        {
            MODE_NAK = 0,
            MODE_ACK,
            MODE_STALL1,
            MODE_STALL2
        };

        EndpointMode UE0M = MODE_NAK;
        uint8_t UE0C = 0;
        bool UE1E = false;
        EndpointMode UE1M = MODE_NAK;
        uint8_t UE1C = 0;
        bool UE2E = false;
        EndpointMode UE2M = MODE_NAK;
        uint8_t UE2C = 0;

        uint8_t UDP0 = 0;

        uint8_t EP0OUT_CNT = 0;
        uint8_t EP2FIFO_ADDR = 0;

        bool UBDE = false;
        bool DDP = false;
        bool DDN = false;


        void setUda(uint8_t uda)
        {
            UDE = (uda >> 7) != 0;
            UDA = uint8_t(uda & 0x7f);
            printf("set UDA to 0x%02x\n", uda);
        }

        uint8_t getUda() const
        {
            printf("get UDA\n");
            return uint8_t((UDE << 7) | (UDA & 0x7f));
        }

        void setUstatus(uint8_t ustatus)
        {
            CRCERR = GETBIT(ustatus, 7);
            PKTERR = GETBIT(ustatus, 6);
            SOF = GETBIT(ustatus, 5);
            BUS_RST = GETBIT(ustatus, 4);
            SUSPEND = GETBIT(ustatus, 3);
            EP0SETUP = GETBIT(ustatus, 2);
            EP0IN = GETBIT(ustatus, 1);
            EP0OUT = GETBIT(ustatus, 0);
            printf("set USTATUS to 0x%02x\n", ustatus);
        }

        uint8_t getUstatus() //const
        {
            printf("get USTATUS\n");
            puts("sleeping 200ms...");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            SUSPEND = !SUSPEND; //TODO: figure out how this actually works
            /*static int count = 0;
            if(count++ > 3)
            {
                SUSPEND = !SUSPEND;
                count = 0;
                throw exception();
            }*/
            return uint8_t((CRCERR << 7) | (PKTERR << 6) | (SOF << 5) | (BUS_RST << 4) | (SUSPEND << 3) | (EP0SETUP << 2) | (EP0IN << 1) | (EP0OUT << 0));
        }

        void setEp_ack(uint8_t ep_ack)
        {
            EP_ACK[0] = GETBIT(ep_ack, 0);
            EP_ACK[1] = GETBIT(ep_ack, 1);
            EP_ACK[2] = GETBIT(ep_ack, 2);
            EP_ACK[3] = GETBIT(ep_ack, 3);
            printf("set EP_ACK to 0x%02x\n", ep_ack);
        }

        uint8_t getEp_ack() const
        {
            printf("get EP_ACK\n");
            return uint8_t((EP_ACK[0] << 0) | (EP_ACK[1] << 1) | (EP_ACK[2] << 2) | (EP_ACK[3] << 3));
        }

        void setUe0r(uint8_t ue0r)
        {
            printf("set UE0R to 0x%02x\n", ue0r);
            UE0M = EndpointMode((ue0r >> 5) & 0x3);
            UE0C = uint8_t(ue0r & 0xf);
        }

        uint8_t getUe0r() const
        {
            printf("get UE0R\n");
            return uint8_t((UE0M << 5) | (UE0C << 0));
        }

        void setUe1r(uint8_t ue1r)
        {
            printf("set UE1R to 0x%02x\n", ue1r);
            UE1E = GETBIT(ue1r, 7);
            UE1M = EndpointMode((ue1r >> 5) & 0x3);
        }

        uint8_t getUe1r() const
        {
            printf("get UE1R\n");
            return uint8_t((UE1E << 7) | (UE1M << 5));
        }

        void setUe2r(uint8_t ue2r)
        {
            printf("set UE2R to 0x%02x\n", ue2r);
            UE2E = GETBIT(ue2r, 7);
            UE2M = EndpointMode((ue2r >> 5) & 0x3);
        }

        uint8_t getUe2r() const
        {
            printf("get UE2R\n");
            return uint8_t((UE2E << 7) | (UE2M << 5));
        }

        void setUdp0(uint8_t udp0)
        {
            UDP0 = udp0;
        }

        uint8_t getUdp0() const
        {
            return UDP0;
        }

        void setUdr0_r(uint8_t udr0_r)
        {
            printf("set UDR0_R to 0x%02x (UDP0: 0x%02x, is this a NOOP?)\n", udr0_r, UDP0);
        }

        uint8_t getUdr0_r() const
        {
            printf("get UDR0_R (UDP0: 0x%02x)\n", UDP0);
            if(UDP0 > 136)
                printf("Invalid USB FIFO access at 0x%02x\n", UDP0);
            return fifo.get(UDP0);
        }

        void setUdr0_w(uint8_t udr0_w)
        {
            printf("set UDR0_W to 0x%02x (UDP0: 0x%02x)\n", udr0_w, UDP0);
        }

        uint8_t getUdr0_w() const
        {
            printf("get UDR0_W (UDP0: 0x%02x, is this a NOOP?)\n", UDP0);
            return 0;
        }

        void setEp0out_cnt(uint8_t ep0out_cnt)
        {
            printf("set EP0OUT_CNT to 0x%02x\n", ep0out_cnt);
            EP0OUT_CNT = uint8_t(ep0out_cnt & 0x1f);
        }

        uint8_t getEp0out_cnt() const
        {
            printf("get EP0OUT_CNT\n");
            return EP0OUT_CNT;
        }

        void setUe1r_c(uint8_t ue1r_c)
        {
            printf("set UE1R_C to 0x%02x\n", ue1r_c);
            UE1C = uint8_t(ue1r_c & 0x7f);
        }

        uint8_t getUe1r_c() const
        {
            printf("get UE1R_C\n");
            return UE1C;
        }

        void setUe2r_c(uint8_t ue2r_c)
        {
            printf("set UE2R_C to 0x%02x\n", ue2r_c);
            UE2C = uint8_t(ue2r_c & 0x7f);
        }

        uint8_t getUe2r_c() const
        {
            printf("get UE2R_C\n");
            return UE1C;
        }

        void setEp2fifo_addr(uint8_t ep2fifo_addr)
        {
            printf("set EP2FIFO_ADDR to 0x%02x\n", ep2fifo_addr);
            EP2FIFO_ADDR = ep2fifo_addr;
        }

        uint8_t getEp2fifo_addr() const
        {
            printf("get EP2FIFO_ADDR\n");
            return EP2FIFO_ADDR;
        }

        void setUpid(uint8_t upid)
        {
            printf("set UPID to 0x%02x\n", upid);
            UBDE = GETBIT(upid, 2);
            DDP = GETBIT(upid, 1);
            DDN = GETBIT(upid, 0);
        }

        uint8_t getUpid() const
        {
            printf("get UPID\n");
            return uint8_t((UBDE << 2) | (DDP << 1) | (DDN << 0));
        }

        void setUsb_int_en(uint8_t usb_int_en)
        {
            printf("set USB_INT_EN to 0x%02x\n", usb_int_en);
            REG_EN = GETBIT(usb_int_en, 7);
            DP_UP_EN = GETBIT(usb_int_en, 6);
            SOF_INT_EN = GETBIT(usb_int_en, 5);
            EP_NAK_INT_EN[3] = GETBIT(usb_int_en, 3);
            EP_NAK_INT_EN[2] = GETBIT(usb_int_en, 2);
            EP_NAK_INT_EN[1] = GETBIT(usb_int_en, 1);
            EP_NAK_INT_EN[0] = GETBIT(usb_int_en, 0);
        }

        uint8_t getUsb_int_en() const
        {
            printf("get USB_INT_EN\n");
            return uint8_t((REG_EN << 7) | (DP_UP_EN << 6) | (SOF_INT_EN << 5) | (EP_NAK_INT_EN[3] << 3) | (EP_NAK_INT_EN[2] << 2) | (EP_NAK_INT_EN[1] << 1) | (EP_NAK_INT_EN[0] << 0));
        }

        void debug() const
        {
            printf("UDE: %d, UDA: 0x%02x\n", UDE, UDA);
            printf("CRCERR: %d, PKTERR: %d, SOF: %d, BUS_RST: %d\n", CRCERR, PKTERR, SOF, BUS_RST);
            printf("SUSPEND: %d, EP0SETUP: %d, EP0IN: %d, EP0OUT: %d\n", SUSPEND, EP0SETUP, EP0IN, EP0OUT);
        }
#undef GETBIT
    } usb;

#define regWhitelist() \
    case decltype(STKP)::index: \
    case decltype(PFLAG)::index: \
    case decltype(PCL)::index: \
    case decltype(PCH)::index: \
    case decltype(R)::index: \
    case decltype(Y)::index: \
    case decltype(Z)::index: \
    case decltype(RBANK)::index: \
    case decltype(STK7L)::index: \
    case decltype(STK7H)::index: \
    case decltype(STK6L)::index: \
    case decltype(STK6H)::index: \
    case decltype(STK5L)::index: \
    case decltype(STK5H)::index: \
    case decltype(STK4L)::index: \
    case decltype(STK4H)::index: \
    case decltype(STK3L)::index: \
    case decltype(STK3H)::index: \
    case decltype(STK2L)::index: \
    case decltype(STK2H)::index: \
    case decltype(STK1L)::index: \
    case decltype(STK1H)::index: \
    case decltype(STK0L)::index: \
    case decltype(STK0H)::index: \
    case decltype(PEROM_L)::index: \
    case decltype(PEROM_H)::index: \
    case decltype(PERAM_L)::index: \
    case decltype(PERAM_CNT)::index:


    uint8_t ramRead(uint16_t index)
    {
        if(index >= 0 && index <= 0x7F) //BANK 0, General purpose area
        {
            return ramData[index];
        }
        else if(index >= 0x80 && index <= 0xFF) //BANK 0, System registers
        {
            switch(index)
            {
                //USB registers
                case decltype(UDA)::index:
                    return usb.getUda();
                case decltype(USTATUS)::index:
                    return usb.getUstatus();
                case decltype(EP_ACK)::index:
                    return usb.getEp_ack();
                case decltype(UE0R)::index:
                    return usb.getUe0r();
                case decltype(UE1R)::index:
                    return usb.getUe1r();
                case decltype(UE2R)::index:
                    return usb.getUe2r();
                case decltype(UDP0)::index:
                    return usb.getUdp0();
                case decltype(UDR0_R)::index:
                    return usb.getUdr0_r();
                case decltype(UDR0_W)::index:
                    return usb.getUdr0_w();
                case decltype(EP0OUT_CNT)::index:
                    return usb.getEp0out_cnt();
                case decltype(UE1R_C)::index:
                    return usb.getUe1r_c();
                case decltype(UE2R_C)::index:
                    return usb.getUe2r_c();
                case decltype(EP2FIFO_ADDR)::index:
                    return usb.getEp2fifo_addr();
                case decltype(UPID)::index:
                    return usb.getUpid();
                case decltype(USB_INT_EN)::index:
                    return usb.getUsb_int_en();

                //TODO: interrupts
                case decltype(INTRQ1)::index:
                case decltype(INTEN1)::index:
                case decltype(INTRQ)::index:
                case decltype(INTEN)::index:
                    printf("TODO: interrupt 0x%02x read\n", index);
                    return ramData[index];

                //TODO: pins
                case decltype(P0M)::index:
                case decltype(P1M)::index:
                case decltype(P2M)::index:
                case decltype(P4M)::index:
                case decltype(P5M)::index:
                case decltype(P0UR)::index:
                case decltype(P1UR)::index:
                case decltype(P2UR)::index:
                case decltype(P4UR)::index:
                case decltype(P5UR)::index:
                case decltype(P1OC)::index:
                case decltype(P0)::index:
                case decltype(P1)::index:
                case decltype(P2)::index:
                case decltype(P4)::index:
                case decltype(P5)::index:
                case decltype(P1W)::index:
                    printf("TODO: pin 0x%02x read\n", index);
                    return ramData[index];
                case 0xc3:
                case 0xd3:
                case 0xe3:
                    printf("invalid P3-related read (doing NOOP)!\n");
                    return 0;

                //TODO: timer/counter
                case decltype(TC0M)::index:
                case decltype(TC1M)::index:
                case decltype(TC2M)::index:
                case decltype(TC0C)::index:
                case decltype(TC1C)::index:
                case decltype(TC2C)::index:
                case decltype(TC0R)::index:
                case decltype(TC1R)::index:
                case decltype(TC2R)::index:
                    printf("TODO: timer 0x%02x read\n", index);
                    return ramData[index];

                case decltype(AT_YZ)::index:
                    return ramRead(YZ.get()); //TODO: possible stack overflow

                case decltype(OSCM)::index:
                    printf("TODO: oscillator control register read\n");
                    return ramData[index];

                regWhitelist()
                    return ramData[index];

                default:
                    printf("unknown register read at 0x%04x, PC: 0x%04x\n", index, PC.get());
            }
        }
        else if(index >= 0x100 && index <= 0x1FF) //BANK 1, General purpose area
        {
            return ramData[index];
        }
        else if(index >= 0x200 && index <= 0x27F) //BANK 2, General purpose area
        {
            return ramData[index];
        }
        printf("location: 0x%04x\n", index);
        throw out_of_range("invalid ram read");
    }

    void ramWrite(uint16_t index, uint8_t value)
    {
        if(index >= 0 && index <= 0x7F) //BANK 0, General purpose area
        {
            ramData[index] = value;
            return;
        }
        else if(index >= 0x80 && index <= 0xFF) //BANK 0, System registers
        {
            switch(index)
            {
                //USB registers
                case decltype(UDA)::index:
                    usb.setUda(value);
                    return;
                case decltype(USTATUS)::index:
                    usb.setUstatus(value);
                    return;
                case decltype(EP_ACK)::index:
                    usb.setEp_ack(value);
                    return;
                case decltype(UE0R)::index:
                    usb.setUe0r(value);
                    return;
                case decltype(UE1R)::index:
                    usb.setUe1r(value);
                    return;
                case decltype(UE2R)::index:
                    usb.setUe2r(value);
                    return;
                case decltype(UDP0)::index:
                    usb.setUdp0(value);
                    return;
                case decltype(UDR0_R)::index:
                    usb.setUdr0_r(value);
                    return;
                case decltype(UDR0_W)::index:
                    usb.setUdr0_w(value);
                    return;
                case decltype(EP0OUT_CNT)::index:
                    usb.setEp0out_cnt(value);
                    return;
                case decltype(UE1R_C)::index:
                    usb.setUe1r_c(value);
                    return;
                case decltype(UE2R_C)::index:
                    usb.setUe2r_c(value);
                    return;
                case decltype(EP2FIFO_ADDR)::index:
                    usb.setEp2fifo_addr(value);
                    return;
                case decltype(UPID)::index:
                    usb.setUpid(value);
                    return;
                case decltype(USB_INT_EN)::index:
                    usb.setUsb_int_en(value);
                    return;

                //TODO: interrupts
                case decltype(INTRQ1)::index:
                case decltype(INTEN1)::index:
                case decltype(INTRQ)::index:
                case decltype(INTEN)::index:
                    printf("TODO: interrupt 0x%02x write, value: 0x%02x\n", index, value);
                    ramData[index] = value;
                    return;

                //TODO: pins
                case decltype(P0M)::index:
                case decltype(P1M)::index:
                case decltype(P2M)::index:
                case decltype(P4M)::index:
                case decltype(P5M)::index:
                case decltype(P0UR)::index:
                case decltype(P1UR)::index:
                case decltype(P2UR)::index:
                case decltype(P4UR)::index:
                case decltype(P5UR)::index:
                case decltype(P1OC)::index:
                case decltype(P0)::index:
                case decltype(P1)::index:
                case decltype(P2)::index:
                case decltype(P4)::index:
                case decltype(P5)::index:
                case decltype(P1W)::index:
                    printf("TODO: pin 0x%02x write, value: 0x%02x\n", index, value);
                    ramData[index] = value;
                    return;
                case 0xc3:
                case 0xd3:
                case 0xe3:
                    printf("invalid P3-related write, value 0x%02x (doing NOOP)!\n", value);
                    return;

                //TODO: timer/counter
                case decltype(TC0M)::index:
                case decltype(TC1M)::index:
                case decltype(TC2M)::index:
                case decltype(TC0C)::index:
                case decltype(TC1C)::index:
                case decltype(TC2C)::index:
                case decltype(TC0R)::index:
                case decltype(TC1R)::index:
                case decltype(TC2R)::index:
                    printf("TODO: timer 0x%02x write, value: 0x%02x\n", index, value);
                    ramData[index] = value;
                    return;

                case decltype(AT_YZ)::index:
                    ramWrite(YZ.get(), value);
                    return;

                case decltype(WDTR)::index:
                    if(value == 0x5a) // clear watchdog timer
                    {
                        //TODO: probably pointless to emulate
                    }
                    return;

                case decltype(OSCM)::index:
                    printf("TODO: oscillator control register write, value: 0x%02x\n", value);
                    ramData[index] = value;
                    return;

                case decltype(PECMD)::index:
                {
                    auto PEROM = uint16_t(PEROM_H.get() << 8) | (PEROM_L.get() << 0);
                    auto PERAMCNT = PERAM_CNT.get();
                    auto PERAM = uint16_t(((PERAMCNT & 0x3) << 9) | (PERAM_L.get() << 0));
                    PERAMCNT >>= 3;
                    if(PERAM > 0x7f && PERAM < 0x100)
                    {
                        printf("invalid RAM address 0x%04x for flash operation!\n", PERAM);
                        return;
                    }

                    if(value == 0x5a) // page program (32 words)
                    {
                        if(PEROM % 32)
                        {
                            printf("misaligned ROM address 0x%04x for page program!\n", PEROM);
                            throw out_of_range("misaligned ROM address");
                        }
                        printf("performing 0x%x word page program, ROM address: 0x%04x, RAM address: 0x%04x\n", PERAMCNT + 1, PEROM, PERAM);
                        for(uint16_t i = 0, j = 0; i < PERAMCNT + 1; i++, j += 2)
                            rom.set(PEROM + i, ramRead(j + 1) << 8 | ramRead(j));
                    }
                    else if(value == 0xc3) // page erase (128 words)
                    {
                        if(PEROM % 128)
                        {
                            printf("misaligned ROM address 0x%04x for page erase!\n", PEROM);
                            throw out_of_range("misaligned ROM address");
                        }
                        printf("performing page erase at 0x%04x\n", PEROM);
                        for(uint16_t i = 0; i < 128; i++)
                            rom.set(PEROM + i, 0);
                    }
                    else
                    {
                        printf("invalid PECMD value 0x%02x\n", value);
                    }
                    return;
                }

                regWhitelist()
                    ramData[index] = value;
                    return;

                default:
                    printf("unknown register write at 0x%04x, PC: 0x%04x\n", index, PC.get());
            }
        }
        else if(index >= 0x100 && index <= 0x1FF) //BANK 1, General purpose area
        {
            ramData[index] = value;
            return;
        }
        else if(index >= 0x200 && index <= 0x27F) //BANK 2, General purpose area
        {
            ramData[index] = value;
            return;
        }
        printf("location: 0x%04x\n", index);
        throw out_of_range("invalid ram write");
    }

    bool flashRom(const std::vector<uint8_t> & data)
    {
        if(data.size() > sizeof(rom) || data.size() & 1)
            return false;
        for(size_t i = 0; i < rom.count; i++)
            rom.setNoLog(i, 0);
        for(size_t i = 0, j = 0; i < data.size(); i+= 2, j++)
            rom.setNoLog(j, data.at(i + 1) << 8 | data.at(i));
        //TODO: last 16 bytes of the ROM ???
        return true;
    }

    void reset(ResetType resetType = ResetPowerOn)
    {
        memset(ramData, 0, sizeof(ramData));
        ACC = 0;
        STKP.set(7);
        GIE.set(0);
        switch(resetType)
        {
            case ResetPowerOn:
                FNT0.set(1);
                FNPD.set(0);
                break;
            case ResetWatchdog:
                FNT0.set(0);
                FNPD.set(0);
                break;
            case ResetExternal:
                FNT0.set(1);
                FNPD.set(1);
                break;
        }
    }

    bool step()
    {
        try
        {
            return stepInternal();
        }
        catch(exception & ex)
        {
            printf("exception during instruction step: %s\n", ex.what());
            return false;
        }
    }

    bool stepInternal()
    {
        struct Timer
        {
            unsigned long long time1;

            Timer()
            {
                unsigned int aux = 0;
                time1 = __rdtscp(&aux);
            }

            ~Timer()
            {
                unsigned int aux = 0;
                auto time2 = __rdtscp(&aux);
                printf("time: %llu\n", time2 - time1);
            }
        };
        //fetch instruction
        auto addr = PC.get();
        auto instruction = rom.get(addr);
        //disassemble instruction
        auto bincode = uint8_t(instruction >> 8);
        uint8_t opcode_key = 0;
        bool is_bit = false;
        if(bincode >= 0x80)
        {
            opcode_key = uint8_t(bincode & 0xc0);
            is_bit = false;
        }
        else if(bincode >= 0x40)
        {
            opcode_key = uint8_t(bincode & 0xf8);
            is_bit = true;
        }
        else
        {
            opcode_key = bincode;
            is_bit = false;
        }

        char opcode[256] = "";
        auto entry = entries[opcode_key];
        if(entry.mnemonic == nullptr)
        {
            printf("DW 0x%02x ; ILLEGAL OPCODE\n", instruction);
            return false;
        }
        auto mask = entry.mask;
        auto opspace = entry.opSpace;
        auto jump_action = entry.branchType;
        auto mnemonic = entry.mnemonic;
        auto caption = entry.operands;

        strcat(opcode, mnemonic);

        uint16_t operand = 0;
        uint8_t bit_address = 0;

        if(opspace == NUL_SPACE)
        {
            //TODO: branch
        }
        else
        {
            char operand_fmt[256] = "";
            operand = instruction & mask;
            //TODO: branch
            if(opspace == ROM_SPACE)
            {
                strcpy(operand_fmt, "0x%04x");
                //assert not is_bit
            }
            else if(opspace == IMM_SPACE)
            {
                strcpy(operand_fmt, "#0x%02x");
                //assert not is_bit
            }
            else // ZRO & RAM
            {
                strcpy(operand_fmt, "0x%02x");
                if(is_bit)
                {
                    bit_address = uint8_t(bincode & 0x7);
                    sprintf(operand_fmt + strlen(operand_fmt), ".%i", bit_address);
                }
            }

            char symbol[256] = "";
            sprintf(symbol, operand_fmt, operand);
            strcat(opcode, " ");
            sprintf(opcode + strlen(opcode), caption, symbol);
        }

        auto next = [&]()
        {
            PC.set(addr + 1);
            return true;
        };
        auto skip = [&]()
        {
            PC.set(addr + 2);
            return true;
        };
        auto trbank = [&](uint16_t index)
        {
            return uint16_t(index + 0x100 * RBANK.get());
        };

        switch(entry.type)
        {
            case INSTR_NUL_NOP:
            {
                return next();
            }

            case INSTR_ROM_JMP_OP:
            {
                PC.set(operand);
                return true;
            }

            case INSTR_ZRO_B0BCLR_OP:
            {
                ram.setBit(operand, bit_address, 0);
                return next();
            }

            case INSTR_ZRO_B0BSET_OP:
            {
                ram.setBit(operand, bit_address, 1);
                return next();
            }

            case INSTR_IMM_B0MOV_Y_OP:
            {
                Y.set(operand);
                return next();
            }

            case INSTR_IMM_B0MOV_Z_OP:
            {
                Z.set(operand);
                return next();
            }

            case INSTR_NUL_MOVC:
            {
                auto yz = YZ.get();
                auto data = rom.get(yz);
                ACC = data & 0xFF;
                R.set(data >> 8);
                return next();
            }

            case INSTR_IMM_CMPRS_A_OP:
            {
                auto result = int(ACC) - int(operand);
                auto zero = result == 0;
                FZ.set(zero);
                FC.set(result < 0);
                return zero ? skip() : next();
            }

            case INSTR_RAM_INCMS_OP:
            {
                auto result = uint8_t(ram.read(operand) + 1);
                ram.write(operand, result);
                return result == 0 ? skip() : next();
            }

            case INSTR_ZRO_B0MOV_A_OP:
            {
                auto data = ram.read(operand);
                ACC = data;
                FZ.set(data == 0);
                return next();
            }

            case INSTR_IMM_MOV_A_OP:
            {
                ACC = operand;
                return next();
            }

            case INSTR_IMM_B0MOV_R_OP:
            {
                R.set(operand);
                return next();
            }

            case INSTR_RAM_B0MOV_OP_A:
            {
                ram.write(operand, ACC);
                return next();
            }

            case INSTR_IMM_B0MOV_RBANK_OP:
            {
                RBANK.set(operand);
                return next();
            }

            case INSTR_RAM_CLR_OP:
            {
                ram.write(operand, 0);
                return next();
            }

            case INSTR_RAM_DECMS_OP:
            {
                auto data = ram.read(operand);
                data--;
                ram.write(operand, data);
                return data == 0 ? skip() : next();
            }

            case INSTR_ZRO_B0BTS0_OP:
            {
                auto data = ram.getBit(operand, bit_address);
                return data == 0 ? skip() : next();
            }

            case INSTR_ZRO_B0BTS1_OP:
            {
                auto data = ram.getBit(operand, bit_address);
                return data == 1 ? skip() : next();
            }

            case INSTR_RAM_MOV_A_OP:
            {
                auto data = ram.read(operand);
                if(data == 0) //TODO: unclear wording on page 10
                    FZ.set(1);
                ACC = data;
                return next();
            }

            case INSTR_RAM_MOV_OP_A:
            {
                ram.write(operand, ACC);
                return next();
            }

            case INSTR_IMM_AND_A_OP:
            {
                auto data = ACC;
                data &= operand;
                FZ.set(data == 0);
                ACC = data;
                return next();
            }

            case INSTR_IMM_XOR_A_OP:
            {
                auto data = ACC;
                data ^= operand;
                FZ.set(data == 0);
                ACC = data;
                return next();
            }

            case INSTR_RAM_BCLR_OP:
            {
                ram.setBit(trbank(operand), bit_address, 0);
                return next();
            }

            case INSTR_RAM_BSET_OP:
            {
                ram.setBit(trbank(operand), bit_address, 1);
                return next();
            }

            case INSTR_ROM_CALL_OP:
            {
                auto stkp = STKP.get();
                switch(stkp)
                {
                    case 0: //TODO: what happens with the stack pointer?
                        STK7.set(addr + 1);
                        break;
                    case 1:
                        STK6.set(addr + 1);
                        break;
                    case 2:
                        STK5.set(addr + 1);
                        break;
                    case 3:
                        STK4.set(addr + 1);
                        break;
                    case 4:
                        STK3.set(addr + 1);
                        break;
                    case 5:
                        STK2.set(addr + 1);
                        break;
                    case 6:
                        STK1.set(addr + 1);
                        break;
                    case 7:
                        STK0.set(addr + 1);
                        break;
                    default:
                        //TODO: halt?
                        return false;
                }
                STKP.set(stkp - 1);
                PC.set(operand);
                return true;
            }

            case INSTR_NUL_RET:
            {
                auto stkp = STKP.get();
                stkp++;
                if(stkp > 7) //overflow
                    stkp = 0;
                auto newpc = 0;
                switch(stkp)
                {
                    case 0: //TODO: what happens with the stack pointer?
                        newpc = STK7.get();
                        break;
                    case 1:
                        newpc = STK6.get();
                        break;
                    case 2:
                        newpc = STK5.get();
                        break;
                    case 3:
                        newpc = STK4.get();
                        break;
                    case 4:
                        newpc = STK3.get();
                        break;
                    case 5:
                        newpc = STK2.get();
                        break;
                    case 6:
                        newpc = STK1.get();
                        break;
                    case 7:
                        newpc = STK0.get();
                        break;
                    default:
                        //TODO: halt?
                        return false;
                }
                STKP.set(stkp);
                PC.set(newpc);
                return true;
            }

            default:
                break;
        }

        printf("instruction not implemented: %s //%s\n", instrTypeToString(entry.type), disassemble(addr, instruction).c_str());
        return false;
    }
};

static bool invalidCommand(vector<string> &)
{
    puts("invalid command!");
    return false;
}

//USB implementation: https://youtube.com/watch?v=GFY_A3jcI28
//QEMU USB emulation: https://www.linux-kvm.org/page/USB

static int chip_main(int argc, char* argv[])
{
    initializeInstructions();
    auto romdata = readFile("lenovo.rom");

    printf("%02X %02X %02X %02X\n", romdata[0], romdata[1], romdata[2], romdata[3]);
    uint16_t instruction = romdata[1] << 8 | romdata[0];
    cout << disassemble(0, instruction) << endl;

    SN8F2288 chip;

    if(!chip.flashRom(romdata))
    {
        puts("flashRom failed...");
        return 1;
    }
    chip.reset(ResetPowerOn);

    vector<LogEntry> oldlog;
    auto step = [&]()
    {
        chip.log.clear(); //TODO
        auto ok = chip.step();
        puts(ok ? "ok" : "error");
        oldlog = chip.log;
        return ok;
    };
    auto disasm = [&](uint16_t addr)
    {
        if(addr > 0x3000)
            return;
        auto pc = chip.PC.get();
        auto data = chip.rom.get(addr);
        if(addr == pc)
            printf("PC: 0x%04x [0x%04x] %s\n", addr, data, disassemble(addr, data).c_str());
        else
            printf("    0x%04x [0x%04x] %s\n", addr, data, disassemble(addr, data).c_str());
    };
    auto showpc = [&](int offset)
    {
        auto pc = chip.PC.get() + offset;
        if(pc < 0 || pc > 0x3000)
            return;
        auto data = chip.rom.get(pc);
        if(offset == 0)
            printf("PC: 0x%04x [0x%04x] %s\n", pc, data, disassemble(pc, data).c_str());
        else
            printf("    0x%04x [0x%04x] %s\n", pc, data, disassemble(pc, data).c_str());
    };

    struct Cmd
    {
        string command;
        int argcount;
        function<bool(vector<string> &)> callback;

        Cmd() : Cmd("", 0, invalidCommand) { }
        Cmd(const char* command, int argcount, function<bool(vector<string> &)> callback) : command(command), argcount(argcount), callback(callback) { }
        Cmd(const char* command, function<void()> callback) : Cmd(command, 0, [callback](vector<string>&) { callback(); return true; }) { }
        Cmd(const char* command, function<void(unsigned int)> callback) : Cmd(command, 1, [callback](vector<string> & args)
        {
            unsigned int arg = 0;
            if(sscanf(args.at(0).c_str(), "%x", &arg) != 1)
            {
                puts("invalid format!");
                return true;
            }
            callback(arg);
            return true;
        }) { }
    };
    vector<Cmd> commands;
    commands.emplace_back("q", 0, [](vector<string>&)
    {
        puts("bye");
        return false;
    });
    commands.emplace_back("s", [&]()
    {
        step();
    });
    commands.emplace_back("pc", [&]()
    {
        showpc(-1);
        showpc(0);
        showpc(1);
    });
    commands.emplace_back("pcs", [&]()
    {
        showpc(-1);
        showpc(0);
        showpc(1);
        step();
    });
    commands.emplace_back("spc", [&]()
    {
        step();
        showpc(-1);
        showpc(0);
        showpc(1);
    });
    commands.emplace_back("log", [&]()
    {
        for(const auto & entry : oldlog)
            entry.print(true);
    });
    commands.emplace_back("st", [&]()
    {
        printf("STKP: %d, GIE: %d\n", chip.STKP.get(), chip.GIE.get());
        printf("STK7: 0x%04x\n", chip.STK7.get());
        printf("STK6: 0x%04x\n", chip.STK6.get());
        printf("STK5: 0x%04x\n", chip.STK5.get());
        printf("STK4: 0x%04x\n", chip.STK4.get());
        printf("STK3: 0x%04x\n", chip.STK3.get());
        printf("STK2: 0x%04x\n", chip.STK2.get());
        printf("STK1: 0x%04x\n", chip.STK1.get());
        printf("STK0: 0x%04x\n", chip.STK0.get());
    });
    commands.emplace_back("skip", [&]()
    {
        chip.PC.set(chip.PC.get() + 1);
        chip.log.clear();
    });
    commands.emplace_back("g", [&]()
    {
        while(chip.step()) ;
    });
    commands.emplace_back("goto", [&](unsigned int addr)
    {
        unsigned int count = 0;
        for(; chip.PC.get() != addr; count++)
            if(!chip.step())
            {
                puts("error");
                break;
            }
        printf("stopped after %u steps\n", count);
    });
    commands.emplace_back("gn", [&](unsigned int n)
    {
        unsigned int count = 0;
        for(; count < n; count++)
            if(!chip.step())
            {
                puts("error");
                break;
            }
        printf("stopped after %u steps\n", count);
    });
    commands.emplace_back("yz", [&]()
    {
        printf("YZ: 0x%04x\n", chip.YZ.get());
    });
    commands.emplace_back("usb", [&]()
    {
        chip.usb.debug();
    });
    commands.emplace_back("dis", [&](unsigned int addr)
    {
        disasm(addr - 3);
        disasm(addr - 2);
        disasm(addr - 1);
        disasm(addr);
        disasm(addr + 1);
        disasm(addr + 2);
        disasm(addr + 3);
    });

    string prevcommand;
    showpc(0);
    showpc(1);
    while(true)
    {
        printf("> ");
        string command;
        getline(cin, command);
        if(command.empty() && !prevcommand.empty())
            command = prevcommand;
        else
            prevcommand = command;

        if(command.empty())
        {
            puts("empty command!");
            continue;
        }

        vector<string> arguments;
        string current;
        for(char ch : command)
        {
            if(ch == ' ')
            {
                if(current.empty())
                    continue;
                arguments.emplace_back(current);
                current.clear();
            }
            else
            {
                current.push_back(ch);
            }
        }
        if(!current.empty())
            arguments.emplace_back(current);
        if(arguments.empty())
        {
            puts("empty command");
            continue;
        }
        auto cmdonly = arguments.at(0);
        arguments.erase(arguments.begin());

        vector<Cmd>::iterator found = find_if(commands.begin(), commands.end(), [&cmdonly](const Cmd & cmd)
        {
            return cmd.command == cmdonly;
        });
        if(found == commands.end())
        {
            puts("unknown command!");
        }
        else if(found->argcount > arguments.size())
        {
            puts("not enough arguments");
        }
        else if(!found->callback(arguments))
        {
            break;
        }
    }

    return 0;
}
