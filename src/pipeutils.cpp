#include <cstdlib>
#include <unistd.h> 
#include <time.h>
#include <iostream>
#include "frameutils.cpp"

#ifndef PIEUTILS_CPP
#define PIEUTILS_CPP
using namespace std;

union pipestruct {
	int arr[2];
	struct {
		int out;
		int in;
	};
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

string read_pipe(pipestruct p) {
	// blocking call to read() Reads untill '\n'
	string result;
	char buf;
	while(1) {
		read(p.out, &buf, 1);
		if(buf == '\n') {
			break;
		}
		result.push_back(buf);
	}
	return result;
}



void send_frame_to_pipe(frame f, pipestruct p) {
	string f_str = string_from_frame(f) + "\n";
	write(p.in, f_str.c_str(), f_str.size());
}

frame read_frame_from_pipe(pipestruct p) {
	string f_str = read_pipe(p);
	return frame_from_string(f_str);
}
frame read_frame_from_pipe(int p) {
	string f_str = read_pipe(p);
	return frame_from_string(f_str);
}
#endif
