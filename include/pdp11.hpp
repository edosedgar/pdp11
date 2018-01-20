#ifndef __PDP11__
#define __PDP11__

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>
#include "memory.hpp"
#include "decoder.hpp"
#include "registers.hpp"

namespace PDP11 {

class PDP11 {
public:
        int PDP11();
        void ~PDP11();
        int load(uint16_t* code, size_t size);
        int exec();
        std::string info_registers();
        std::string info_display();
        std::string info_instruction(uint16_t addr);
private:
        Decoder dec;
        Memory mem;        
        Register regs[8];
        Register pc;
        uint64_t timer;
        Register psw;
        std::vector<uint16_t> breakpoints;
};

};

#endif
