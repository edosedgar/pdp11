#ifndef __PDP11_REG_
#define __PDP11_REG_
#include<string>
#include"timings.hpp"

namespace PDP11 {

class Register {
        public:
                Register(std::string name)
                {
                        _name = name;
                        _value = 0;
                }
                std::string get_name()
                {
                        return _name;
                }
                uint16_t get_value()
                {
                        return _value;
                }
                int set_value(uint16_t value)
                {
                        _value = value;
                        return 0;
                }
        private:
                std::string _name;
                uint16_t _value;
};

};

#endif
