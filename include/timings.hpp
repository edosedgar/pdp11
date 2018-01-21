#ifndef __PDP11_TIMINGS_
#define __PDP11_TIMINGS_

//namespace PDP11 {

class Timings {
        int _mem_r;
        int _mem_w;
        int _dec;
        int _exec;
        int _reg;
        static Timings* _timings;
        Timings(int mem_r = 10,
                int mem_w = 10,
                int dec = 1,
                int reg = 1,
                int exec = 1)
        {
                _mem_r = mem_r;
                _mem_w = mem_w;
                _dec = dec;
                _exec = exec;
                _reg = reg;
        }
public:
        int get_mem_r()
        {
                return _mem_r;
        }

        int get_mem_w()
        {
                return _mem_w;
        }

        int get_dec()
        {
                return _dec;
        }

        int get_exec()
        {
                return _exec;
        }
        int get_reg()
        {
                return _reg;
        }
        static Timings* timings()
        {
                if (!_timings) {
                        _timings = new Timings;
                }
                return _timings;
        }
};

//};

#endif
