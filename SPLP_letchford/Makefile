###########################################
###   Makefile for SPLP Letchford   ###

## forked from https://github.com/GordanM/makefile-skeleton/blob/master/Makefile

## edited by Yunus Tuncbilek (github.com/ynst)
###########################################

SOURCE_FOLDER = ./sources
OBJECTS_FOLDER = ./objects

CXX = g++
AR = ar

# Debug
CXXFLAGS = -g3 -O0 -Wall 

# Release
#CXXFLAGS = -O2 -Wall

ARFLAGS = rcs

RM = rm -f

SOURCES = SPLP_create_and_preprocess_letchford.cpp

OBJECTS = $(patsubst %.cpp, $(OBJECTS_FOLDER)/%.o, $(SOURCES)) 

EXECUTABLE = Letchford_SPLP

all: $(LIBRARY) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

$(OBJECTS_FOLDER)/%.o: $(SOURCE_FOLDER)/%.cpp | $(OBJECTS_FOLDER)
	$(CXX) $(CXXFLAGS) -o $@ -c $< 

$(OBJECTS_FOLDER): 
	mkdir -p $(OBJECTS_FOLDER)

clean:
 	$(RM) $(OBJECTS_FOLDER)/*.o $(LIBRARY) $(EXECUTABLE)