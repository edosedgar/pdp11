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

// States after requests

enum state {
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
        uint16_t *binary;
        int byte_rec;
public:
        GUI_channel() {
                port = 6700;
                ip = "127.0.0.1";
                struct sockaddr_in server_addr;
                int yes = 1;
                socklen_t size;
                current_state = REQUEST_DONE;
                binary = new uint16_t[12288]();

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
                        std::cerr << ">> GUI started \n";
                        memset(buffer, 0, 256);
                        return;
                }
                if (strstr(buffer, "em_load_file")) {
                        sscanf(buffer, "em_load_file %u", &eff_bin_size);
                        current_state = BINARY_RECEIVE;
                        answer_ok();
                        std::cerr << ">> GUI is going to send bin \n";
                        memset(buffer, 0, 256);
                        byte_rec = 0;
                        return;
                }
                if (strstr(buffer, "em_get_command")) {
                        unsigned int com_adr;

                        sscanf(buffer, "em_get_command %u", &com_adr);
                        std::stringstream ss;

                        ss << std::hex << com_adr;
                        current_state = COMMAND_SENDING;
                        std::cerr << "GUI requested com with address: 0x" \
                                  << std::setfill('0') << std::setw(4) \
                                  << std::hex << ss.str() << "\n";
                        current_address = com_adr;

                        memset(buffer, 0, 256);
                        return;
                }
skip_first:
                switch (current_state) {
                case COMMAND_SENDING:
                        strcpy(buffer, "2 MOV R1, R2");
                        send(client, &buffer, strlen(buffer), 0);
                        current_state = REQUEST_DONE;
                        std::cerr << "GUI received command \n";
                        memset(buffer, 0, 256);
                        break;
                case BINARY_RECEIVE:
                        memcpy(binary + byte_rec, buffer, 256);
                        byte_rec += 128;
                        if (byte_rec * 2 >= eff_bin_size) {
                                current_state = REQUEST_DONE;
                                answer_ok();
                        }
                        break;
                default:
                        break;
                }
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
