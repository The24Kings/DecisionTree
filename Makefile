CXX = g++
CXXFLAGS = -Wall -std=c++17 -g

TARGET = assign3
SRCS = assign3.cpp n-array-tree.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< 

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean