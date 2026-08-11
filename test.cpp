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
        // //double stall test
        // // lw x1,0(x0)
        // 0b00000000000000000010000010000011,
        //
        // // add x2,x1,x1
        // 0b00000000000100001000000100110011,
        //
        // // lw x3,4(x0)
        // 0b00000000010000000010000110000011,
        //
        // // add x4,x3,x3
        // 0b00000000001100011000001000110011

        //branch test
        // // addi x1,x0,5
        // 0b00000000010100000000000010010011,
        //
        // // addi x2,x0,5
        // 0b00000000010100000000000100010011,
        //
        // // beq x1,x2,+8
        // 0b00000000001000001000010001100011,
        //
        // // addi x3,x0,1
        // 0b00000000000100000000000110010011,
        //
        // // addi x4,x0,2
        // 0b00000000001000000000001000010011

        // // addi x1,x0,5
        // 0b00000000010100000000000010010011,
        //
        // // addi x2,x0,6
        // 0b00000000011000000000000100010011,
        //
        // // beq x1,x2,+8
        // 0b00000000001000001000010001100011,
        //
        // // addi x3,x0,1
        // 0b00000000000100000000000110010011,
        //
        // // addi x4,x0,2
        // 0b00000000001000000000001000010011

        // // addi x1,x0,2
        // 0b00000000001000000000000010010011,
        //
        // // addi x1,x1,-1
        // 0b11111111111100001000000010010011,
        //
        // // bne x1,x0,-4
        // 0b11111110000000001001111011100011

        // // addi x1,x0,1
        // 0b00000000000100000000000010010011,
        //
        // // jal x5,8
        // 0b00000000100000000000001011101111,
        //
        // // addi x1,x0,99
        // 0b00000110001100000000000010010011,
        //
        // // addi x2,x0,2
        // 0b00000000001000000000000100010011
        // addi x5,x0,12
        0b00000000110000000000001010010011,

        // jalr x1,0(x5)
        0b00000000000000101000000011100111,

        // addi x2,x0,99
        0b00000110001100000000000100010011,

        // addi x3,x0,3
        0b00000000001100000000000110010011
    };

    ram.loadCommands(commands, 4);
    cout<<core.l1_cache.Load(READWORD,UNSIGN,0,l2cache,ram)<<endl;



    // for (int i=0;i<20;i++) {
    //     cout<<endl;
    //     cout<<"================================"<<endl;
    //     cout<<"->Cycle "<<i<<endl;
    //     core.Step(l2cache,ram);
    // }
    // core.registerFile.dumpRawValue();

    int min_cycles = 0;
    for (int i=0;i<size(commands)*5;i++) {
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
    RunCommands(core0,l2cache,ram);
}