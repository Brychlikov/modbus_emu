#include <iostream>
#include <vector>
#include <bitset>

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

/* class Slave; */

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
	/* string m = msg + "000"; */
	/* string crc = "1011"; */
	string m = msg + "0000000000000000";
	string crc = "11000000000000101";
	int len = m.length();

	for (int i = 0; i < m.length() - 16; ++i) {
		if(m[i] == '0') continue;
		/* cout << m << endl; */
		string_xor(m.begin() + i, crc.begin(), 17);
	}
	/* cout << "final\n"; */
	cout <<  m << endl;
	return m.substr(len - 16, 16);
}

class Master {
	/* vector<Slave> slaves; */

	public: 
	void send(string msg, uint8_t addr, ostream &out) {
		frame f;

		f.addr = bitset<8>(addr).to_string();
		f.func = bitset<8>(1).to_string();
		f.data = string();
		f.data.reserve(msg.length() * 8);
		for(int i=0; i < msg.length(); i++) {
			f.data.replace(i * 8, 8, bitset<8>(msg[i]).to_string());
		}
		f.crc = compute_crc(f.addr + f.func + f.data);
		out << f;
	}

};

int main(int argc, char ** argv) {

	Master m;
	m.send("abc", 1, cout);
	cout << compute_crc("11010011101110") << endl;

	return 0;
}
