#ifndef __PDP11__
#define __PDP11__

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>
#include "memory.hpp"
#include "decoder.hpp"

typedef union {
        struct __attribute((packed))__ {
                unsigned c:1;
                unsigned v:1;
                unsigned z:1;
                unsigned n:1;
                unsigned t:1;
                unsigned spl:3;
                unsigned kernel:8;
        } psw;
        uint16_t raw;
} PSW;

static void comp_check_func () {
        int a[MEM_SIZE - ROM_SIZE - VRAM_SIZE - RAM_SIZE - IO_SIZE];
        int b[-(MEM_SIZE - ROM_SIZE - VRAM_SIZE - RAM_SIZE - IO_SIZE)];
        int c[-(MEM_SIZE + 16 - sizeof(MemoryMap))];
        int d[MEM_SIZE  + 16 - sizeof(MemoryMap)];
        return;
}

enum Interrupts {
        IILL = 0,
        IFAULT,
        IIOT,
        IEMT
};

//namespace PDP11 {

enum state {
        HALTED = 0,
        RUNNING = 1
};

class PDP11 {
public:
        PDP11();
        ~PDP11() {
        }
        int load(uint8_t* code, size_t size);
        int exec();
        int reset();
        std::string info_registers() {
                uint16_t tmp1;
                auto out = std::string("");
                for (int i = 0; i < 8; i++) {
                        mem.read(MEM_SIZE + i * 2, &tmp1);
                        out += "R";
                        out += std::to_string(i);
                        out += "=";
                        out += std::to_string(tmp1);
                        out += " ";
                }
                out += "PSW=";
                out += std::to_string(_psw.raw);
                return out;
        }
        std::string info_display();
        std::string info_instruction(uint16_t addr) {
                uint16_t ins[3];
                mem.read(addr, ins);
                mem.read(addr + 1, ins + 1);
                mem.read(addr + 2, ins + 2);
                Instruction cur = dec.decode(ins, 3);
                std::string out = std::string("");
                out += std::to_string(2 * cur.get_size());
                out += " ";
                out += cur.str;
                return out;
        }
        int get_state() {
                return _state;
        }
        uint16_t get_pc() {
                uint16_t pc;
                mem.read(MEM_SIZE + 14, &pc);
                return pc;
        }
private:
        Decoder dec;
        Memory mem;
        unsigned _state;
        uint64_t timer;
        int vram_mode;
        PSW _psw;
        int op_addr(Instruction i, uint32_t* addr, int n);
        std::vector<uint16_t> breakpoints;
        int psw_value(uint32_t res)
        {
                _psw.psw.z = !!!res;
                _psw.psw.n = !!(res & (1 << 15));
                _psw.psw.v = !!(res & 0xFFFF0000);
                return 0;
        }
        int psw_bit(uint32_t res)
        {
                _psw.psw.v = 0;
                _psw.psw.z = !!!res;
                _psw.psw.n = !!(res & (1 << 15));
                return 0;
        }
        int interrupt(int type);
};

//};

#endif
