# Compiler settings
CXX = clang++
CXXFLAGS = -std=c++23 -Wall -O2

# SFML paths
SFML_DIR = $(HOME)/Archangel/vendor/SFML-3.0.2
SFML_INCLUDE = $(SFML_DIR)/include
SFML_LIB = $(SFML_DIR)/lib
SFML_FRAMEWORKS = $(SFML_DIR)/Frameworks

# ImGui paths
IMGUI_DIR = vendor/imgui
IMGUI_SFML_DIR = vendor/imgui-sfml

# Include paths
INCLUDES = -I$(SFML_INCLUDE) -I$(IMGUI_DIR) -I$(IMGUI_SFML_DIR)

# Library paths
LDFLAGS = -L$(SFML_LIB) -Wl,-rpath,$(SFML_LIB)

# SFML libraries (linking order matters)
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# macOS specific frameworks
FRAMEWORKS = -framework OpenGL -framework Foundation -framework CoreFoundation

# ImGui source files
IMGUI_SOURCES = $(IMGUI_DIR)/imgui.cpp \
                $(IMGUI_DIR)/imgui_demo.cpp \
                $(IMGUI_DIR)/imgui_draw.cpp \
                $(IMGUI_DIR)/imgui_tables.cpp \
                $(IMGUI_DIR)/imgui_widgets.cpp \
                $(IMGUI_SFML_DIR)/imgui-SFML.cpp

# Application source files
APP_SOURCES = src/main.cpp src/Game.cpp

# All sources
SOURCES = $(APP_SOURCES) $(IMGUI_SOURCES)
OBJECTS = main.o Game.o imgui.o imgui_demo.o imgui_draw.o imgui_tables.o imgui_widgets.o imgui-SFML.o
EXECUTABLE = sfml-app

# Build rules
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS) $(FRAMEWORKS)

# Compile application files
main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

Game.o: src/Game.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile ImGui files
imgui.o: $(IMGUI_DIR)/imgui.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

imgui_demo.o: $(IMGUI_DIR)/imgui_demo.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

imgui-SFML.o: $(IMGUI_SFML_DIR)/imgui-SFML.cpp
	$(CXX) $(CXXFLAGS) -DNDEBUG $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) *.o

run: $(EXECUTABLE)
	./$(EXECUTABLE)

.PHONY: all clean run
