OUT_DIR = target
SRC_DIR = src

CC=gcc
CPP=g++

CFLAGS=-lm -O3 -fomit-frame-pointer -pipe -lpthread
CPPFLAGS=-O3 -fomit-frame-pointer -pipe -lpthread

DEPS = $(wildcard $(SRC_DIR)/*.h)

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OUT_FILES = $(patsubst $(SRC_DIR)/%.c,$(OUT_DIR)/%,$(SRC_FILES))

CPP_SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
CPP_OUT_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OUT_DIR)/%,$(CPP_SRC_FILES))

$(OUT_DIR)/%: $(SRC_DIR)/%.c $(DEPS) 
	$(CC) -o $@ $< $(CFLAGS)

$(OUT_DIR)/%: $(SRC_DIR)/%.cpp $(DEPS) 
	$(CPP) -o $@ $< $(CPPFLAGS)

all: $(OUT_FILES) $(CPP_OUT_FILES)

clean:
	rm -rf $(OUT_DIR)
	mkdir $(OUT_DIR)
