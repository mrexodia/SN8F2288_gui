#pragma once

#include <cstdint>

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
