#include "DisassemblerBackend.h"

#include <QDebug>
#include <queue>
#include <QThread>
#include <QTimer>

DisassemblerBackend::DisassemblerBackend()
{
    initializeInstructions();
}

bool DisassemblerBackend::disassemble(DisasmLine & line, std::vector<uint16_t> & branches) const
{
    auto address = line.addr;
    auto instruction = line.data;

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
        return false;
    }
    auto mask = entry.mask;
    auto opspace = entry.opSpace;
    auto jump_action = entry.branchType;
    auto mnemonic = entry.mnemonic;
    auto caption = entry.operands;
    auto operand = instruction & mask;
    line.valid = true;
    line.entry = entry;

    strcat(opcode, mnemonic);

    auto doBranch = [&]()
    {
        switch(jump_action)
        {
        case NONXT: //There is no fixed next-instruction address (RET & RETI).
            break;
        case NEXTI: //Always execute next instruction (normal instructions).
            branches.push_back(address + 1);
            break;
        case BRNCH: //Execute next instruction or the one after that (conditional branches).
            branches.push_back(address + 1);
            branches.push_back(address + 2);
            break;
        case JUMPI: //Jump to some far address (unconditional branches).
            branches.push_back(operand);
            break;
        case CALLI: //Call some address (call instructions).
            branches.push_back(address + 1);
            branches.push_back(operand);
            break;
        }
    };

    if(opspace == NUL_SPACE)
    {
        doBranch();
    }
    else
    {
        Operand op;
        op.valid = true;
        op.value = operand;

        char operand_fmt[256] = "";
        doBranch();
        if(opspace == ROM_SPACE)
        {
            op.type = Operand::Rom;
            strcpy(operand_fmt, "0x%04x");
        }
        else if(opspace == IMM_SPACE)
        {
            op.type = Operand::Imm;
            strcpy(operand_fmt, "#0x%02x");
        }
        else // ZRO & RAM
        {
            strcpy(operand_fmt, "0x%02x");
            if(is_bit)
            {
                auto bit_address = bincode & 0x7;
                sprintf(operand_fmt + strlen(operand_fmt), ".%i", bit_address);

                op.bit = bit_address;
                op.type = Operand::Bit;
            }
            else
                op.type = Operand::Ram;
        }

        auto opIndex = 0;
        auto split = QString(caption).split(',');
        for(QString & text : split)
        {
            if(text.trimmed() == "%s")
            {
                op.text = QString().sprintf(operand_fmt, operand);
                line.operands[opIndex++] = op;
            }
            else
            {
                Operand reg;
                reg.valid = true;
                reg.type = Operand::Reg;
                reg.text = text.trimmed();
                line.operands[opIndex++] = reg;
            }
        }

        char symbol[256] = "";
        sprintf(symbol, operand_fmt, operand);
        strcat(opcode, " ");
        sprintf(opcode + strlen(opcode), caption, symbol);
    }
    //return string(opcode);
    return true;
}

void DisassemblerBackend::loadRom(const QString & file)
{
    Core::cpu()->haltCpu();
    auto & chip = Core::chip();
    Core::db().clear();
    auto romData = readFile(file.toUtf8().constData());
    if(!chip.flashRom(romData))
    {
        qDebug() << "failed to flash rom!";
        return;
    }
    chip.reset(ResetPowerOn);

    lineInfo.resize(chip.rom.count);
    tokenLines.resize(lineInfo.size());
    for(size_t i = 0; i < chip.rom.count; i++)
    {
        lineInfo[i].addr = i;
        lineInfo[i].data = chip.rom.get(uint16_t(i));
    }

    std::queue<uint16_t> q;
    q.push(0); //reset
    q.push(8); //interrupt
    std::vector<uint16_t> callTargets;
    while(!q.empty()) //bfs
    {
        auto curAddr = q.front();
        q.pop();

        DisasmLine & curLine = lineInfo[curAddr];

        if(curLine.valid)
            continue; //skip instructions we already disassembled (visited list)

        std::vector<uint16_t> branches;
        disassemble(curLine, branches);

        if(curLine.entry.branchType == CALLI)
            callTargets.push_back(curLine.operands[0].value);

        for(auto branch : branches)
        {
            q.push(branch);
            if(curLine.entry.branchType == NEXTI) //don't create xrefs for the previous instruction
                continue;
            lineInfo[branch].referencedFrom.push_back(curAddr);
        }
    }

    /*qDebug() << "possible functions:";

    //https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
    sort(callTargets.begin(), callTargets.end());
    callTargets.erase(unique(callTargets.begin(), callTargets.end()), callTargets.end());
    for(uint16_t func : callTargets)
        qDebug() << QString().sprintf("0x%04x", func);*/
}
