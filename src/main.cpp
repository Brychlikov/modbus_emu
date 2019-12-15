#include <iostream>
#include <sstream>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <unistd.h> 
#include <time.h>

#include "Slave.cpp"
#include "Master.cpp"

using namespace std;


int main(int argc, char ** argv) {


	Master m;
	for (int i = 1; i < 5; ++i) {
		auto s = new Slave(i);
		m.add_slave(s);
	}
	m.run();
	return 0;
}
