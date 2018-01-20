#include"decoder.hpp"
#include<iostream>
#include<cstdio>
#include<cstdint>

int main(int argc, char* argv[]) {
        size_t size;
        if (argc == 1) {
                std::cout << "Usage \"decoder file\"\n";
                return -1;
        }
        FILE* file = fopen(argv[1], "rb");
        if (file == NULL) {
                std::cout << "Error open file " << argv[1] << '\n';
                return -1;
        }
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        rewind(file);
        if (size & 1) {
                std::cout << "Incorrect file " << argv[1] << '\n';
                return -1;
        }

        auto rw = new InstrRaw[(size / 2) + 3];
        fread(rw, sizeof(InstrRaw), size / 2, file);
        //std::cout << sizeof(rw->zero_op) << '\n';
        for (int i = 0; i < 8; i++) {
                Instruction instr1(&rw[i], 3);
                std::cout << instr1.get_str() << '\n';
        }

        delete [] rw;
        return 0;
}
