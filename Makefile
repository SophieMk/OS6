all: \
	controller computer

deps:
	apt install libzmq3-dev

GCC = g++ -Wall -lzmq

controller: controller.cpp zmq_functions.hpp requester.hpp
	$(GCC) $< -o $@
computer: computer.cpp zmq_functions.hpp requester.hpp
	$(GCC) $< -o $@

clean:
	rm -f controller computer
