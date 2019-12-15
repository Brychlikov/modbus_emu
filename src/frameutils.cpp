#include <iostream>
#include <bitset>
#ifndef FRAMEUTILS_CPP
#define FRAMEUTILS_CPP

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
#endif
