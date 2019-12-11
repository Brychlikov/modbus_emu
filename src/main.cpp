#include <iostream>
#include <sstream>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <unistd.h> 
#include <time.h>

using namespace std;

struct frame {
	string addr;
	string func;
	string data;
	string crc;
};

ostream& operator << (ostream &out, const frame &f) {
	out << f.addr << " ";
	out << f.func << " ";
	out << f.data << " "; 
	out << f.crc;
	return out;
}

union pipestruct {
	int arr[2];
	struct {
		int out;
		int in;
	};
};

class Slave;

void string_xor(char * s1, char * s2, int len) {
	for (int i = 0; i < len; ++i) {
		s1[i] = s1[i] != s2[i]? '1' : '0';
	}
}

void string_xor(string::iterator it1, string::iterator it2, int len) {
	for (int i = 0; i < len; ++i) {
		char c1 = *it1;
		char c2 = *it2;

		*it1 = c1 != c2? '1' : '0';
		it1++;
		it2++;
	}
}

string compute_crc(string msg) {
//	string m = msg + "000";
//	string crc = "1011";
	string m = msg + "0000000000000000";
	string crc = "10000000000000101";
	int len = m.length();

	for (int i = 0; i < m.length() - 16; ++i) {
		if(m[i] == '0') continue;
		/* cout << m << endl; */
		string_xor(m.begin() + i, crc.begin(), 17);
	}
	/* cout << "final\n"; */
//	cout <<  m << endl;
	return m.substr(len - 16, 16);
}

bool crc_check(string msg, string crc_sum) {
//    string crc = "1011";
    string crc = "10000000000000101";
	string m = msg + crc_sum;
	int len = m.length();

	for (int i = 0; i < m.length() - 16; ++i) {
		if(m[i] == '0') continue;
		string_xor(m.begin() + i, crc.begin(), 17);
	}
	/* cout << "final\n"; */
//	cout <<  m << endl;
	for(auto c: m.substr(len - 16, 16)) {
		if(c != '0')
			return false;
	}
	return true;
}

string string_to_binary(string s)  {
    string result;
    result.reserve(s.length() * 8);
    for(int i=0; i < s.length(); i++) {
        result.replace(i * 8, 8, bitset<8>(s[i]).to_string());
    }
    return result;
}

frame frame_from_string(string s) {
	frame result;
	long len = s.size();
	result.crc = s.substr(len - 16, 16);
	result.addr = s.substr(0, 8);
	result.func = s.substr(8, 8);
	result.data = s.substr(16, len - 32);
	return result;
}

string string_from_frame(frame &f) {
	return f.addr + f.func + f.data + f.crc;
}

class Master {
	vector<pipestruct> slavepipes;
	
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

};

class Slave {
    int16_t reg = 0;

    frame errorFrame(frame const &f) {
        frame result;
        result.addr = "00000000";

        string func = f.func;
        func.at(0) = '1';  // mark response as an exception

        result.func = func;
        result.data = "";
        result.crc = compute_crc(result.addr + result.func + result.data);
        return result;
    }

public:
    frame respond(frame const &f) {
        // supported functions:
        // 1 - add to register, responds with nothing
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
            result.data = "";
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

};

string read_pipe(int p) {
	// blocking call to read() Reads untill '\n'
	string result;
	char buf;
	while(1) {
		read(p, &buf, 1);
		if(buf == '\n') {
			break;
		}
		result.push_back(buf);
	}
	return result;
}

int child_func(int addr, pipestruct inpipe, pipestruct outpipe) {
	cout << "Child func started" << endl;
	close(inpipe.in);
	close(outpipe.out);
	Slave s;
	while(1) {
		cout << "Child wait begins" << endl;
		string sframe = read_pipe(inpipe.out);
		cout << "Child got message"<< endl;
		frame f = frame_from_string(sframe);
		frame res = s.respond(f);
		string response = string_from_frame(res) + "\n";
		write(outpipe.in, response.c_str(), response.size());
	}
	return 0;
}

int main(int argc, char ** argv) {


	pipestruct readpipe, writepipe;
	if(pipe(readpipe.arr) < 0 || pipe(writepipe.arr) < 0) {
		printf("TO NIE DZIALA AAAAAAAA\n");
		exit(1);
	}


	int pid;
	if ((pid = fork()) > 0) {
		// child process
		cout << "Child process started" << endl;
		child_func(0, writepipe, readpipe);
		exit(0);
	}

	Master m;
	frame f = m.makeFrame("101", 1, 1);
	string f_str = string_from_frame(f) + "\n";
	write(writepipe.in, f_str.c_str(), f_str.size());
	string s_res = read_pipe(readpipe.out);
	frame res = frame_from_string(s_res);
	cout << "OTRZYMANO RAMKE Z POWROTEM:\n" << res << endl;


	/* frame res = s.respond(f); */
	/* cout << res << endl; */
    /* res = s.respond(f); */
    /* cout << res << endl; */
    /* res = s.respond(f); */
    /* cout << res << endl; */
    /* f = m.makeFrame("", 1, 2); */
    /* res = s.respond(f); */
    /* cout << res << endl; */
    /* f = m.makeFrame("100000000000000000000", 1, 1); */
    /* res = s.respond(f); */
    /* cout << res << endl; */
	return 0;
}
