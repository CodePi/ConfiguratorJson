HDR = ../ConfiguratorJson/ConfiguratorJson.h ../ConfiguratorJson/Optional.h ../ConfiguratorJson/json.hpp
TESTS = TestConfig TestConfig2 TestConfig3 testOptional

all : $(TESTS)

% : %.cpp $(HDR)
	g++ -Wall -Werror -o $@ $<

clean:
	rm $(TESTS)
