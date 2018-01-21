#include "pdp11.hpp"
#include <decoder.hpp>
#include <cstring>
#include <cerrno>
#include <iostream>

//namespace PDP11 {
int PDP11::reset() {
       _state = HALT;
       _psw.raw = 0;
       mem.reset();
       vram_mode = 0;
       timer = 0;
}

int PDP11::op_addr(Instruction i, uint32_t* addr, int n) {
        int op;
        int mod;
        int ret = 0;
        uint32_t temp1, temp2, temp, x;

        if (n == 1) {
                op = i._op1;
                mod = i._mod1;
        } else if (n == 2) {
                op = i._op2;
                mod = i._mod2;
        } else {
                return -EINVAL;
        }

        if (op == 07) {
                switch (mod) {
                        case R7IMM:
                                ret += mem.read(MEM_SIZE + 14, addr);
                                ret += mem.incr(MEM_SIZE + 14, 2);
                                return ret;
                        case R7ABS:
                                ret += mem.read(MEM_SIZE + 14, &temp);
                                ret += mem.read(temp, addr);
                                ret += mem.incr(MEM_SIZE + 14, 2);
                                return ret;
                        case R7RELD:
                                ret += mem.read(MEM_SIZE + 14, &temp);
                                ret += mem.read(temp, &x);
                                ret += mem.incr(MEM_SIZE + 14, 2);
                                ret += mem.read(MEM_SIZE + 14, &temp);
                                *addr = (uint16_t) (x + temp);
                                return ret;
                        case R7RELI:
                                ret += mem.read(MEM_SIZE + 14, &temp);
                                ret += mem.read(temp, &x);
                                ret += mem.incr(MEM_SIZE + 14, 2);
                                ret += mem.read(MEM_SIZE + 14, &temp);
                                ret += mem.read( (x + temp) & 0xFFFF , addr);
                                return ret;
                        default:
                                break;
                }
        }

        switch (mod) {
                case MREG:
                        *addr = MEM_SIZE + op * 2;
                        return ret;

                case MREGI:
                        ret += mem.read(MEM_SIZE + op * 2, addr);
                        return ret;

                case MINC:
                        ret += mem.read(MEM_SIZE + op * 2, addr);
                        if (i._bit == 1) {
                                ret += mem.incr(MEM_SIZE + op * 2, 1);
                        } else {
                                ret += mem.incr(MEM_SIZE + op * 2, 2);
                        }
                        return ret;

                case MINCI:
                        ret += mem.read(MEM_SIZE + op * 2, &temp);
                        ret += mem.read(temp, addr);
                        if (i._bit == 1) {
                                ret += mem.incr(MEM_SIZE + op * 2, 1);
                        } else {
                                ret += mem.incr(MEM_SIZE + op * 2, 2);
                        }
                        return ret;

                case MDEC:
                        if (i._bit == 1) {
                                ret += mem.incr(MEM_SIZE + op * 2, -1);
                        } else {
                                ret += mem.incr(MEM_SIZE + op * 2, -2);
                        }
                        ret += mem.read(MEM_SIZE + op * 2, addr);
                        return ret;

                case MDECI:
                        if (i._bit == 1) {
                                ret += mem.incr(MEM_SIZE + op * 2, -1);
                        } else {
                                ret += mem.incr(MEM_SIZE + op * 2, -2);
                        }
                        ret += mem.read(MEM_SIZE + op * 2, &temp);
                        ret += mem.read(temp, addr);
                        return ret;

                case MIND:
                        ret += mem.read(MEM_SIZE + 14, &temp1);
                        ret += mem.read(temp1, &temp2);
                        ret += mem.incr(MEM_SIZE + 14, 2);
                        ret += mem.read(MEM_SIZE + op * 2, &temp1);
                        *addr = (uint16_t) temp1 + temp2;
                        return ret;

                case MINDI:
                        ret += mem.read(MEM_SIZE + 14, &temp1);
                        ret += mem.read(temp1, &temp2);
                        ret += mem.incr(MEM_SIZE + 14, 2);
                        ret += mem.read(MEM_SIZE + op * 2, &temp1);
                        ret += mem.read((temp1 + temp2) & 0xFFFF, addr);
                        return ret;
                default:
                        break;
           }
}

int PDP11::load(uint8_t* code, size_t size) {
        if (size > ROM_SIZE) {
                return -ENOMEM;
        }
        
        return mem.rom_load(code, size);
}

int PDP11::exec() {
        uint16_t ins[3];
        uint16_t pc;

        int ret = 0;
        uint32_t ea1 = -1;
        uint32_t ea2 = -1;
        uint16_t tmp1 = -1;
        uint16_t tmp2 = -1;

        ret += mem.read(MEM_SIZE + 14, &pc);
        ret += mem.read(pc, ins);
        mem.read(pc + 2, ins + 1);
        mem.read(pc + 4, ins + 2);
        Instruction cur = dec.decode(ins, 3);

        ret += mem.incr(MEM_SIZE + 14, 2);
        ret += mem.read(MEM_SIZE + 14, &pc);
        //std::cerr << "I am in exec of instr " << cur.str << " pc is " << pc << '\n';

        _state = RUNNING;

        switch (cur._type) {
                case HALT:
                case WAIT:
                        reset();
                        _state = HALTED;
                        ret++;
                        break;
                case RESET:
                case NOP:
                        ret++;
                        break;
                case CLR:
                        ret += op_addr(cur, &ea1, 1);
                        tmp1 = 0;
                        ret += mem.write(ea1, &tmp1);
                        _psw.psw.n = 0;
                        _psw.psw.v = 0;
                        _psw.psw.c = 0;
                        _psw.psw.z = 0;
                        break;
                case INC:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        tmp1++;
                        ret += mem.write(ea1, &tmp1);
                        psw_value(tmp1);
                        break;
                case DEC:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        tmp1--;
                        ret += mem.write(ea1, &tmp1);
                        psw_value(tmp1);
                        break;
                case ADC:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        tmp1 +=_psw.psw.c;
                        ret += mem.write(ea1, &tmp1);
                        psw_value(tmp1);
                        break;
                case SBC:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        tmp1 -=_psw.psw.c;
                        ret += mem.write(ea1, &tmp1);
                        psw_value(tmp1);
                        break;
                case ASR:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        _psw.psw.c = tmp1 & 1;
                        tmp1 >>= 1;
                        _psw.psw.z = !!tmp1;
                        _psw.psw.n = tmp1 & (1 << 15);
                        _psw.psw.v = _psw.psw.n + _psw.psw.c;
                        ret += mem.write(ea1, &tmp1);
                        break;
                case ASL:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        _psw.psw.c = tmp1 & (1 << 15);
                        tmp1 <<= 1;
                        _psw.psw.z = !!tmp1;
                        _psw.psw.n = tmp1 & (1 << 15);
                        _psw.psw.v = _psw.psw.n + _psw.psw.c;
                        ret += mem.write(ea1, &tmp1);
                        break;
                case MOV:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.write(ea2, &tmp1);
                        psw_value(tmp1);
                        _psw.psw.v = 0;
                        break;
                default:
                        break;
        }

        return ret;
}

PDP11::PDP11() {
        _state = HALTED;
        timer = 0;
        vram_mode = 0;
        _psw.raw = 0;
}
