#include <iostream>
#include "frameutils.cpp"
#include "pipeutils.cpp"

#ifndef SLAVE_CPP
#define SLAVE_CPP

using namespace std;

class Slave {
    int16_t reg = 0;
    uint8_t addr;

    frame errorFrame(frame const &f) {
        frame result;
        result.addr = "00000000";

        string func = f.func;
        func.at(0) = '1';  // mark response as an exception

        result.func = func;
        result.data = f.data;
        result.crc = compute_crc(result.addr + result.func + result.data);
        return result;
    }

public:
    frame respond(frame const &f) {
        // supported functions:
        // 1 - add to register
        // 2 - read register

        // check data for errors
        frame result;
        result.addr = "00000000";
        bool correct = crc_check(f.addr + f.func + f.data, f.crc);
        if(!correct) {
            return errorFrame(f);
        }

        if(f.func == "00000001") {  // add
            if(f.data.length() > 16) {
                return errorFrame(f);
            }

            bitset<16> data_bit(f.data);
            uint16_t to_add = data_bit.to_ulong();
            reg += to_add;

            result.func = f.func;
            result.data = f.data;
            result.crc = compute_crc(result.addr + result.func + result.data);
            return result;
        }

        else if(f.func == "00000010") {  // read
            result.func = f.func;
            result.data = bitset<16>(reg).to_string();
            result.crc = compute_crc(result.addr + result.func + result.data);
            return result;
        } else {
            return errorFrame(f);
        }

    }

    Slave(uint8_t addr): addr(addr) {}

    void detach_and_listen(pipestruct inpipe, pipestruct outpipe) {
	if(fork() > 0) {
		printf("Child process with addr %d started\n", addr);

		close(inpipe.in);
		close(outpipe.out);
		
		while(1) {
			frame f = read_frame_from_pipe(inpipe);
			bitset <8> bitaddr(f.addr);
			if(bitaddr.to_ulong() != addr) {
				/* printf("Slave %d ignoring message\n", addr); */
				continue;
			}
			frame res = respond(f);
			string response = string_from_frame(res) + "\n";
			write(outpipe.in, response.c_str(), response.size());
		}
	}
    }

};
#endif
