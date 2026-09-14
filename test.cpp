//this is a test environment for each cpu core
#include "src/RegisterFile.h"
#include<iostream>
#include<string>
#include<bitset>
#include<array>
#include "src/CPUcore.h"
#include <vector>

using namespace std;
void dumpSelectedReg(CPUcore core, int reg1_index, int reg2_index, int reg3_index) {
    cout<<"Reg "<<reg1_index<<" :"<<core.registerFile.read(reg1_index)<<endl;
    cout<<"Reg "<<reg2_index<<" :"<<core.registerFile.read(reg2_index)<<endl;
    cout<<"Reg "<<reg3_index<<" :"<<core.registerFile.read(reg3_index)<<endl;
}

void RunCommands(CPUcore &core, L2Cache& l2cache, RAM &ram) {
    uint32_t commands[] = {
        // // addi x1, x0, 5
        // 0b00000000010100000000000010010011,
        //
        // // addi x2, x0, 0
        // 0b00000000000000000000000100010011,
        //
        // // loop:
        // // addi x2, x2, 1
        // 0b00000000000100010000000100010011,
        //
        // // addi x1, x1, -1
        // 0b11111111111100001000000010010011,
        //
        // // bne x1, x0, -8
        // 0b11111110000000001001110011100011

        // // addi x1, x0, 100
        // 0b00000110010000000000000010010011,
        //
        // // sw x1, 128(x0)
        // 0b00001000000100000010000000100011,
        //
        // // lw x2, 128(x0)
        // 0b00001000000000000010000100000011,
        //
        // // sw x2, 132(x0)
        // 0b00001000001000000010001000100011

        // addi x2, x0, 100
        0b00000110010000000000000100010011,

        // lw x3, 0(x2)
        0b00000000000000010010000110000011,

        // sw x3, 4(x2)
        0b00000000001100010010001000100011,
    };

    ram.loadCommands(commands, 3);
    cout<<core.l1_cache.Load(READWORD,UNSIGN,0,l2cache,ram)<<endl;



    // for (int i=0;i<20;i++) {
    //     cout<<endl;
    //     cout<<"================================"<<endl;
    //     cout<<"->Cycle "<<i<<endl;
    //     core.Step(l2cache,ram);
    // }
    // core.registerFile.dumpRawValue();

    int min_cycles = 0;
    for (int i=0;i<20;i++) {
        cout<<endl;
        cout<<"================================"<<endl;
        cout<<"->Cycle "<<i<<endl;
        core.Step(l2cache,ram);

        if (i != 0 and core.pipeline_registers_read.IF_ID_register.valid == 0
            and core.pipeline_registers_read.ID_EX_register.valid == 0
            and core.pipeline_registers_read.EX_MEM_register.valid == 0
            and core.pipeline_registers_read.MEM_WB_register.valid == 0) {
            min_cycles = i+1;
            break;
            }
    }
    core.registerFile.dumpRawValue();
    cout<<"In total "<<min_cycles<<" cycles operated"<<endl;
    cout<<min_cycles<<" is the minimum cycles to fully run the commands"<<endl;

}

int main() {
    RAM ram = RAM();
    L2Cache l2cache = L2Cache(ram);
    CPUcore core0 = CPUcore(0,11);

    ram.writeCell(100,230);

    RunCommands(core0,l2cache,ram);
    cout<<"Branch Predictor Status: "<<core0.branch_predictor.enable<<endl;
    // core0.branch_predictor.DumpGHR();
    // core0.branch_predictor.DumpPHT();
    cout<<core0.l1_cache.readWord(104,l2cache,ram)<<endl;

}