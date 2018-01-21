#ifndef __PDP11_DEC_
#define __PDP11_DEC_

#include<string>
#include<cstdint>
#include<cstddef>

enum modes {
        MREG = 00,
        MREGI = 01,
        MINC = 02,
        MINCI = 03,
        MDEC = 04,
        MDECI = 05,
        MIND = 06,
        MINDI = 07
};

enum r7modes {
        R7IMM = 02,
        R7ABS = 03,
        R7RELD = 06,
        R7RELI = 07
};

enum instructions {
        HALT = 0,
        WAIT,
        RESET,
        NOP,
        CLR,
        INC,
        DEC,
        ADC,
        SBC,
        TST,
        NEG,
        COM,
        ROR,
        ROL,
        ASR,
        ASL,
        SWAB,
        SXT,
        MUL,
        DIV,
        ASH,
        ASHC,
        XOR,
        MOV,
        ADD,
        SUB,
        CMP,
        BIS,
        BIC,
        BIT,
        BR,
        BNE,
        BEQ,
        BPL,
        BMI,
        BVC,
        BVS,
        BHIS,
        BCC,
        BLO,
        BCS,
        BGE,
        BLT,
        BGT,
        BLE,
        BHI,
        BLOS,
        JMP,
        SOB,
        JSR,
        RTS,
        RTI,
        TRAP,
        BPT,
        IOT,
        EMT,
        RTT,
        SPL,
        CLC,
        CLV,
        CLZ,
        CLN,
        SEC,
        SEV,
        SEZ,
        SEN,
        SSS,
        SCC
};
//
//namespace PDP11 {

typedef union {
        uint16_t raw;
        struct __attribute__((packed)) {
                unsigned opcode:16;
        } zero_op;
        struct __attribute__((packed)) {
                unsigned op:3;
                unsigned mod:3;
                unsigned opcode:9;
                unsigned bit:1;
        } one_op;
        struct __attribute__((packed)) {
                unsigned op:3;
                unsigned mod:3;
                unsigned reg:3;
                unsigned opcode:7;
        } oneh_op;
        struct __attribute__((packed)) {
                unsigned op1:3;
                unsigned mod1:3;
                unsigned op2:3;
                unsigned mod2:3;
                unsigned opcode:3;
                unsigned bit:1;
        } two_op;
        struct __attribute__((packed)) {
                unsigned addr:8;
                unsigned opcode:8;
        } branch;
        struct __attribute__((packed)) {
                unsigned hop:3;
                unsigned opcode:13;
        } h_op;
                 
} InstrRaw;

class Instruction {
        InstrRaw _offsets[2];
        std::string get_str();
        public:
        int _type;
        int _op1;
        int _mod1;
        int _op2;
        int _mod2;
        int _bit;
        size_t _size; //In words
        std::string str;
        Instruction(InstrRaw* arr, size_t n);
        size_t get_size() {
                return _size;
        }
};

class Decoder {
        public:
                Instruction decode(uint16_t* ins, size_t n)
                {
                        return Instruction((InstrRaw*) ins, n);
                }
};

//};

#endif
