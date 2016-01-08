EXEC_NAME = tinysql
CXX_SRCS = main.cpp cmd.cpp dbMgr.cpp query.cpp test.cpp obj_util.cpp wrapper.cpp tiny_util.cpp
C_SRCS = parser.c
LEX_SRC = sql.l
YACC_SRC = sql.y

DB_API_DIR = db_sim_api
DB_MGR_OBJ = $(DB_API_DIR)/StorageManager.o

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
EXEC_TEST = $(BIN_FOLDER)/$(EXEC_NAME).test
#DIRSRCS = $(addprefix $(SRC_FOLDER)/, $(SRCS))
OBJS = $(addprefix $(OBJ_FOLDER)/, $(addsuffix .o, $(basename $(CXX_SRCS))))
DEPS = $(addprefix $(DEP_FOLDER)/, $(addsuffix .d, $(basename $(CXX_SRCS))))

LIBS = -ly -ll -lreadline
INCLUDES = -Iinclude -I$(DB_API_DIR)
FLAGS = -gdwarf-2 -g3 -Wall $(INCLUDES)
#FLAGS = -O2 -Iinclude
CXXFLAGS = $(FLAGS) --std=c++11
#CXXFLAGS = $(FLAGS)
#CFLAGS = $(FLAGS) -std=c99
CFLAGS = $(FLAGS) -std=gnu99
LFLAGS = $(FLAGS) $(LIBS)
LEXFLAG = -i -t
YACCFLAG = -d

CC = gcc
CXX = g++
LINK = g++
LEX = lex
YACC = yacc

all: $(EXEC)

#$(EXEC): $(OBJS) obj/parser.o $(OBJ_FOLDER)/$(LEX_OBJ) obj/y.tab.o $(DB_MGR_OBJ)
#$(EXEC): $(OBJS) $(OBJ_FOLDER)/$(LEX_OBJ) obj/y.tab.o obj/parser.o $(DB_MGR_OBJ)
$(EXEC): $(OBJS) obj/y.tab.o $(OBJ_FOLDER)/$(LEX_OBJ) obj/parser.o $(DB_MGR_OBJ)
	$(LINK) -o $@ $^ $(LFLAGS)

-include $(DEPS)

$(OBJ_FOLDER)/%.o: $(SRC_FOLDER)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(CXX) $(CXXFLAGS) -MT $(OBJ_FOLDER)/$*.o -MM $^ > $(DEP_FOLDER)/$*.d

$(OBJ_FOLDER)/%.o: $(SRC_FOLDER)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -MT $(OBJ_FOLDER)/$*.o -MM $^ > $(DEP_FOLDER)/$*.d

#obj/parser.o: src/parser.c src/parser.h src/y.tab.h src/lex.yy.h
#	$(CC) $(CFLAGS) -c $< -o $@
#
#obj/$(LEX_OBJ): src/$(LEX_C)
#	$(CC) $(CFLAGS) -c $< -o $@
#
#obj/y.tab.o: src/y.tab.c
#	$(CC) $(CFLAGS) -c $< -o $@


src/$(LEX_C) src/lex.yy.h: src/$(LEX_SRC) src/y.tab.h src/parser.h
	$(LEX) --header-file=src/lex.yy.h $(LEXFLAG) src/$(LEX_SRC) > src/$(LEX_C)

src/y.tab.c src/y.tab.h: src/sql.y src/parser.h
	cd src; $(YACC) $(YACCFLAG) sql.y


$(DB_MGR_OBJ):
	cd $(DB_API_DIR); make


.PHONY:
ctags:
	ctags $(SRC_FOLDER)/*

.PHONY: run
run:
	@./$(EXEC) testcases/example.do

.PHONY: test
test: testcases/*.in
	@cp ./$(EXEC) ./$(EXEC_TEST)
	@for FILE in $^; do \
		base=`basename $$FILE .in`; \
		out_file="testcases/$$base.out"; \
		res_file="results/$$base.res"; \
		diff_file="results/$$base.diff"; \
		echo ">>> start $$base:"; \
		./$(EXEC_TEST) $$FILE > $$res_file; \
		diff $$out_file $$res_file > $$diff_file; \
		echo ">>> diff $$base:"; \
		cat $$diff_file; \
		echo ">>> finish $$base:"; \
	done

.PHONY: valgrind
valgrind:
	valgrind --leak-check=yes $(EXEC) testcases/example.do

.PHONY: clean
clean:
	rm -rf $(OBJ_FOLDER)/*
	rm -rf $(BIN_FOLDER)/*
	rm -rf $(DEP_FOLDER)/*
	rm -rf tags
	rm -rf $(SRC_FOLDER)/$(LEX_C)
	rm -rf $(SRC_FOLDER)/lex.yy.h
	rm -rf $(SRC_FOLDER)/y.tab.c
	rm -rf $(SRC_FOLDER)/y.tab.h
	cd $(DB_API_DIR); make clean

