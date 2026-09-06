//
// Created by Enze Zhou on 2026/8/3.
//
#ifndef RISCV_CPU_BRANCHUNIT_H
#define RISCV_CPU_BRANCHUNIT_H
#include <stdexcept>
#include "ProgramCounter.h"
enum Branch_op {
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    NO_BRANCH_OP
};
class BranchUnit {
public:
    BranchUnit(){};
    int BEQ(uint32_t x1_value, uint32_t x2_value, uint32_t B_imm, ProgramCounter& program_counter, uint32_t command_PC_value, int BPU_enable) {
        if (int32_t(x1_value) == int32_t(x2_value)) {
            if (BPU_enable == 0) {
                program_counter.branch_taken = 1;
                program_counter.Branch(B_imm,command_PC_value);
                cout<<"EQUAL triggered"<<endl;
            }
            return 1;
        }
        else {
            cout<<"not equal triggered"<<endl;
            return 0;
        }
    }
    int BNE(uint32_t x1_value, uint32_t x2_value, uint32_t B_imm, ProgramCounter& program_counter, uint32_t command_PC_value, int BPU_enable) {
        if (int32_t(x1_value) != int32_t(x2_value)) {
            if (BPU_enable == 0) {
                program_counter.branch_taken = 1;
                program_counter.Branch(B_imm, command_PC_value);
                cout<<"BNE triggered"<<endl;
            }
            return 1;
        }
        else {
            cout<<"BNE not triggered"<<endl;
            return 0;
        }
    }
    int BLT(uint32_t x1_value, uint32_t x2_value, uint32_t B_imm, ProgramCounter& program_counter, uint32_t command_PC_value, int BPU_enable) {
        if (int32_t(x1_value) < int32_t(x2_value)) {
            if (BPU_enable == 0) {
                program_counter.branch_taken = 1;
                program_counter.Branch(B_imm, command_PC_value);
                cout<<"BNE triggered"<<endl;
            }
            return 1;
        }
        else {
            return 0;
        }
    }
    int BGE(uint32_t x1_value, uint32_t x2_value, uint32_t B_imm, ProgramCounter& program_counter, uint32_t command_PC_value, int BPU_enable) {
        if (int32_t(x1_value) >= int32_t(x2_value)) {
            if (BPU_enable == 0) {
                program_counter.branch_taken = 1;
                program_counter.Branch(B_imm, command_PC_value);
                cout<<"BNE triggered"<<endl;
            }
            return 1;
        }
        else {
            return 0;
        }
    }
    int BLTU(uint32_t x1_value, uint32_t x2_value, uint32_t B_imm, ProgramCounter& program_counter, uint32_t command_PC_value, int BPU_enable) {
        if (x1_value < x2_value) {
            if (BPU_enable == 0) {
                program_counter.branch_taken = 1;
                program_counter.Branch(B_imm, command_PC_value);
                cout<<"BNE triggered"<<endl;
            }
            return 1;
        }
        else {
            return 0;
        }
    }
    int BGEU(uint32_t x1_value, uint32_t x2_value, uint32_t B_imm, ProgramCounter& program_counter, uint32_t command_PC_value, int BPU_enable) {
        if (x1_value >= x2_value) {
            if (BPU_enable == 0) {
                program_counter.branch_taken = 1;
                program_counter.Branch(B_imm, command_PC_value);
                cout<<"BNE triggered"<<endl;
            }
            return 1;
        }
        else {
            return 0;
        }
    }

    //this returns the valid digit for pipeline registers when branch taken
    int operate(Branch_op branch_op, uint32_t x1_value, uint32_t x2_value, int32_t B_imm, ProgramCounter& program_counter, uint32_t command_PC_value, int BPU_enable) {
        switch (branch_op) {
            case Branch_op::BEQ:cout<<"BRCH TRGD"<<endl;return BEQ(x1_value,x2_value,B_imm,program_counter,command_PC_value,BPU_enable);
            case Branch_op::BNE:cout<<"BRCH TRGD"<<endl;return BNE(x1_value,x2_value,B_imm,program_counter,command_PC_value,BPU_enable);
            case Branch_op::BLT:cout<<"BRCH TRGD"<<endl;return BLT(x1_value,x2_value,B_imm,program_counter,command_PC_value,BPU_enable);
            case Branch_op::BGE:cout<<"BRCH TRGD"<<endl;return BGE(x1_value,x2_value,B_imm,program_counter,command_PC_value,BPU_enable);
            case Branch_op::BLTU:cout<<"BRCH TRGD"<<endl;return BLTU(x1_value,x2_value,B_imm,program_counter,command_PC_value,BPU_enable);
            case Branch_op::BGEU:cout<<"BRCH TRGD"<<endl;return BGEU(x1_value,x2_value,B_imm,program_counter,command_PC_value, BPU_enable);
            case Branch_op::NO_BRANCH_OP:return 0;
            default: throw std::runtime_error("Invalid Branch Operation");
        }
    }

};
#endif //RISCV_CPU_BRANCHUNIT_H