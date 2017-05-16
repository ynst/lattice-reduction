###########################################
###   Makefile for ae   ###

## forked from https://github.com/GordanM/makefile-skeleton/blob/master/Makefile

## edited by Yunus Tuncbilek (github.com/ynst)
###########################################

INC_FOLDER = ./includes
SOURCE_FOLDER = ./sources
OBJECTS_FOLDER = ./objects

CXX = g++

# Debug
CXXFLAGS = -g3 -O0 -Wall -std=c++11

# Release
#CXXFLAGS = -O2 -Wall

INC = -I$(INC_FOLDER)

RM = rm -f

SOURCES = utils.cpp test.cpp splp.cpp ae.cpp

OBJECTS = $(patsubst %.cpp, $(OBJECTS_FOLDER)/%.o, $(SOURCES)) 

EXECUTABLE = ae

all: $(LIBRARY) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

$(OBJECTS_FOLDER)/%.o: $(SOURCE_FOLDER)/%.cpp | $(OBJECTS_FOLDER)
	$(CXX) $(CXXFLAGS) $(INC) -o $@ -c $< 

$(OBJECTS_FOLDER): 
	mkdir -p $(OBJECTS_FOLDER)

# clean:
#  	$(RM) $(OBJECTS_FOLDER)/*.o $(LIBRARY) $(EXECUTABLE)