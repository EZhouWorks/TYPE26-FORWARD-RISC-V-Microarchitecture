//
// Created by zhoue on 2026/7/17.
//

#ifndef RISC_V_CPU_SIMULATOR_PROGRAMCOUNTER_H
#define RISC_V_CPU_SIMULATOR_PROGRAMCOUNTER_H
#include <cstdint>

class ProgramCounter {
public:
    uint32_t PC_value = 0;
    uint32_t End_value;
    int branch_taken = 0; //indicates if branch op activated
    int jump_taken = 0;
    int enable;

    ProgramCounter(uint32_t entry_point,uint32_t end_point) {
        this->PC_value = entry_point;
        this->End_value = end_point;
        this->enable = 1;
    }

    void StepForward(int step) {
        PC_value = PC_value + step;
    }
    int CheckValid() { //this checks if the fetched machine code is still within the valid range
        if (PC_value < End_value) {
            return 1;
        }
        else {
            return 0;
        }
    }
    void Branch(uint32_t shift_amount,uint32_t command_PC_value) {
        PC_value = command_PC_value + int32_t(shift_amount);
        cout<<"Command PC Value = "<<command_PC_value<<endl;
        cout<<"Shift amount = "<<int32_t(shift_amount)<<endl;
        cout<<"BRANCHED TO "<<PC_value<<endl;
    }
    void Jump(uint32_t target_addr) {
        PC_value = target_addr;
    }

};
#endif //RISC_V_CPU_SIMULATOR_PROGRAMCOUNTER_H
