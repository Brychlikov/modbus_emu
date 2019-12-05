CXX=clang++

all: modbus

modbus: build/main.o
	$(CXX) -o modbus build/main.o

build/main.o: src/main.cpp
	$(CXX) -o build/main.o -c src/main.cpp

clean:
	rm build/*

