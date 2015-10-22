EXEC_NAME = tinysql
SRCS = main.cpp
LEX_SRC = sql.l
YACC_SRC = sql.y

DB_MGR_OBJ = db_mgr/StorageManager.o

BIN_FOLDER = bin
SRC_FOLDER = src
OBJ_FOLDER = obj
DEP_FOLDER = dep

LEX_C = lex.yy.c
#LEX_C = $(addsuffix .c, $(basename $(LEX_SRC)))
LEX_OBJ = lex.yy.o
#------------------------------------------------------------------------------
#   
#------------------------------------------------------------------------------
#vpath %.cpp $(SRC_FOLDER)
#vpath %.l $(SRC_FOLDER)
#vpath %.o $(OBJ_FOLDER)

YACC_C = y.tab.c
YACC_OBJ = y.tab.o

EXEC = $(BIN_FOLDER)/$(EXEC_NAME)
#DIRSRCS = $(addprefix $(SRC_FOLDER)/, $(SRCS))
OBJS = $(addprefix $(OBJ_FOLDER)/, $(addsuffix .o, $(basename $(SRCS))))
DEPS = $(addprefix $(DEP_FOLDER)/, $(addsuffix .d, $(basename $(SRCS))))

FLAGS = -gdwarf-2 -g3 -Wall -Iinclude
CXXFLAGS = $(FLAGS) --std=c++11
#CXXFLAGS = $(FLAGS)
#CFLAGS = $(FLAGS) -std=c99
CFLAGS = $(FLAGS) -std=gnu99
LFLAGS = $(FLAGS) -ll
LEXFLAG = -t
YACCFLAG = -d

CC = gcc
CXX = g++
LINK = g++
LEX = lex
YACC = yacc

all: $(EXEC)

$(EXEC): $(OBJS) obj/parser.o $(OBJ_FOLDER)/$(LEX_OBJ) obj/y.tab.o $(DB_MGR_OBJ)
	$(LINK) -o $@ $^ $(LFLAGS)
	@#$(LINK) $(LFLAGS) -o $@ $^

-include $(DEPS)

$(OBJ_FOLDER)/%.o: $(SRC_FOLDER)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(CXX) $(CXXFLAGS) -MT $(OBJ_FOLDER)/$*.o -MM $^ > $(DEP_FOLDER)/$*.d


obj/parser.o: src/parser.c src/parser.h src/y.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

obj/$(LEX_OBJ): src/$(LEX_C)
	$(CC) $(CFLAGS) -c $< -o $@

obj/y.tab.o: src/y.tab.c
	$(CC) $(CFLAGS) -c $< -o $@


src/$(LEX_C): src/$(LEX_SRC) src/y.tab.h src/parser.h
	$(LEX) $(LEXFLAG) src/$(LEX_SRC) > src/$(LEX_C)

src/y.tab.c src/y.tab.h: src/sql.y src/parser.h
	cd src; $(YACC) $(YACCFLAG) sql.y
#$(SRC_FOLDER)/$(LEX_C): $(SRC_FOLDER)/$(LEX_SRC)
#	$(LEX)

$(DB_MGR_OBJ):
	cd db_mgr; make

.PHONY:
ctags:
	ctags $(SRC_FOLDER)/*

.PHONY: run
run:
	@./$(EXEC) < testcases/example.in

.PHONY: clean
clean:
	rm -f $(OBJ_FOLDER)/*
	rm -f $(BIN_FOLDER)/*
	rm -f $(DEP_FOLDER)/*
	rm -f tags
	rm -f $(SRC_FOLDER)/$(LEX_C)
	rm -f $(SRC_FOLDER)/y.tab.c
	rm -f $(SRC_FOLDER)/y.tab.h
	cd db_mgr; make clean
	@#rm -f $(DB_MGR_OBJ)


