#ifndef __PDP11_MEM_
#define __PDP11_MEM_
#include<cerrno>
#include"timings.h"

namespace PDP11 {

class Memory {
        uint8_t _mem;
        size_t _size;
        public:
                Memory(size_t size)
                {
                        _mem = new uint8_t[size];
                        _size = size;
                }
                ~Memory()
                {
                        delete [] _mem;
                }
                int read(uint16_t addr, uint16_t* dest)
                {
                        if (addr > size || (addr & 1))
                        {
                                return -EFAULT;
                        }
                        *dest = _mem[addr];
                        return timings()->get_mem_r();
                }
                int write(uint16_t addr, uint16t* src)
                {
                        if (addr > size || (addr & 1))
                        {
                                return -EFAULT;
                        }
                        _mem[addr] = *src;
                        return timings()->get_mem_w();
                }
};

};

#endif
