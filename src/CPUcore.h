//
// Created by zhoue on 2026/7/17.
//

#ifndef RISC_V_CPU_SIMULATOR_CPUCORE_H
#define RISC_V_CPU_SIMULATOR_CPUCORE_H
#include <iostream>
#include <cstdint>
#include <bitset>
#include "ALU.h"
#include "Decoder.h"
#include "Controller.h"
#include "ForwardingUnit.h"
#include "RegisterFile.h"
#include "L1Cache.h"
#include "RAM.h"
#include "PipelineRegisters.h"
#include "ProgramCounter.h"
#include "StallUnit.h"
#include "BranchUnit.h"
#include "JumpUnit.h"

class CPUcore {
public:
    EnableSignals enable_signal;
    Controller controller = Controller();
    ALU alu = ALU();
    Decoder decoder = Decoder();
    RegisterFile registerFile = RegisterFile();
    int core_id;
    L1Cache l1_cache = L1Cache();
    PipelineRegisters pipeline_registers_write = PipelineRegisters(); //this pipeline register set is write only
    PipelineRegisters pipeline_registers_read = PipelineRegisters();  //this pipeline register set is read only
    ForwardingUnit forwarding_unit = ForwardingUnit();
    StallUnit stall_unit = StallUnit();
    ProgramCounter program_counter;
    BranchUnit branch_unit = BranchUnit();
    JumpUnit jump_unit = JumpUnit();

    CPUcore(int core_id,int end_point)
        :core_id(core_id),
         program_counter(0,end_point)
    {
        this->enable_signal.FetchEnable = 1;
        this->enable_signal.DecodeEnable = 1;
        this->enable_signal.ExecuteEnable = 1;
        this->enable_signal.MemoryEnable = 1;
        this->enable_signal.WriteBackEnable = 1;
    }

    uint8_t CPULoadByte(uint32_t addr,L2Cache& l2cache,RAM& ram) {
        return l1_cache.readByte(addr,l2cache,ram);
    }
    uint16_t CPULoadHalfWord(uint32_t addr,L2Cache& l2cache,RAM& ram) {
        return l1_cache.readHalfWord(addr,l2cache,ram);
    }
    uint32_t CPULoadWord(uint32_t addr,L2Cache& l2cache,RAM& ram) {
        return l1_cache.readWord(addr,l2cache,ram);
    }

    void Fetch(L2Cache& l2cache,RAM& ram) {
        cout<<"FETCH VALID "<<pipeline_registers_write.IF_ID_register.valid<<endl<<endl;
        if (program_counter.enable == 0) {  //stall
            cout<<"MACHINE CODE AT FETCH: "<<bitset<32>(pipeline_registers_write.IF_ID_register.machine_code)<<endl;
            return;
        }
        else {
            if (program_counter.CheckValid() == 1 and pipeline_registers_write.IF_ID_register.enable == 1 or program_counter.branch_taken == 1 or program_counter.jump_taken == 1) {
                //Normal
                pipeline_registers_write.IF_ID_register.machine_code = CPULoadWord(program_counter.PC_value,l2cache,ram);
                pipeline_registers_write.IF_ID_register.command_PC_value = program_counter.PC_value;
                pipeline_registers_write.IF_ID_register.valid = 1;
                cout<<"MACHINE CODE AT FETCH "<<bitset<32>(pipeline_registers_write.IF_ID_register.machine_code)<<endl;

                //Branch
                if (program_counter.branch_taken == 1) {
                    program_counter.branch_taken = 0;
                    pipeline_registers_write.IF_ID_register.machine_code = CPULoadWord(program_counter.PC_value,l2cache,ram);
                    cout<<"BRANCHED TO "<<bitset<32>(pipeline_registers_write.IF_ID_register.machine_code)<<endl;
                }

                //Jump
                if (program_counter.jump_taken == 1) {
                    program_counter.jump_taken = 0;
                    pipeline_registers_write.IF_ID_register.machine_code = CPULoadWord(program_counter.PC_value,l2cache,ram);
                    cout<<"JUMPED TO "<<bitset<32>(pipeline_registers_write.IF_ID_register.machine_code)<<endl;
                }

                if (program_counter.enable == 1) {
                    program_counter.StepForward(4);
                }
            }
            else {
                pipeline_registers_write.IF_ID_register.valid = 0;
            }
        }

    }

    void Decode() {
        cout<<"DECODE VALID "<<pipeline_registers_read.IF_ID_register.valid<<endl;
        cout<<"DECODING "<<bitset<32>(pipeline_registers_read.IF_ID_register.machine_code)<<endl;

        if (pipeline_registers_read.IF_ID_register.valid == 1 or decoder.insert_bubble == 1) { //stall has higher priority than normal drain procedure
            controller.SetControlSignal(decoder.Decode(pipeline_registers_read.IF_ID_register.machine_code)); //decode and generate control singal

            //Stall check
            if (controller.ALU_source1 == rs1) {
                if (pipeline_registers_read.ID_EX_register.valid == 1 and stall_unit.checkStall(pipeline_registers_read.ID_EX_register,decoder.rs1) == 1) {
                    pipeline_registers_write.ID_EX_register.ALU_operation = NO_ALU_OP; //create bubble, NOP will be passed down for every stall cycle
                    pipeline_registers_write.ID_EX_register.Memory_op = NO_MEMORY_OP;
                    pipeline_registers_write.ID_EX_register.Store_op = NO_STORE_OP;
                    stall_unit.SetStall(program_counter,decoder,pipeline_registers_write.IF_ID_register);
                    pipeline_registers_write.ID_EX_register.valid = 0;
                    return;
                }
                else {
                    //cout<<"Stall End1"<<endl;
                    stall_unit.ExitStall(program_counter,decoder,pipeline_registers_write.IF_ID_register);
                }
            }
            else if (controller.ALU_source1 == rs2) {
                if (pipeline_registers_read.ID_EX_register.valid == 1 and stall_unit.checkStall(pipeline_registers_read.ID_EX_register,decoder.rs2) == 1) {
                    pipeline_registers_write.ID_EX_register.ALU_operation = NO_ALU_OP;
                    pipeline_registers_write.ID_EX_register.Memory_op = NO_MEMORY_OP;
                    pipeline_registers_write.ID_EX_register.Store_op = NO_STORE_OP;
                    stall_unit.SetStall(program_counter,decoder,pipeline_registers_write.IF_ID_register);
                    pipeline_registers_write.ID_EX_register.valid = 0;
                    return;
                }
                else {
                    //cout<<"Stall End2"<<endl;
                    stall_unit.ExitStall(program_counter,decoder,pipeline_registers_write.IF_ID_register);
                }
            }
            if (controller.ALU_source2 == rs2) {
                if (pipeline_registers_read.ID_EX_register.valid == 1 and stall_unit.checkStall(pipeline_registers_read.ID_EX_register,decoder.rs2) == 1) {
                    pipeline_registers_write.ID_EX_register.ALU_operation = NO_ALU_OP;
                    pipeline_registers_write.ID_EX_register.Memory_op = NO_MEMORY_OP;
                    pipeline_registers_write.ID_EX_register.Store_op = NO_STORE_OP;
                    stall_unit.SetStall(program_counter, decoder,pipeline_registers_write.IF_ID_register);
                    pipeline_registers_write.ID_EX_register.valid = 0;
                    return;
                }
                else {
                    //cout<<"Stall End3"<<endl;
                    stall_unit.ExitStall(program_counter,decoder,pipeline_registers_write.IF_ID_register);
                }
            }

            //store data and control signal into pipeline register
            pipeline_registers_write.ID_EX_register.ALU_operation = controller.ALU_operation;
            pipeline_registers_write.ID_EX_register.ALU_source1 = controller.ALU_source1;
            pipeline_registers_write.ID_EX_register.ALU_source2 = controller.ALU_source2;
            pipeline_registers_write.ID_EX_register.Memory_op = controller.Memory_op;
            pipeline_registers_write.ID_EX_register.Memory_data_type = controller.Memory_data_type;
            pipeline_registers_write.ID_EX_register.rs1_addr = decoder.rs1;
            pipeline_registers_write.ID_EX_register.rs2_addr = decoder.rs2;
            pipeline_registers_write.ID_EX_register.rd_addr = decoder.rd;

            //WB Bypass
            if (pipeline_registers_read.MEM_WB_register.valid == 1 and forwarding_unit.ForwardingCompare(pipeline_registers_read.MEM_WB_register.rd_addr,decoder.rs1) == 1) {
                pipeline_registers_write.ID_EX_register.rs1_val = pipeline_registers_read.MEM_WB_register.ALU_result;
            }
            else {
                pipeline_registers_write.ID_EX_register.rs1_val = registerFile.read(decoder.rs1);
            }
            if (pipeline_registers_read.MEM_WB_register.valid == 1 and forwarding_unit.ForwardingCompare(pipeline_registers_read.MEM_WB_register.rd_addr,decoder.rs2) == 1) {
                pipeline_registers_write.ID_EX_register.rs2_val = pipeline_registers_read.MEM_WB_register.ALU_result;
            }
            else {
                pipeline_registers_write.ID_EX_register.rs2_val = registerFile.read(decoder.rs2);
            }

            pipeline_registers_write.ID_EX_register.I_12bit_imm = decoder.I_12bit_imm;
            pipeline_registers_write.ID_EX_register.I_shamt_imm = decoder.I_shamt_imm;
            pipeline_registers_write.ID_EX_register.Memory_op = controller.Memory_op;
            pipeline_registers_write.ID_EX_register.Store_imm = decoder.Store_imm;
            pipeline_registers_write.ID_EX_register.Store_op = controller.Store_op;
            pipeline_registers_write.ID_EX_register.RegFile_op = controller.RegFile_op;
            pipeline_registers_write.ID_EX_register.Branch_op = controller.Branch_op;
            pipeline_registers_write.ID_EX_register.B_imm = decoder.B_imm;
            pipeline_registers_write.ID_EX_register.command_PC_value = pipeline_registers_read.IF_ID_register.command_PC_value;
            pipeline_registers_write.ID_EX_register.jump_op = controller.Jump_op;
            pipeline_registers_write.ID_EX_register.J_imm = decoder.J_imm;
            pipeline_registers_write.ID_EX_register.valid = 1;

        }
        else {
            pipeline_registers_write.ID_EX_register.valid = 0;
        }
        cout<<endl;
    }

    void Execution() {
        cout<<"EX VALID "<<pipeline_registers_read.ID_EX_register.valid<<endl;
        if (pipeline_registers_read.ID_EX_register.valid == 1) {
            uint32_t ALU_result = 0b0;
            ALU_op alu_op = pipeline_registers_read.ID_EX_register.ALU_operation;
            ALU_source alu_source1 = pipeline_registers_read.ID_EX_register.ALU_source1;
            ALU_source alu_source2 = pipeline_registers_read.ID_EX_register.ALU_source2;
            uint32_t rs1_val = pipeline_registers_read.ID_EX_register.rs1_val;
            uint32_t rs2_val = pipeline_registers_read.ID_EX_register.rs2_val;
            uint32_t rs1_addr = pipeline_registers_read.ID_EX_register.rs1_addr;
            uint32_t rs2_addr = pipeline_registers_read.ID_EX_register.rs2_addr;
            uint32_t rs1_input_val = rs1_val;
            uint32_t rs2_input_val = rs2_val;
            uint32_t I_12bit_imm = pipeline_registers_read.ID_EX_register.I_12bit_imm;
            uint32_t I_shamt_imm = pipeline_registers_read.ID_EX_register.I_shamt_imm;
            uint32_t store_imm = pipeline_registers_read.ID_EX_register.Store_imm;
            Branch_op branch_op = pipeline_registers_read.ID_EX_register.Branch_op;
            int32_t B_imm = pipeline_registers_read.ID_EX_register.B_imm;
            uint32_t command_PC_value = pipeline_registers_read.ID_EX_register.command_PC_value;
            Jump_op jump_op = pipeline_registers_read.ID_EX_register.jump_op;
            uint32_t J_imm = pipeline_registers_read.ID_EX_register.J_imm;

            //ALU forwarding check
            if (pipeline_registers_write.EX_MEM_register.valid == 1 and forwarding_unit.ForwardingCompare(rs1_addr,pipeline_registers_read.EX_MEM_register.rd_addr) == 1) {
                rs1_input_val = pipeline_registers_read.EX_MEM_register.ALU_result; //fetch ALU result from the read register of the next stage
                //cout<<"FORWARDED FROM ALU RESULT"<<endl;
            }
            if (pipeline_registers_write.EX_MEM_register.valid == 1 and forwarding_unit.ForwardingCompare(rs2_addr,pipeline_registers_read.EX_MEM_register.rd_addr) == 1) {
                rs2_input_val = pipeline_registers_read.EX_MEM_register.ALU_result;
                //cout<<"FORWARDED FROM ALU RESULT"<<endl;
            }

            //WB bypass
            if (pipeline_registers_write.MEM_WB_register.valid == 1 and forwarding_unit.ForwardingCompare(rs1_addr,pipeline_registers_read.MEM_WB_register.rd_addr) == 1) {
                //check if data comes from ALU result or loaded from MEM
                if (pipeline_registers_read.MEM_WB_register.Memory_op == Memory_op::NO_MEMORY_OP) {
                    rs1_input_val = pipeline_registers_read.MEM_WB_register.ALU_result;
                }
                else {
                    rs1_input_val = pipeline_registers_read.MEM_WB_register.data;
                }
            }
            if (pipeline_registers_write.MEM_WB_register.valid == 1 and forwarding_unit.ForwardingCompare(rs2_addr,pipeline_registers_read.MEM_WB_register.rd_addr) == 1) {
                if (pipeline_registers_read.MEM_WB_register.Memory_op == Memory_op::NO_MEMORY_OP) {
                    rs2_input_val = pipeline_registers_read.MEM_WB_register.ALU_result;
                }
                else {
                    rs2_input_val = pipeline_registers_read.MEM_WB_register.data;
                }
            }

            //ALU execution
            switch (alu_source1) {
                case rs1:
                    switch (alu_source2) {
                    case(ALU_source::rs2): ALU_result = alu.operate(alu_op,rs1_input_val, rs2_input_val);break;
                    case(ALU_source::I_12bit_imm):ALU_result = alu.operate(alu_op,rs1_input_val, I_12bit_imm);break;
                    case(ALU_source::I_shamt_imm):ALU_result = alu.operate(alu_op,rs1_input_val, I_shamt_imm);break;
                    case(ALU_source::store_imm):ALU_result = alu.operate(alu_op,rs1_input_val, store_imm);break;
                    default: throw runtime_error("clear ALU src1 but unknown ALU src2");
                    };break;
                case rs2:
                    ALU_result = alu.operate(alu_op,rs1_input_val, I_12bit_imm);break;
                case ALU_source::J_imm:
                    ALU_result = alu.operate(alu_op, J_imm, command_PC_value);break;
                default: throw runtime_error("Unknown ALU source 1");
            }

            //BranchUnit execution
            pipeline_registers_write.ID_EX_register.valid = branch_unit.operate(branch_op,rs1_input_val,rs2_input_val,B_imm,program_counter,command_PC_value);
            pipeline_registers_write.IF_ID_register.valid = pipeline_registers_write.ID_EX_register.valid;

            //JumpUnit execution
            pipeline_registers_write.EX_MEM_register.command_PC_value = jump_unit.operate(jump_op,command_PC_value,ALU_result, program_counter); //command_PC_value register is used by JumpUnit to save command_addr+4.
            if (program_counter.jump_taken == 1) {
                pipeline_registers_write.IF_ID_register.valid = 0;
                pipeline_registers_write.ID_EX_register.valid = 0;
                // cout<<"PC value at EXE = "<<program_counter.PC_value<<endl;
            }

            //pass on data from ID/EX Register
            pipeline_registers_write.EX_MEM_register.ALU_result = ALU_result;
            pipeline_registers_write.EX_MEM_register.Memory_op = pipeline_registers_read.ID_EX_register.Memory_op;
            pipeline_registers_write.EX_MEM_register.Memory_data_type = pipeline_registers_read.ID_EX_register.Memory_data_type;
            pipeline_registers_write.EX_MEM_register.rd_addr = pipeline_registers_read.ID_EX_register.rd_addr;
            pipeline_registers_write.EX_MEM_register.rs1_val = rs1_input_val;
            pipeline_registers_write.EX_MEM_register.rs2_val = rs2_input_val;
            pipeline_registers_write.EX_MEM_register.Store_imm = pipeline_registers_read.ID_EX_register.Store_imm;
            pipeline_registers_write.EX_MEM_register.Store_op = pipeline_registers_read.ID_EX_register.Store_op;
            pipeline_registers_write.EX_MEM_register.ALU_result = ALU_result;
            pipeline_registers_write.EX_MEM_register.RegFile_op = pipeline_registers_read.ID_EX_register.RegFile_op;
            pipeline_registers_write.EX_MEM_register.valid = 1;


        }
        else {
            pipeline_registers_write.EX_MEM_register.valid = 0;
        }
        cout<<endl;
    }

    void Memory(L2Cache &l2cache, RAM& ram) {
        cout<<"MEM VALID "<<pipeline_registers_read.EX_MEM_register.valid<<endl;
        if (pipeline_registers_read.EX_MEM_register.valid == 1) {
            Memory_op memory_op = pipeline_registers_read.EX_MEM_register.Memory_op;
            Memory_data_type memory_data_type = pipeline_registers_read.EX_MEM_register.Memory_data_type;
            uint32_t ALU_result = pipeline_registers_read.EX_MEM_register.ALU_result;
            pipeline_registers_write.MEM_WB_register.data = l1_cache.Load(memory_op, memory_data_type,ALU_result,l2cache, ram); //Remember to connect this to I/O

            //pass on data from EX/MEM Register
            pipeline_registers_write.MEM_WB_register.ALU_result = pipeline_registers_read.EX_MEM_register.ALU_result;
            pipeline_registers_write.MEM_WB_register.rd_addr = pipeline_registers_read.EX_MEM_register.rd_addr;
            pipeline_registers_write.MEM_WB_register.rs1_val = pipeline_registers_read.EX_MEM_register.Store_imm;
            pipeline_registers_write.MEM_WB_register.Store_op = pipeline_registers_read.EX_MEM_register.Store_op;
            pipeline_registers_write.MEM_WB_register.RegFile_op = pipeline_registers_read.EX_MEM_register.RegFile_op;
            pipeline_registers_write.MEM_WB_register.Memory_op = pipeline_registers_read.EX_MEM_register.Memory_op;
            pipeline_registers_write.MEM_WB_register.command_PC_value = pipeline_registers_read.EX_MEM_register.command_PC_value;
            pipeline_registers_write.MEM_WB_register.valid = 1;
        }
        else {
            pipeline_registers_write.MEM_WB_register.valid = 0;
        }
        cout<<endl;
    }

    void WriteBack(L2Cache& l2cache, RAM& ram) {
        cout<<"WB VALID "<<pipeline_registers_read.MEM_WB_register.valid<<endl;
        if (pipeline_registers_read.MEM_WB_register.valid == 1) {
            uint32_t ALU_result = pipeline_registers_read.MEM_WB_register.ALU_result;
            Store_op Store_op = pipeline_registers_read.MEM_WB_register.Store_op;
            uint32_t Store_addr = ALU_result;
            uint32_t rd_addr = pipeline_registers_read.MEM_WB_register.rd_addr;
            uint32_t data = pipeline_registers_read.MEM_WB_register.data;
            RegFile_op RegFile_op = pipeline_registers_read.MEM_WB_register.RegFile_op;
            uint32_t command_PC_value = pipeline_registers_write.MEM_WB_register.command_PC_value;
            uint32_t rs2_val = pipeline_registers_read.MEM_WB_register.rs2_val;

            registerFile.operate(RegFile_op,data,ALU_result,rd_addr,command_PC_value);
            l1_cache.Store(Store_op,Store_addr,data, l2cache, ram);
        }
    }


    void Step(L2Cache &l2cache, RAM& ram) {
        Fetch(l2cache,ram);
        Decode();
        Execution();
        Memory(l2cache,ram);
        WriteBack(l2cache,ram);

        //sync pipeline registers according to enable signals
        if (pipeline_registers_write.IF_ID_register.enable == 1) {
            pipeline_registers_read.IF_ID_register = pipeline_registers_write.IF_ID_register;
        }
        if (pipeline_registers_write.ID_EX_register.enable == 1) {
            pipeline_registers_read.ID_EX_register = pipeline_registers_write.ID_EX_register;
        }
        if (pipeline_registers_write.EX_MEM_register.enable == 1) {
            pipeline_registers_read.EX_MEM_register = pipeline_registers_write.EX_MEM_register;
        }
        if (pipeline_registers_write.MEM_WB_register.enable == 1) {
            pipeline_registers_read.MEM_WB_register = pipeline_registers_write.MEM_WB_register;
        }
    }
};

#endif //RISC_V_CPU_SIMULATOR_CPUCORE_H

