CPPFLAGS  := -D_UNIX_ -D_LINUX_ -DQT_GUI_LIB -fPIC -std=c++14 -O3 -Wall -MMD
CXX       := g++

-include *.d

test: test.cpp
	$(CXX) test.cpp -o $@ $(CPPFLAGS)