CXX      := g++ 
OBJ_DIR  := obj
APP_DIR  := bin
TARGET   := ray_tracer
INCLUDE  := -Iinclude
# LDFLAGS  := pkg-config glfw3 glew --cflags --libs
SRC  :=  $(wildcard *.cpp) 


OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)  $(LIB)


all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS) 

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS) $(shell pkg-config glfw3 glew --cflags --libs)

.PHONY: all build clean debug release

build:
	@mkdir -p $(APP_DIR) 
	
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g $(pkg-config glfw3 glew --cflags --libs)  
debug: all

release: CXXFLAGS += -O2 $(pkg-config glfw3 glew --cflags --libs)  
release: all

run:
	$(APP_DIR)/$(TARGET) 

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

