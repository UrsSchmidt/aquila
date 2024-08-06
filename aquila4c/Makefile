TITLE=aquila4c
TARGET_EXEC=$(TITLE)

GEN=gen
OBJ=obj

# build target

SRC=src
GRAMMAR=../Aquila.g4

SRCCPPFILES=$(wildcard $(SRC)/*.cpp)
SRCOFILES=$(SRCCPPFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
GENCPPFILES=$(GEN)/AquilaBaseVisitor.cpp $(GEN)/AquilaLexer.cpp $(GEN)/AquilaParser.cpp $(GEN)/AquilaVisitor.cpp
GENOFILES=$(GENCPPFILES:$(GEN)/%.cpp=$(OBJ)/%.o)

CXX=g++
CXXFLAGS=-std=c++17 -I/usr/local/include/antlr4-runtime
LDLIBS=-lantlr4-runtime -lgmp

$(TARGET_EXEC): $(SRCOFILES) $(GENOFILES)
	$(CXX) -o $@ $^ $(LDLIBS)

$(OBJ)/Aquila%.o: $(GEN)/Aquila%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ)/%.o: $(SRC)/%.cpp $(GENCPPFILES)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -Wall -Wpedantic -c -o $@ $<

$(GENCPPFILES)&: $(GRAMMAR)
	@mkdir -p $(dir $@)
	java -jar ../../antlr-4.12.0-complete.jar $(shell pwd)/$^ -Dlanguage=Cpp -visitor -no-listener -encoding utf8 -o $(dir $@)

# install/uninstall targets

FILE_BIN=$(TARGET_EXEC)
INSTALL_DIR_BIN=/usr/local/bin/
INSTALL_PATH_BIN=$(INSTALL_DIR_BIN)$(FILE_BIN)

.PHONY: install
install: $(FILE_BIN)
ifneq ($(wildcard $(INSTALL_DIR_BIN).),)
	cp $(FILE_BIN) $(INSTALL_PATH_BIN)
else
	@echo directory not found $(INSTALL_DIR_BIN)
endif

.PHONY: uninstall
uninstall:
ifneq ($(wildcard $(INSTALL_PATH_BIN)),)
	rm -f $(INSTALL_PATH_BIN)
endif

# clean target

.PHONY: clean
clean:
	rm -f $(TARGET_EXEC)
	rm -rf $(OBJ)
	rm -rf $(GEN)
