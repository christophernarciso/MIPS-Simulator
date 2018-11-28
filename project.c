#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {
    /* A and B are inputs to the ALU
     * Output the result Z to #ALUresult
     * Assign #Zero to 1 if the result is zero; otherwise, assign 0.
     * */
    switch (ALUControl) {
        case '0':
            // Z = A + B
            *ALUresult = A + B;
            break;
        case '1':
            // Z = A - B
            *ALUresult = A - B;
            break;
        case '2':
            // A < B ? Z = 1 : Z = 0 when A and B are signed
            *ALUresult = (signed) A < (signed) B ? 1 : 0;
            break;
        case '3':
            // A < B ? Z = 1 : Z = 0 when A and B are unsigned
            *ALUresult = A < B ? 1 : 0;
            break;
        case '4':
            // Z = A AND B
            *ALUresult = A & B;
            break;
        case '5':
            // Z = A OR B
            *ALUresult = A | B;
            break;
        case '6':
            // Shift left B by 16 bits;
            *ALUresult = B << 16;
            break;
        case '7':
            // Z = NOT A
            *ALUresult = ~A;
            break;
    }

    *Zero = (*ALUresult == 0) ? '1' : '0';
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
    /*
     * 1. Fetch the instruction addressed by PC from Mem and write it to instruction.
     * 2. Return 1 if a halt condition occurs; otherwise, return 0.
     * Memory is byte addressed!!
     */
    if (PC % 4 == 0) {
        /*
         * PC is the index of the Mem[], where the address is. But to access it we shift right by 2.
         * Mem[PC >> 2] is the decimal value of the instruction that was in Hex in the file.
         */
        *instruction = Mem[PC >> 2];
        return 0;
    }

    return 1;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3,
                           unsigned *funct, unsigned *offset, unsigned *jsec) {
    /*
     * 1. Partition instruction into several parts (op, r1, r2, r3, funct, offset, jsec).
     * 2. Read line 41 to 47 of spimcore.c for more information.
     *
     * Information:
     *  op,         instruction [31-26]
     *  r1,         instruction [25-21]
     *  r2,         instruction [20-16]
     *  r3,         instruction [15-11]
     *  funct,      instruction [5-0]
     *  offset,     instruction [15-0]
     *  jsec;       instruction [25-0]
     */


    // The six most significant bits [bits 31-26] for all instructions types are the opcode, we send these bits to the control unit.
    *op = instruction >> 26;

    /* Grab the registers used in this operation.
     * The next five bits [bits 25-21] & same for [bits 20-16] are sent to the register file to identify the first register & second register we will be reading from.
     * r1,         instruction [25-21]
     * r2,         instruction [20-16]
     * The next five bits [bits 15-11] are sent to the write register multiplexor.
     * r3,         instruction [15-11]
     */
    *r1 = instruction >> 21 & 0x1F;
    *r2 = instruction >> 16 & 0x1F;
    *r3 = instruction >> 11 & 0x1F;

    // Function code for R-type instructions. instruction [5-0] Last 6 bits
    *funct = instruction & 0x3F;

    // Used for both immediate values and branching. instruction [15-0] Last 16bits
    *offset = instruction & 0xFFFF;

    // Address to jump to incase of an event to jump. instruction [25-0] Last 26 bits
    *jsec = instruction & 0x3FFFFFF;
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
    /*
     * 1. Decode the instruction using the opcode (op).
     * 2. Assign the values of the control signals to the variables in the structure controls (See spimcore.h file).
     *
     * In this stage, we send the opcode to the Control Unit and determine the control signals for the rest of the datapath.
     *
     * The meanings of the values of the control signals:
     * - For MemRead, MemWrite or RegWrite, the value 1 means that enabled, 0 means that disabled, 2 means “don’t care”.
     * - For RegDst, Jump, Branch, MemtoReg or ALUSrc, the value 0 or 1 indicates the selected path of the multiplexer; 2 means “don’t care”.
     *
     * 3. Return 1 if a halt condition occurs; otherwise, return 0.
     *
     * Information:
     * MemRead  - Operation needs to read from memory
     * MemWrite - Operation needs to write to memory
     * RegWrite - Operation needs to write to a register
     * RegDst   - Operation is storing in (i-type / branch) ? 0 : 1 (r-type)
     * Jump     - Operation jumps
     * Branch   - Operation branches
     * MemtoReg - Operation needs to write to a register from memory
     * ALUSrc   - Operation is 0 - choose read data2 : 1 - choose the sign extended value
     * ALUOp    - Operation opcode given to the ALU: 0 - add or don't care, 1 - sub, 2 - slt, 3 - sltu, 4 - and, 5 - or, 6 - sll, 7 - r-type
     *
     * usigned op : https://opencores.org/project/plasma/opcodes
     */
    switch (op) {
        case 0:
            // R-type [Add, Subtract, And, Or, Set Less Than, Set Less Than Unsigned]
            controls->RegDst = '1';
            controls->Jump = '0';
            controls->Branch = '0';
            controls->MemRead = '0';
            controls->MemtoReg = '0';
            controls->ALUOp = '7';
            controls->MemWrite = '0';
            controls->ALUSrc = '0';
            controls->RegWrite = '1';
            break;
        case 2:
            // Jump
            controls->RegDst = '2';
            controls->Jump = '1';
            controls->Branch = '0';
            controls->MemRead = '0';
            controls->MemtoReg = '2';
            controls->ALUOp = '0';
            controls->MemWrite = '0';
            controls->ALUSrc = '1';
            controls->RegWrite = '0';
            break;
        case 4:
            // Branch Equal To
            controls->RegDst = '2';
            controls->Jump = '0';
            controls->Branch = '1';
            controls->MemRead = '0';
            controls->MemtoReg = '2';
            controls->ALUOp = '1';
            controls->MemWrite = '0';
            controls->ALUSrc = '0';
            controls->RegWrite = '0';
            break;
        case 8:
            // Add Immediate
            controls->RegDst = '0';
            controls->Jump = '0';
            controls->Branch = '0';
            controls->MemRead = '0';
            controls->MemtoReg = '0';
            controls->ALUOp = '0';
            controls->MemWrite = '0';
            controls->ALUSrc = '1';
            controls->RegWrite = '1';
            break;
        case 10:
            // Set Less Than Immediate
            controls->RegDst = '0';
            controls->Jump = '0';
            controls->Branch = '0';
            controls->MemRead = '0';
            controls->MemtoReg = '0';
            controls->ALUOp = '2';
            controls->MemWrite = '0';
            controls->ALUSrc = '1';
            controls->RegWrite = '1';
            break;
        case 11:
            // Set Less Than Immediate Unsigned
            controls->RegDst = '0';
            controls->Jump = '0';
            controls->Branch = '0'; //set dont care? '2'?
            controls->MemRead = '0';
            controls->MemtoReg = '0';
            controls->ALUOp = '3';
            controls->MemWrite = '0';
            controls->ALUSrc = '1';
            controls->RegWrite = '1';
            break;
        case 15:
            // Load Upper Immediate
            controls->RegDst = '0';
            controls->Jump = '0';
            controls->Branch = '0';
            controls->MemRead = '0';
            controls->MemtoReg = '0';
            controls->ALUOp = '6';
            controls->MemWrite = '0';
            controls->ALUSrc = '1';
            controls->RegWrite = '1';
            break;
        case 35:
            // Load Word
            controls->RegDst = '0';
            controls->Jump = '0';
            controls->Branch = '0';
            controls->MemRead = '1';
            controls->MemtoReg = '1';
            controls->ALUOp = '0';
            controls->MemWrite = '0';
            controls->ALUSrc = '1';
            controls->RegWrite = '1';
            break;
        case 43:
            // Store Word
            controls->RegDst = '2';
            controls->Jump = '0';
            controls->Branch = '0';
            controls->MemRead = '0';
            controls->MemtoReg = '2';
            controls->ALUOp = '0';
            controls->MemWrite = '1';
            controls->ALUSrc = '1';
            controls->RegWrite = '0';
            break;
        default:
            return 1;
    }

    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
    /*
     * 1. Read the registers addressed by r1 and r2 from Reg, and write the read values to data1 and data2 respectively.
     */
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value) {
    /*
     * 1. Assign the sign-extended value of offset to extended_value.
     * send our 16 least significant bits to the sign-extension unit.
     * Check if the MSB is negative. If it is, fill with 1s else, fill with 0s
     */
    *extended_value = (offset >> 15) == 1 ? (offset | 0xFFFF0000) : (offset & 0x0000FFFF);
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc,
                   unsigned *ALUresult, char *Zero) {
    /*
     * 1. Apply ALU operations on data1, and data2 or extended_value (determined by ALUSrc).
     *    - ALUSrc Operation is 0 - choose read data2 : 1 - choose the sign extended value
     * 2. The operation performed is based on ALUOp and funct: https://opencores.org/project/plasma/opcodes
     * 3. Apply the function ALU(…).
     * 4. Output the result to ALUresult.
     * 5. Return 1 if a halt condition occurs; otherwise, return 0.
     */
    // Data we are operating on by ALUSrc
    data2 = ALUSrc == '1' ? extended_value : data2;

    // If ALUOp is an r-type instruction based on instruction_decode
    if (ALUOp == '7') {
        switch (funct) {
            case 32:
                // Add
                ALUOp = '0';
                break;
            case 34:
                // Subtract
                ALUOp = '1';
                break;
            case 36:
                // And
                ALUOp = '4';
                break;
            case 37:
                // Or
                ALUOp = '5';
                break;
            case 42:
                // Set Less Than signed
                ALUOp = '2';
                break;
            case 43:
                // Set Less Than Unsigned
                ALUOp = '3';
                break;
            default:
                return 1;
        }

        // Updated ALUOp sent to ALU(...)
        ALU(data1, data2, ALUOp, ALUresult, Zero);
        return 0;
    } else {
        switch (ALUOp) {
            case '0':
                // Add immediate, Store word, Load word, Jump
                ALU(data1, data2, ALUOp, ALUresult, Zero);
                return 0;
            case '1':
                // Branch Equal to
                ALU(data1, data2, ALUOp, ALUresult, Zero);
                return 0;
            case '2':
                // Set Less Than Immediate
                ALU(data1, data2, ALUOp, ALUresult, Zero);
                return 0;
            case '3':
                // Set Less Than Immediate Unsigned
                ALU(data1, data2, ALUOp, ALUresult, Zero);
                return 0;
            case '6':
                // Load Upper Immediate
                ALU(data1, data2, ALUOp, ALUresult, Zero);
                return 0;
            default:
                return 1;
        }
    }
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {
    /*
     * 1. Base on the value of MemWrite or MemRead to determine memory write operation or memory read operation.
     * 2. Read the content of the memory location addressed by ALUresult to memdata.
     * 3. Write the value of data2 to the memory location addressed by ALUresult.
     * 4. Return 1 if a halt condition occurs; otherwise, return 0.
     */
    if (MemRead == '1') {
        if (ALUresult % 4 != 0)
            return 1;
        *memdata = Mem[ALUresult >> 2];
    }

    if (MemWrite == '1') {
        if (ALUresult % 4 != 0)
            return 1;
        Mem[ALUresult >> 2] = data2;
    }

    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst,
                    char MemtoReg, unsigned *Reg) {
    /*
     * 1. Write the data (ALUresult or memdata) to a register (Reg) addressed by r2 or r3.
     */
    if (RegWrite == '1') {
        // Information comes from memory
        if (MemtoReg == '1')
            Reg[r2] = memdata;
        else if (MemtoReg == '0') {
            // Information comes from a register
            // If its an R-Type write to r3, else its an I-Type write to r2
            if (RegDst == '1')
                Reg[r3] = ALUresult;
            else
                Reg[r2] = ALUresult;
        }
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
    /*
     * 1. Update the program counter (PC).
     */
    // Always increment PC by 4.
    *PC += 4;

    // Branch signal and Zero signal further increments PC by the offset
    if (Branch == '1' && Zero == '1')
        *PC += extended_value << 2;

    // Jump signal sets PC using upper 4 bits of PC and left shift bits of jsec by 2.
    if (Jump == '1')
        *PC = (jsec << 2) | (*PC & 0xf000000);
}
