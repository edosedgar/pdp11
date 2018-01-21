#include"memory.hpp"

Timings* Timings::_timings = 0;

int Memory::read(uint32_t addr, uint32_t* dest) {
        if (addr >= sizeof(MemoryMap))
        {
                abort();
        }
        *dest = (0xFFFF & (uint32_t) _mem->wmem[addr / 2]);
        if (addr < MEM_SIZE) {
                return Timings::timings()->get_mem_r();
        } 
        return Timings::timings()->get_reg();
}

int Memory::write(uint32_t addr, uint32_t* src)
{
        if (addr >= sizeof(MemoryMap))
        {
                return -EFAULT;
        }
        if (addr < ROM_SIZE) {
                _violated = true;
                return Timings::timings()->get_mem_r();
        }
        if (addr < MEM_SIZE) {
                return Timings::timings()->get_mem_r();
        } 
        _mem->wmem[addr / 2] = (uint16_t) (0xFFFF & *src);
        return Timings::timings()->get_reg();
}

int Memory::read(uint32_t addr, uint16_t* dest)
{
        if (addr >= sizeof(MemoryMap))
        {
                abort();
        }
        *dest = _mem->wmem[addr / 2];
        if (addr < MEM_SIZE) {
                return Timings::timings()->get_mem_r();
        } 
        return Timings::timings()->get_reg();
}

int Memory::write(uint32_t addr, uint16_t* src)
{
        if (addr >= sizeof(MemoryMap))
        {
                return -EFAULT;
        }
        if (addr < ROM_SIZE) {
                _violated = true;
                return Timings::timings()->get_mem_r();
        }
        if (addr < MEM_SIZE) {
                return Timings::timings()->get_mem_r();
        } 
        _mem->wmem[addr / 2] = *src;
        return Timings::timings()->get_reg();
}
