#include <iostream>
#include <vector>
#include "Slave.cpp"
#include "pipeutils.cpp"
#include "frameutils.cpp"

#ifndef MASTER_CPP
#define MASTER_CPP

using namespace std;



class Master {
	vector<pair<pipestruct, pipestruct>> slavepipes;
	vector<Slave *> slaves;
	
	public: 
	frame makeFrame(string msg, uint8_t addr, uint8_t func) {
		frame f;

		f.addr = bitset<8>(addr).to_string();
		f.func = bitset<8>(func).to_string();
		f.data = msg;
		f.crc = compute_crc(f.addr + f.func + f.data);
		return f;
	}
	void send(string msg, uint8_t addr, uint8_t func, ostream &out) {
		frame f = makeFrame(msg, addr, func);
		out << f;
	}

	void add_slave(Slave * sp) {
		pipestruct readpipe, writepipe;
		if(pipe(readpipe.arr) < 0 || pipe(writepipe.arr) < 0) {
			cerr << "Could not create pipes\n";
			exit(1);
		}
		slaves.push_back(sp);
		slavepipes.push_back(make_pair(writepipe, readpipe));
	}

	void run() {
		for (int i = 0; i < slaves.size(); ++i) {
			slaves[i]->detach_and_listen(slavepipes[i].first, slavepipes[i].second);
		}

		int max_file_desc = 0;
		for(auto el: slavepipes) {
			if (el.second.out > max_file_desc) {
				max_file_desc = el.second.out;
			}
		}

		fd_set fd;
		fd_set fd_copy;
		FD_ZERO(&fd);
		int res;
		for (int i = 0; i < slaves.size(); ++i) {
			printf("Slave %d descriptor: %d\n", i, slavepipes[i].second.out);
			FD_SET(slavepipes[i].second.out, &fd);
		}

		timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		int addr, func, val;
		cout << "\n\n"
			"Usage: \n"
			"Enter three numbers\n"
			"<address> <function> <value>\n\n"
			"Available functions:\n"
			"1 - add <value> to slave's register\n"
			"2 - read slave's register. <value> is ignored\n";
		while(1) {
			cin >> addr >> func >> val;
			frame f = makeFrame(bitset<16>(val).to_string(), addr, func);
			for (int i = 0; i < slaves.size(); ++i) {
				send_frame_to_pipe(f, slavepipes[i].first);
			}

			fd_copy = fd;
			res = select(max_file_desc + 1, &fd_copy, NULL, NULL, &timeout);

			for (int i = 0; i < max_file_desc; ++i) {
				/* cout << "Trying " << i << endl; */
				if(FD_ISSET(i, &fd_copy)) {
					/* cout << "Reading " << i << endl; */
					frame f = read_frame_from_pipe(i);
					cout << "Received frame: " << f << endl;
					if(f.func == "00000010") {
						cout << "Received data: " << bitset<16>(f.data).to_ulong() << endl;
					}
				}
			}
		}

		
	}

};
#endif
