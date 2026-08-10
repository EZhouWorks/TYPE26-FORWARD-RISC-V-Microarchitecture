#ifndef RISC_V_CPU_SIMULATOR_ALU_H
#define RISC_V_CPU_SIMULATOR_ALU_H
#include <cstdint>
#include <iostream>
#include "ProgramCounter.h"
using namespace std;

enum ALU_op
{
    ADD_op,
    SUB_op,
    AND_op,
    OR_op,
    XOR_op,
    SLL_op,
    SRL_op,
    SRA_op,
    SLT_op,
    SLTU_op,
    BEQ_op,
    BNE_op,
    BLT_op,
    BGE_op,
    BLTU_op,
    BGEU_op,
    NO_ALU_OP,
};

enum ALU_source {
    rs1,
    rs2,
    I_12bit_imm,
    I_shamt_imm,
    store_imm,
    J_imm,
    command_PC_value
};


class ALU {
private:
    uint32_t ADD(uint32_t x1_value, uint32_t x2_value) { //command 0
        return x1_value+x2_value;
    }
    uint32_t SUB(uint32_t x1_value, uint32_t x2_value) { //command 1
        return x1_value-x2_value;
    }
    uint32_t AND(uint32_t x1_value, uint32_t x2_value) { //command 2
        return x1_value&x2_value;
    }
    uint32_t OR(uint32_t x1_value, uint32_t x2_value) { //command 3
        return x1_value|x2_value;
    }
    uint32_t XOR(uint32_t x1_value, uint32_t x2_value) { //command 4
        return x1_value^x2_value;
    }

    uint32_t SLL(uint32_t x1_value, uint32_t shift_amount) { //command 5
        return x1_value<<(shift_amount & 0x1f);
    }

    uint32_t SRL(uint32_t x1_value, uint32_t shift_amount) { //unsign; command 6
        return x1_value >> (shift_amount & 0x1f);
    }

    uint32_t SRA(uint32_t x1_value, uint32_t shift_amount) { //signed; command 7
        return (uint32_t)((int32_t)x1_value >> (shift_amount & 0x1f));
    }

    uint32_t SLT(uint32_t x1_value, uint32_t x2_value) { //command 8
        //signed
        if ((int32_t)x1_value<(int32_t)x2_value) {
            return 1;
        }
        else {
            return 0;
        }
    }

    uint32_t SLTU(uint32_t x1_value, uint32_t x2_value){//unsigned; command 9
        if (x1_value<x2_value) {
            return 1;
        }
        else {
            return 0;
        }
    }

public:
    ALU(){};
    uint32_t operate(ALU_op command, uint32_t x1_value = 0, uint32_t x2_value = 0) {
        switch (command) {
                case ADD_op: return ADD(x1_value, x2_value);
                case SUB_op: return SUB(x1_value, x2_value);
                case AND_op: return AND(x1_value, x2_value);
                case OR_op:  return OR(x1_value, x2_value);
                case XOR_op: return XOR(x1_value, x2_value);
                case SLL_op: return SLL(x1_value, x2_value);
                case SRL_op: return SRL(x1_value, x2_value);
                case SRA_op: return SRA(x1_value, x2_value);
                case SLT_op: return SLT(x1_value, x2_value);
                case SLTU_op: return SLTU(x1_value, x2_value);
                case NO_ALU_OP: return 0;
                default:
                    throw runtime_error("Invalid ALU command");
        }
    }

};

#endif //RISC_V_CPU_SIMULATOR_ALU_H
