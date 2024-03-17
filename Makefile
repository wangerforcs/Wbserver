CXX=g++
TARGET=server
INCLUDE=include
CFLAGS=-std=c++14 -O2 -Wall -g -I $(INCLUDE)
LIBS=-pthread -lmysqlclient

SRCDIR=src
BINDIR=bin
SRC=$(wildcard $(SRCDIR)/*.cpp)
SRC+=main.cpp
OBJS=$(patsubst %.cpp, $(BINDIR)/%.o, $(notdir $(SRC)))

vpath %.cpp $(SRCDIR)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

$(BINDIR)/%.o: %.cpp
	$(CXX) -c $< $(CFLAGS) -o $@ $(LIBS)

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TARGET)
