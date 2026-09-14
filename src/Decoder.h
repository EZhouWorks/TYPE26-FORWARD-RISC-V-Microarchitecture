#ifndef RISC_V_CPU_SIMULATOR_DECODER_H
#define RISC_V_CPU_SIMULATOR_DECODER_H
#include<cstdint>
#include<string>
#include<bitset>
class Decoder {
public:
    uint32_t rd,rs1,rs2,I_shamt_imm,SYS_code,I_12bit_imm,Store_imm,B_imm, J_imm = 0b0;
    int insert_bubble = 0; // allow Decoder to run for bubbles while Fetch starts draining.
    int StoreLoad_bubble = 0; // sep L-S stall from other types of stall
    uint32_t Decode(uint32_t machine_code) {
        //machine code dispart (addr)
        uint32_t opcode = machine_code & 0x7F;
        rd     = (machine_code >> 7) & 0x1F;
        uint32_t funct3 = (machine_code >> 12) & 0x07;
        rs1    = (machine_code >> 15) & 0x1F;
        rs2    = (machine_code >> 20) & 0x1F;
        uint32_t funct7 = (machine_code >> 25) & 0x7F;
        I_12bit_imm = (((machine_code >> 20) & 0xFFF) & 0x800) ? (((machine_code >> 20) & 0xFFF) | 0xFFFFF000) : ((machine_code >> 20) & 0xFFF);
        I_shamt_imm = (machine_code >> 20) & 0x1F;
        SYS_code = I_12bit_imm;
        Store_imm = ((((machine_code >> 25) & 0x7F) << 5) | ((machine_code >> 7) & 0x1F)) & 0x800 ? ((((machine_code >> 25) & 0x7F) << 5) | ((machine_code >> 7) & 0x1F)) | 0xFFFFF000 : (((machine_code >> 25) & 0x7F) << 5) | ((machine_code >> 7) & 0x1F);
        uint32_t raw_B_imm = (((machine_code >> 31) & 1) << 12) | (((machine_code >> 7) & 1) << 11) | (((machine_code >> 25) & 0x3F) << 5) | (((machine_code >> 8) & 0xF) << 1);
        B_imm = (raw_B_imm & 0x1000) ? (raw_B_imm | 0xFFFFE000) : raw_B_imm;
        J_imm = (uint32_t)(((int32_t)((((((machine_code >> 31) & 0x1) << 20) | (((machine_code >> 21) & 0x3FF) << 1) | (((machine_code >> 20) & 0x1) << 11) | (((machine_code >> 12) & 0xFF) << 12)) << 11)) >> 11));


        switch (opcode) {
            case(0b0110011): //R-Type
                switch(funct3) {
                    case(0b000):
                        switch(funct7) {
                            case(0b0000000):return 1; //ADD
                            case(0b0100000):return 2;//SUB
                        default: return 0;
                        }
                    case(0b001):return 3;//SLL
                    case(0b010):return 4;//SLT
                    case(0b011):return 5;//SLTU
                    case(0b100):return 6;//XOR
                    case(0b101):
                        switch (funct7) {
                            case(0b0000000):return 7;//SRL
                            case(0b0100000):return 8;//SRA
                        default: return 0;
                        }
                    case(0b110):return 9;//OR
                    case(0b111):return 10;//AND
                default:return 0;
                }
            case(0b0010011):   // I-Type ALU
                switch(funct3) {
                    case(0b000):
                        return 11;   // ADDI
                    case(0b010):
                        return 12;   // SLTI
                    case(0b011):
                        return 13;   // SLTIU
                    case(0b100):
                        return 14;   // XORI
                    case(0b001):
                        switch(funct7) {
                            case(0b0000000):
                                return 15;   // SLLI
                        default:return 0;
                        }
                    case(0b101):
                        switch(funct7) {
                            case(0b0000000):
                                return 16;   // SRLI
                            case(0b0100000):
                                return 17;   // SRAI
                        default:return 0;
                        }
                    case(0b110):
                        return 18;   // ORI
                    case(0b111):
                        return 19;   // ANDI
                default:return 0;
                }
            case(0b0000011): //Load
                switch (funct3) {
                    case(0b000):return 20; //LB
                    case(0b001):return 21; //LH
                    case(0b010):return 22; //LW
                    case(0b100):return 23; //LBU
                    case(0b101):return 24; //LHU
                default:return 0;
                }
            case(0b0100011): //Store
                switch (funct3) {
                    case(0b000):return 25; //SB
                    case(0b001):return 26; //SH
                    case(0b010):return 27; //SW
                default:return 0;
                }
            case(0b1100011): //Branch
                switch(funct3) {
                    case(0b000):return 28; //BEQ
                    case(0b001):return 29; //BNE
                    case(0b100):return 30; //BLT
                    case(0b101):return 31; //BGE
                    case(0b110):return 32; //BLTU
                    case(0b111):return 33; //BGEU
                default:return 0;
                }
            case(0b0110111): //LUI
                return 34;
            case(0b0010111): //AUIPC
                return 35;
            case(0b1101111):
                return 36; //JAL
            case(0b1100111):
                return 37; //JALR
            case(1110011):
                if (SYS_code== 0) {
                    return 38; //ECALL
                }
                else
                    return 39; //EBREAK

            default:return 0;
        }

    }



};
#endif //RISC_V_CPU_SIMULATOR_DECODER_H
