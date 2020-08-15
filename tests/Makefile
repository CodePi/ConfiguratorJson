CC=g++
HDR = ../ConfiguratorJson/ConfiguratorJson.h ../ConfiguratorJson/Optional.h ../ConfiguratorJson/json.hpp
TESTS = TestConfig TestConfig2 TestConfig3 testOptional
FLAGS = -Wall -Werror -std=c++11

all : $(TESTS)

% : %.cpp $(HDR)
	$(CC) $(FLAGS) -o $@ $<

clean:
	rm $(TESTS)
