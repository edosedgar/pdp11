#include"decoder.hpp"

const char* instruction_set[SCC+1] = {
        "HALT",
        "WAIT",
        "RESET",
        "NOP",
        "CLR",
        "INC",
        "DEC",
        "ADC",
        "SBC",
        "TST",
        "NEG",
        "COM",
        "ROR",
        "ROL",
        "ASR",
        "ASL",
        "SWAB",
        "SXT",
        "MUL",
        "DIV",
        "ASH",
        "ASHC",
        "XOR",
        "MOV",
        "ADD",
        "SUB",
        "CMP",
        "BIS",
        "BIC",
        "BIT",
        "BR",
        "BNE",
        "BEQ",
        "BPL",
        "BMI",
        "BVC",
        "BVS",
        "BHIS",
        "BCC",
        "BLO",
        "BCS",
        "BGE",
        "BLT",
        "BGT",
        "BLE",
        "BHI",
        "BLOS",
        "JMP",
        "SOB",
        "JSR",
        "RTS",
        "RTI",
        "TRAP",
        "BPT",
        "IOT",
        "EMT",
        "RTT",
        "SPL",
        "CLC",
        "CLV",
        "CLZ",
        "CLN",
        "SEC",
        "SEV",
        "SEZ",
        "SEN",
        "SSS",
        "SCC"
};

Instruction::Instruction(InstrRaw* arr, size_t n) {
        _type = -1;
        _op1 = -1;
        _mod1 = -1;
        _op2 = -1;
        _mod2 = -1;
        _bit = -1;
        _size = 1;
        if (n > 1) {
                _offsets[0] = arr[1];
        }
        if (n > 2) {
                _offsets[1] = arr[2];
        }
        InstrRaw instr_raw = arr[0];
        switch (instr_raw.zero_op.opcode) {
                case 00:
                        _type = HALT;
                        break;
                case 02:
                        _type = RTI;
                        break;
                case 01:
                        _type = WAIT;
                        break;
                case 04:
                        _type = IOT;
                        break;
                case 05:
                        _type = RESET;
                        break;
                case 0240:
                        _type = NOP;
                        break;
                default:
                        break;
        }
        if (_type != -1) {
                str = get_str();
                return;
        }
        switch (instr_raw.one_op.opcode) {
                case 0050:
                        _type = CLR;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0052:
                        _type = INC;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0053:
                        _type = DEC;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0055:
                        _type = ADC;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0056:
                        _type = SBC;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0057:
                        _type = TST;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0054:
                        _type = NEG;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0051:
                        _type = COM;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0060:
                        _type = ROR;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0061:
                        _type = ROL;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 0062:
                        _type = ASR;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;


                case 0063:
                        _type = ASL;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        _bit = instr_raw.one_op.bit;
                        break;

                case 00003:
                        _type = SWAB;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        break;

                case 00067:
                        _type = SXT;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        break;

                case 00001:
                        _type = JMP;
                        _op1 = instr_raw.one_op.op;
                        _mod1 = instr_raw.one_op.mod;
                        break;
         }

        if (_type != -1) {
                str = get_str();
                return;
        }

        switch (instr_raw.oneh_op.opcode) {
                case 0070:
                        _type = MUL;
                        _op1 = instr_raw.oneh_op.op;
                        _mod1 = instr_raw.oneh_op.mod;
                        _op2 = instr_raw.oneh_op.reg;
                        break;

                case 0071:
                        _type = DIV;
                        _op1 = instr_raw.oneh_op.op;
                        _mod1 = instr_raw.oneh_op.mod;
                        _op2 = instr_raw.oneh_op.reg;
                        break;

                case 0072:
                        _type = ASH;
                        _op1 = instr_raw.oneh_op.op;
                        _mod1 = instr_raw.oneh_op.mod;
                        _op2 = instr_raw.oneh_op.reg;
                        break;

                case 0073:
                        _type = ASHC;
                        _op1 = instr_raw.oneh_op.op;
                        _mod1 = instr_raw.oneh_op.mod;
                        _op2 = instr_raw.oneh_op.reg;
                        break;

                case 0074:
                        _type = XOR;
                        _op1 = instr_raw.oneh_op.op;
                        _mod1 = instr_raw.oneh_op.mod;
                        _op2 = instr_raw.oneh_op.reg;
                        break;
                case 0004:
                        _type = JSR;
                        _op1 = instr_raw.oneh_op.op;
                        _mod1 = instr_raw.oneh_op.mod;
                        _op2 = instr_raw.oneh_op.reg;
                        break;
                case 0104:
                        if (instr_raw.oneh_op.reg > 3) {
                                _type = TRAP;
                        } else {
                                _type = EMT;
                        }
                        break;
                 default:
                        break;
         }
        if (_type != -1) {
                str = get_str();
                return;
        }

        switch (instr_raw.two_op.opcode) {
                case 01:
                        _type = MOV;
                        _op1 = instr_raw.two_op.op1;
                        _mod1 = instr_raw.two_op.mod1;
                        _op2 = instr_raw.two_op.op2;
                        _mod2 = instr_raw.two_op.mod2;
                        _bit = instr_raw.two_op.bit;
                        break;

                case 06:
                        if (instr_raw.two_op.bit == 0) {
                                _type = ADD;
                        } else {
                                _type = SUB;
                        }
                        _op1 = instr_raw.two_op.op1;
                        _mod1 = instr_raw.two_op.mod1;
                        _op2 = instr_raw.two_op.op2;
                        _mod2 = instr_raw.two_op.mod2;
                        break;

                case 02:
                        _type = CMP;
                        _op1 = instr_raw.two_op.op1;
                        _mod1 = instr_raw.two_op.mod1;
                        _op2 = instr_raw.two_op.op2;
                        _mod2 = instr_raw.two_op.mod2;
                        _bit = instr_raw.two_op.bit;
                        break;

                case 05:
                        _type = BIS;
                        _op1 = instr_raw.two_op.op1;
                        _mod1 = instr_raw.two_op.mod1;
                        _op2 = instr_raw.two_op.op2;
                        _mod2 = instr_raw.two_op.mod2;
                        _bit = instr_raw.two_op.bit;
                        break;

                case 04:
                        _type = BIC;
                        _op1 = instr_raw.two_op.op1;
                        _mod1 = instr_raw.two_op.mod1;
                        _op2 = instr_raw.two_op.op2;
                        _mod2 = instr_raw.two_op.mod2;
                        _bit = instr_raw.two_op.bit;
                        break;

                case 03:
                        _type = BIC;
                        _op1 = instr_raw.two_op.op1;
                        _mod1 = instr_raw.two_op.mod1;
                        _op2 = instr_raw.two_op.op2;
                        _mod2 = instr_raw.two_op.mod2;
                        _bit = instr_raw.two_op.bit;
                        break;
        }
        if (_type != -1) {
                str = get_str();
                return;
        }
        switch (instr_raw.branch.opcode) {
                case 0x01:
                        _type = BR;
                        _op1 = instr_raw.branch.addr;
                        break;
                case 0x02:
                        _type = BNE;
                        _op1 = instr_raw.branch.addr;
                        break;
                case 0x03:
                        _type = BEQ;
                        _op1 = instr_raw.branch.addr;
                        break;
                case 0x80:
                        _type = BPL;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x81:
                        _type = BMI;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x84:
                        _type = BVC;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x85:
                        _type = BVS;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x86:
                        _type = BHIS;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x87:
                        _type = BLO;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x04:
                        _type = BGE;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x05:
                        _type = BLT;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x06:
                        _type = BGT;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x07:
                        _type = BLE;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x82:
                        _type = BHI;
                        _op1 = instr_raw.branch.addr;
                        break;
                 case 0x83:
                        _type = BLOS;
                        _op1 = instr_raw.branch.addr;
                        break;
                 default:
                        break;
        }
        if (_type != -1) {
                str = get_str();
                return;
        }
        switch (instr_raw.h_op.opcode) {
                case 020:
                        _type = RTS;
                        _op1 = instr_raw.h_op.hop;
                        break;
                default:
                        break;
        }
        str = get_str();
}

std::string Instruction::get_str() {

        if (_type == -1) {
                return std::string("ERROR");
        }
        std::string ret(instruction_set[_type]);
        if (_op1 == -1) {
                return ret;
        }
        if (_bit == 1) {
                ret += "B ";
        } else {
                ret += " ";
        }
        std::string op1("");
        if (_mod1 != -1) {
                                    //fprintf(stderr, "!!!!! %u\n", _op1);
                if (_op1 == 07) {
                        if (_mod1 == R7IMM) {
                                op1 += "#";
                        } else if (_mod1 == R7ABS) {
                                op1 += "@#";
                        } else if (_mod1 == R7RELD) {
                                op1 += "";
                        } else if (_mod1 == R7RELI) {
                                op1 += "@";
                        }
                        op1 += std::to_string(_offsets[_size - 1].raw);
                        _size++;
                } else {
                        switch (_mod1) {
                                case MREG:
                                     op1 += "R";
                                     op1 += std::to_string(_op1);
                                     break;
                                case MREGI:
                                     op1 += "@R";
                                     op1 += std::to_string(_op1);
                                     break;
                                case MINC:
                                    op1 += "(R";
                                    op1 += std::to_string(_op1);
                                    op1 += ")+";
                                    break;
                                case MINCI:
                                    op1 += "@(R";
                                    op1 += std::to_string(_op1);
                                    op1 += ")+";
                                    break;
                                case MDEC:
                                    op1 += "-(R";
                                    op1 += std::to_string(_op1);
                                    op1 += ")";
                                    break;
                                case MDECI:
                                    op1 += "-@(R";
                                    op1 += std::to_string(_op1);
                                    op1 += ")";
                                    break;
                                case MIND:
                                    op1 += std::to_string(_offsets[_size - 1].raw);
                                    _size++;
                                    op1 += "(R";
                                    op1 += std::to_string(_op1);
                                    op1 += ")";
                                    break;
                                case MINDI:
                                    op1 += "@";
                                    op1 += std::to_string(_offsets[_size - 1].raw);
                                    _size++;
                                    op1 += "(R";
                                    op1 += std::to_string(_op1);
                                    op1 += ")";
                                    break;
                                default:
                                     break;
                         }
                }
        } else {
                if (BR <= _type && _type <= BLOS) {
                        uint8_t t = _op1;
                        op1 += std::to_string((int8_t) t);
                } else {
                        op1 += "R";
                        op1 += std::to_string(_op1);
                }
        }

        ret += op1;

        if (_op2 == -1) {
                return ret;
        }                   
        std::string op2("");
        if (_mod2 != -1) {
                if (_op2 == 07) {
                        if (_mod2 == R7IMM) {
                                op2 += "#";
                        } else if (_mod2 == R7ABS) {
                                op2 += "@#";
                        } else if (_mod2 == R7RELD) {
                                op2 += "";
                        } else if (_mod2 == R7RELI) {
                                op2 += "@";
                        }
                        op2 += std::to_string(_offsets[_size - 1].raw);
                        _size++;
                } else {
                        switch (_mod2) {
                                case MREG:
                                     op2 += "R";
                                     op2 += std::to_string(_op2);
                                     break;
                                case MREGI:
                                     op2 += "@R";
                                     op2 += std::to_string(_op2);
                                     break;
                                case MINC:
                                    op2 += "(R";
                                    op2 += std::to_string(_op2);
                                    op2 += ")+";
                                    break;
                                case MINCI:
                                    op2 += "@(R";
                                    op2 += std::to_string(_op2);
                                    op2 += ")+";
                                    break;
                                case MDEC:
                                    op2 += "-(R";
                                    op2 += std::to_string(_op2);
                                    op2 += ")";
                                    break;
                                case MDECI:
                                    op2 += "-@(R";
                                    op2 += std::to_string(_op2);
                                    op2 += ")";
                                    break;
                                case MIND:
                                    op2 += std::to_string(_offsets[_size - 1].raw);
                                    _size++;
                                    op2 += "(R";
                                    op2 += std::to_string(_op2);
                                    op2 += ")";
                                    break;
                                case MINDI:
                                    op2 += "@";
                                    op2 += std::to_string(_offsets[_size - 1].raw);
                                    _size++;
                                    op2 += "(R";
                                    op2 += std::to_string(_op2);
                                    op2 += ")";
                                    break;
                                default:
                                     break;
                         }
                }
     } else {
             op2 += "R";
             op2 += std::to_string(_op2);
     }
     ret += " ";
     ret += op2;
     return ret;

}

