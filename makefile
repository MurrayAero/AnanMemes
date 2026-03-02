BIN        := demo
SRC_DIR    := .
BUILD_DIR  := build
LIB_DIR    := lib
INC_DIR    := include

CXX        := g++
CXXFLAGS   := -g -Wall -Wextra -Wpedantic -std=c++17
INC_PATH   := -I$(INC_DIR)
LIB_PATH   := -L$(LIB_DIR)
LIBS       := 

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

MACRO :=

.PHONY: all clean run debug

all: MACRO :=
all: $(BIN)

debug: MACRO := -DDEBUG
debug: clean $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ $(LIB_PATH) $(LIBS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $(MACRO) $< $(INC_PATH) -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN)

run: all
	./$(BIN) 