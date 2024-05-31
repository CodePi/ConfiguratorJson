HDR = ../ConfiguratorJson/ConfiguratorJson.h ../ConfiguratorJson/Optional.h ../ConfiguratorJson/json.hpp
TESTS = TestConfigJS TestConfigJS2 TestConfigJS3 testOptionalCJS
FLAGS = -Wall -Werror -std=c++11

all : $(TESTS)

% : %.cpp $(HDR)
	$(CXX) $(FLAGS) -o $@ $<

clean:
	rm -f $(TESTS)
