CXX ?= g++
CXXFLAGS ?= -std=c++20 -O3 -Wall -Wextra -pedantic
INCLUDES = -I../include -I../src -Iinclude
SRCS = src/benchmark_runner.cpp ../src/collision.cpp ../src/physics.cpp
TARGET = benchmark_runner

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

quick: $(TARGET)
	./$(TARGET) --quick

clean:
	rm -f $(TARGET) $(TARGET).exe
