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
       return 0;
}

int PDP11::op_addr(Instruction i, uint32_t* addr, int n) {
        int op;
        int mod;
        int ret = 0;
        uint32_t temp1, temp2, temp, x;
        uint16_t eff;

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

        if (mod == -1)
                mod = MREG;

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
                        eff = temp1;
                        eff += (uint16_t) temp2;
                        *addr = eff;
                        return ret;

                case MINDI:
                        ret += mem.read(MEM_SIZE + 14, &temp1);
                        ret += mem.read(temp1, &temp2);
                        ret += mem.incr(MEM_SIZE + 14, 2);
                        ret += mem.read(MEM_SIZE + op * 2, &temp1);
                        ret += mem.read((temp1 + temp2) & 0xFFFF, addr);
                        return ret;
                default:
                        abort();
                        break;
           }
        return -1;
}

int PDP11::interrupt(int type) {
        uint16_t handler;
        uint16_t pc;
        uint16_t sp;
        if (type == IILL) {
                 mem.read(010, &handler);
        } else if (type == IFAULT) {
                 mem.read(004, &handler);
        } else if (type == IEMT) {
                 mem.read(030, &handler);
        } else if (type == IIOT) {
                 mem.read(020, &handler);
        }

        mem.read(MEM_SIZE + 12, &sp);
        mem.read(MEM_SIZE + 14, &pc);
        mem.write(sp, &pc);
        mem.write(MEM_SIZE + 14, &handler);
        mem.incr(MEM_SIZE + 12, -2);
        return 0;
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
        long res = -1;

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
                        _psw.psw.z = 1;
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
                        _psw.psw.z = !!!tmp1;
                        _psw.psw.n = !!(tmp1 & (1 << 15));
                        _psw.psw.v = _psw.psw.n + _psw.psw.c;
                        ret += mem.write(ea1, &tmp1);
                        break;
                case ASL:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        _psw.psw.c = !!(tmp1 & (1 << 15));
                        tmp1 <<= 1;
                        _psw.psw.z = !!!tmp1;
                        _psw.psw.n = !!(tmp1 & (1 << 15));
                        _psw.psw.v = _psw.psw.n + _psw.psw.c;
                        ret += mem.write(ea1, &tmp1);
                        break;
                case ASH:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1); 
                        ret += op_addr(cur, &ea2, 2);
                        ret += mem.read(ea2, &tmp2); 

                        {
                        uint16_t i = 0;
                        tmp1 &= 0077;
                        if (tmp1 & (1 << 5)) {
                                for (i = 0; i <= 32; ++i) {
                                        if (((i + tmp1) & 0x1F) == 0) {
                                                goto end1;
                                        }
                                }
end1:
                                _psw.psw.c = !!(tmp2 & (1 << i)); // ???
                                tmp2 >>= i;
                        } else {
                                tmp2 <<= (tmp1 & 0x1F);
                        }
                        ret++;
                        }

                        tmp1 <<= 1;
                        _psw.psw.z = !!!tmp2;
                        _psw.psw.n = !!(tmp2 & (1 << 15));
                        _psw.psw.v = !!(tmp2 & (1 << 15)); //Incorrect
                        ret += mem.write(ea2, &tmp2);
                        break;
                case TST:
                        ret += op_addr(cur, &ea1, 1);
                        ret += mem.read(ea1, &tmp1);
                        ret++;
                        _psw.psw.c = 0;
                        _psw.psw.z = !!!tmp1;
                        _psw.psw.n = !!(tmp1 & (1 << 15));
                        _psw.psw.v = 0;
                        break;
                case MOV:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.write(ea2, &tmp1);
                        psw_value(tmp1);
                        _psw.psw.v = 0;
                        break;
                case CMP:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.read(ea2, &tmp2);
                        res = (long) tmp1 - (long) tmp2;
                        ret++;
                        _psw.psw.z = !!!res;
                        _psw.psw.n = (res < 0);
                        _psw.psw.v = (res > 0xFFFF || res < 0xFFFF);
                        _psw.psw.c = (res & (1<<15));
                        break;
                case SUB:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.read(ea2, &tmp2);
                        res = (long) tmp2 - (long) tmp1;
                        tmp1 = tmp2 - tmp1;
                        ret++;
                        ret += mem.write(ea2, &tmp1);
                        psw_value((uint16_t) res);
                        _psw.psw.c = !!(((short) res) & (1<<15));
                        break;
                case ADD:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.read(ea2, &tmp2);
                        res = (long) tmp2 + (long) tmp1;
                        tmp1 = tmp2 + tmp1;
                        ret++;
                        ret += mem.write(ea2, &tmp1);
                        psw_value((uint16_t) res);
                        _psw.psw.c = !!(((short) res) & (1<<15));
                        break;
                case BIS:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.read(ea2, &tmp2);
                        tmp1 = tmp2 | tmp1;
                        ret++;
                        ret += mem.write(ea2, &tmp1);
                        psw_bit(tmp1);
                        break;
                case BIC:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.read(ea2, &tmp2);
                        tmp1 = tmp2 & ~tmp1;
                        ret++;
                        ret += mem.write(ea2, &tmp1);
                        psw_bit(tmp1);
                        break;
                case BIT:
                        ret += op_addr(cur, &ea1, 2);
                        ret += mem.read(ea1, &tmp1);
                        ret += op_addr(cur, &ea2, 1);
                        ret += mem.read(ea2, &tmp2);
                        tmp1 = tmp2 & tmp1;
                        ret++;
                        psw_bit(tmp1);
                        break;

                case BEQ:
                        if (_psw.psw.z == 0) {
                                break;
                        }
                        goto branch;
                case BPL:
                        if (_psw.psw.n != 0) {
                                break;
                        }
                        goto branch;
                case BMI:
                        if (_psw.psw.n == 0) {
                                break;
                        }
                        goto branch;
                case BVC:
                        if (_psw.psw.v != 0) {
                                break;
                        }
                        goto branch;
                case BVS:
                        if (_psw.psw.v == 0) {
                                break;
                        }
                        goto branch;
                case BHIS:
                        if (_psw.psw.c != 0) {
                                break;
                        }
                        goto branch;
                case BLO:
                        if (_psw.psw.c == 0) {
                                break;
                        }
                        goto branch;
                case BGE:
                        if ((_psw.psw.n | _psw.psw.v) != 0) {
                                break;
                        }
                        goto branch;
                case BLT:
                        if ((_psw.psw.n | _psw.psw.v) == 0) {
                                break;
                        }
                        goto branch;
                case BGT:
                        if ((_psw.psw.z | _psw.psw.n | _psw.psw.v) != 0) {
                                break;
                        }
                        goto branch;
                case BLE:
                        if ((_psw.psw.z | _psw.psw.n | _psw.psw.v) == 0) {
                                break;
                        }
                        goto branch;
                case BHI:
                        if ((_psw.psw.c & _psw.psw.z) != 0) {
                                break;
                        }
                        goto branch;
                case BLOS:
                        if ((_psw.psw.c & _psw.psw.z) == 0) {
                                break;
                        }
                        goto branch;
                case BNE:
                        if (_psw.psw.z != 0) {
                                break;
                        }
branch:
                        ret++;
                case BR:
                        {
                        int8_t tb = cur._op1;
                        ret += mem.read(MEM_SIZE + 14, &tmp2);
                        int sb = (int) tmp2;
                        sb += tb * 2;
                        tmp2 = sb;
                        ret += mem.write(MEM_SIZE + 14, &tmp2);
                        break;
                        }
                case JSR:
                        {
                        uint16_t sp;
                        uint16_t old_pc;
                        ret += op_addr(cur, &ea1, 1);
                        tmp1 = ea1;
                        ret += mem.read(MEM_SIZE + cur._op2  * 2, &tmp2);
                        ret += mem.read(MEM_SIZE + 14, &old_pc);

                        ret += mem.incr(MEM_SIZE + 12, -2);
                        ret += mem.read(MEM_SIZE + 12, &sp);
                        ret += mem.write(sp, &tmp2);
                        ret += mem.write(MEM_SIZE + cur._op2  * 2, &old_pc);

                        ret += mem.write(MEM_SIZE + 14, &tmp1);
                        }
                        break;
                case RTS:
                        {
                        uint16_t sp;
                        uint16_t old_pc;
                        ret += mem.read(MEM_SIZE + cur._op1  * 2, &tmp1);
                        ret += mem.write(MEM_SIZE + 14, &tmp1);

                        ret += mem.read(MEM_SIZE + 12, &sp);
                        ret += mem.read(sp, &tmp2);
                        ret += mem.incr(MEM_SIZE + 12, 2);

                        ret += mem.write(MEM_SIZE + cur._op1  * 2, &tmp2);
                        }
                        break;
                 default:
                        interrupt(IILL);
                        break;
        }

        if (mem.get_violation() == true) {
                mem.set_violation(false);
                interrupt(IFAULT);
        }

        return ret;
}

PDP11::PDP11() {
        _state = HALTED;
        timer = 0;
        vram_mode = 0;
        _psw.raw = 0;
}
