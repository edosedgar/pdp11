#ifndef __PDP11_MEM_
#define __PDP11_MEM_
#include<cerrno>
#include<cstring>
#include<cstdint>
#include<cstddef>
#include<cstdlib>
#include<cstdio>
#include"timings.hpp"
//#include"pdp11.hpp"

//namespace PDP11 {
#define ROM_SIZE 0x6000
#define VRAM_SIZE 0x4000
#define RAM_SIZE 0x4000
#define IO_SIZE 0x2000

#define MEM_SIZE ((uint32_t) 0x10000)

typedef union {
        struct __attribute((packed))__ {
                uint8_t ram[RAM_SIZE];
                uint8_t vram[VRAM_SIZE];
                uint8_t rom[ROM_SIZE];
                uint8_t io[IO_SIZE];
                uint16_t regs[8];
        } map;
        uint8_t mem[MEM_SIZE + 16];
        uint16_t wmem[(MEM_SIZE / 2) + 8];
} MemoryMap;

class Memory {

        size_t _size;
        bool _violated;
        int _vram_mode;
        bool _vram_dirty;
        MemoryMap* _mem;
        public:
                Memory()
                {
                        _mem = new MemoryMap;
                        _violated = false;
                }
                ~Memory()
                {
                        delete _mem;
                }
                int reset()
                {
                        memset(_mem->map.ram, 0, RAM_SIZE);
                        memset(_mem->map.vram, 0, VRAM_SIZE);
                        memset(_mem->map.regs, 0, 16);
                        _mem->wmem[(MEM_SIZE / 2) + 7] = 0x8000;
                        return 0;
                }
                int read(uint32_t addr, uint32_t* dest);
                int write(uint32_t addr, uint32_t* src);
                int read(uint32_t addr, uint16_t* dest);
                int write(uint32_t addr, uint16_t* src);
                int incr(uint32_t addr, int n) {
                        uint16_t temp;
                        int ret = 0;
                        ret += this->read(addr, &temp);
                        if (n > 0) {
                                temp += n;
                        } else {
                                temp -= ((uint16_t) (-n));
                        }
                        ret++;
                        ret += this->write(addr, &temp);
                        return ret;
                }
                bool get_violation() {
                        return _violated;
                }
                void set_violation(bool arg) {
                        _violated = arg;
                }
                int rom_load(uint8_t* code, size_t size) {
                        memset(_mem->map.rom, 0, ROM_SIZE);
                        memcpy(_mem->map.rom, code, size);
                        return 0;
                }
};

//};

#endif
