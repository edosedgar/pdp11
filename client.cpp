#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <sstream>
#include <iomanip>
#include "pdp11.hpp"

#define DEBUG_DEF
// States after requests

enum state_enum {
        INITIAL_STATE = 0,
        BINARY_PATH_WAIT,
        BINARY_PATH_ACCEPTED,
        COMMAND_REQUESTED,
        COMMAND_SENDING,
        REQUEST_DONE,
        OK_WAIT,
        BINARY_RECEIVE,
};

class GUI_channel
{
        std::string name;
        int port;
        std::string ip;
        int server, client;
        char buffer[256];
        struct pollfd poll_set[1];
        int numfds = 0;
        int max_fd = 0;
        int current_state = 0;
        char bin_path[256];
        unsigned int current_address = 0;
        unsigned int emul_address = 0;
        unsigned int eff_bin_size = 0;
        uint8_t* binary;
        int byte_rec;
        PDP11 *pdp;
        uint8_t *breakpoint;
public:
        GUI_channel() {
                port = 6700;
                ip = "192.168.0.113";
                struct sockaddr_in server_addr;
                int yes = 1;
                socklen_t size;
                current_state = REQUEST_DONE;
                binary = new uint8_t[24576]();

                std::cout << "\n- Starting server..." << std::endl;
                server = socket(AF_INET, SOCK_STREAM, 0);

                inet_pton(AF_INET, ip.c_str(), &(server_addr.sin_addr));
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(port);

                setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

                bind(server, (struct sockaddr*) &server_addr, sizeof(server_addr));

                size = sizeof(server_addr);

                listen(server, 1);
                client = accept(server, (struct sockaddr *) &server_addr, &size);

                poll_set[0].fd = client;
                poll_set[0].events = POLLIN;
                numfds++;
                max_fd = client;
        }
        void send_msg(std::string msg) {
                //
        }
        std::string rcv_msg(int len) {
                std::string My = std::string(256, ' ');
                return My;
        }
        void loop() {
                poll(poll_set, numfds, 1);
                if (poll_set[0].revents & POLLIN) {
                        recv(client, buffer, 256, 0);
                }
        };
        void answer_ok() {
                memset(buffer, 0, 256);
                buffer[0] = '0';
                send(client, &buffer, 1, 0);
        }
        void process_request() {
                if (current_state != REQUEST_DONE)
                        goto skip_first;
                if (strstr(buffer, "em_init_gui")) {
                        answer_ok();
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI started \n";
#endif
                        memset(buffer, 0, 256);
                        return;
                }
                if (strstr(buffer, "em_load_file")) {
                        sscanf(buffer, "em_load_file %u", &eff_bin_size);
                        //current_state = BINARY_RECEIVE;
                        answer_ok();
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI is going to send bin :" << eff_bin_size << "\n";
#endif
                        recv(client, binary, eff_bin_size, 0);
                        current_state = REQUEST_DONE;
                        answer_ok();
                        breakpoint = new uint8_t[65536]();
                        start_machine();
                        memset(buffer, 0, 256);
                        return;
                }
                if (strstr(buffer, "em_get_command")) {
                        unsigned int com_adr;
                        sscanf(buffer, "em_get_command %u", &com_adr);
                        std::stringstream ss;

                        ss << std::hex << com_adr;
                        current_state = COMMAND_SENDING;
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI requested com with address: 0x" \
                                  << std::setfill('0') << std::setw(4) \
                                  << std::hex << ss.str() << "\n";
#endif
                        current_address = com_adr;

                        memset(buffer, 0, 256);
                        return;
                }
                if (strstr(buffer, "em_get_state")) {
                        std::string cm = pdp->info_registers();
                        strcpy(buffer, cm.c_str());
                        send(client, &buffer, strlen(buffer), 0);
                        current_state = REQUEST_DONE;
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI received machine state >> " \
                                  << cm << "\n";
#endif
                        memset(buffer, 0, 256);
                        return;
                }
                if (strstr(buffer, "em_make_step")) {
                        int cycle = pdp->exec();
                        if (pdp->get_state() == HALTED) {
                                cycle = -1;
                        }
                        int instr_addr = pdp->get_pc();
                        int dirty_bit = pdp->get_dirty();
                        std::string cc = std::to_string(cycle);
                        std::string ad = std::to_string(instr_addr);
                        std::string dd = std::to_string(dirty_bit);
                        ad += " " + cc;
                        ad += " " + dd;

                        strcpy(buffer, ad.c_str());
                        send(client, &buffer, strlen(buffer), 0);
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI requsted step " << \
                                     ">> address, cycle: " << ad << "\n";
#endif
                        memset(buffer, 0, 256);
                        return;
                }
                if (strstr(buffer, "em_reset_state")) {
                        pdp->reset();
                        answer_ok();
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI requsted reset \n";
#endif
                        return;
                }
                if (strstr(buffer, "em_get_vram")) {
                        uint8_t *vram = pdp->get_vram();
                        send(client, vram, 8192, 0);
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI requsted display state \n";
#endif
                        memset(buffer, 0, 256);
                        return;
                }
                if (strstr(buffer, "em_toggle_break")) {
                        unsigned int com_adr;
                        sscanf(buffer, "em_toggle_break %u", &com_adr);
                        breakpoint[com_adr] ^= 255;
                        answer_ok();
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI toggled break >> "
                                  << std::setfill('0') << std::setw(4) \
                                  << std::hex << com_adr << "\n";
#endif
                        return;
                }
                if (strstr(buffer, "em_run_machine")) {
                        int cycle = 0;
                        int get_state = -1;
                        int instr_addr = pdp->get_pc();
                        int dirty_bit = 0;
                        int sum_cycle = 0;
                        int old_adr = 0;
                        uint8_t *vram = NULL;
#ifdef DEBUG_DEF
                        std::cerr << ">> GUI requsted run \n";
#endif
                        while (get_state != HALTED) {
                                cycle = pdp->exec();
                                old_adr = instr_addr;
                                instr_addr = pdp->get_pc();
                                dirty_bit = pdp->get_dirty();
                                sum_cycle += cycle;
                                if (dirty_bit) {
                                        std::cerr << "GUI is being received VRAM \n";
                                        vram = pdp->get_vram();
                                        send(client, "vram", 5, 0);
                                        recv(client, buffer, 256, 0);
                                        send(client, vram, 8192, 0);
                                        recv(client, buffer, 256, 0);
                                        std::string cc = std::to_string(sum_cycle);
                                        std::string ad = std::to_string(instr_addr);
                                        ad += " " + cc;

                                        strcpy(buffer, ad.c_str());
                                        send(client, &buffer, strlen(buffer), 0);
                                        recv(client, buffer, 256, 0);
                                        sum_cycle = 0;
                                }
                                if (breakpoint[instr_addr]) {
                                        std::cerr << "GUI is being received BREAK \n";
                                        send(client, "break", 6, 0);
                                        recv(client, buffer, 256, 0);
                                        break;
                                }
                                get_state = pdp->get_state();
                        }
                        if (pdp->get_state() == HALTED) {
                                std::cerr << "GUI is being received HALT \n";
                                send(client, "halt", 5, 0);
                                recv(client, buffer, 256, 0);
                                instr_addr = old_adr;
                        }
                        std::string cc = std::to_string(sum_cycle);
                        std::string ad = std::to_string(instr_addr);
                        ad += " " + cc;

                        strcpy(buffer, ad.c_str());
                        send(client, &buffer, strlen(buffer), 0);
                        memset(buffer, 0, 256);
                        return;
                }
skip_first:
                switch (current_state) {
                case COMMAND_SENDING: {
                        std::string cm = pdp->info_instruction(current_address);
                        strcpy(buffer, cm.c_str());
                        send(client, &buffer, strlen(buffer), 0);
                        current_state = REQUEST_DONE;
#ifdef DEBUG_DEF
                        std::cerr << "GUI received command >> " << cm << "\n";
#endif
                        memset(buffer, 0, 256);
                        break;
                }
                case BINARY_RECEIVE:
                        memcpy(binary + byte_rec, buffer, 256);
                        byte_rec += 256;
                        if (byte_rec >= eff_bin_size) {
                                current_state = REQUEST_DONE;
                                answer_ok();
                                for (auto i = 0; i < eff_bin_size; i++) {
                                        printf("0x%X ", binary[i]);
                                };
                                breakpoint = new uint8_t[65536]();
                                start_machine();
                        }
                        break;
                default:
                        break;
                }
        }
        void start_machine() {
                pdp = new PDP11;
                pdp->load((uint8_t*) binary, eff_bin_size);
                pdp->reset();
        }
};

int
main(int argc, char *argv[])
{
        GUI_channel GUICh = GUI_channel();
        while (1) {
                GUICh.loop();
                GUICh.process_request();
        }
        return 0;
}
