CXX = g++
TARGET = server
CFLAGS = -std=c++14 -O2 -Wall -g
LIBS = -pthread -lmysqlclient

SRCDIR = ./code 
SRC = $(shell find $(SRCDIR) -name "*.cpp")
DESDIR = ./bin
# OBJS = $(addprefix $(DESTDIR)/, $(notdir $(SRC:.cpp=.o)))
OBJS = $(patsubst  %.cpp, $(DESDIR)/%.o, $(notdir $(SRC)))

SOURSE_DIR = $(dir $(SRC))
vpath %.cpp $(SOURSE_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $@ $(LIBS)

$(DESDIR)/%.o: %.cpp
	$(CXX) -c $< -o $@ $(LIBS)

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TARGET)