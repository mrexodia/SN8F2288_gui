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

#include "sn8f2288_chip.h"

using namespace std;

vector<uint8_t> readFile(const char* filename)
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

const char* instrTypeToString(InstrType type)
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

InstrEntry entries[256];

void initializeInstructions()
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

string disassemble(uint16_t address, uint16_t instruction)
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
