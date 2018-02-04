# Makefile
THIS_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include $(THIS_DIR)Makefile_vars

# Compile parallel in a sub makefile
MAKEFILE_SUB_COMPILE=$(THIS_DIR)Makefile_compile
MAKEFILE_PARALLEL=-j 5

ALL_UPDATED_VARIABLES+= THIS_DIR MAKEFILE_SUB_COMPILE MAKEFILE_PARALLEL

.PHONY: all clean rebuild check time test show FORCE

all: $(TARGETS)

# Compile if source files are changed
$(TARGETS): FORCE
	$(MAKE) $(MAKEFILE_PARALLEL) -f $(MAKEFILE_SUB_COMPILE)

clean:
	$(MAKE) -f $(MAKEFILE_SUB_COMPILE) clean

rebuild: clean all

check: $(TARGETS)
	perl sudoku_solve_all.pl
	ruby sudoku_check.rb

test: $(TARGETS)
	time $(CELLS_UNPACKED_TARGET) $(TEST_CASE_ORIGINAL_PUZZLE) sse print > $(TEST_CASE_SOLUTION)
	time $(CELLS_UNPACKED_TARGET) $(TEST_CASE_ORIGINAL_PUZZLE) c++ print > $(TEST_CASE_SOLUTION)
	time $(CELLS_DIAGONAL_TARGET) $(TEST_CASE_DIAGONAL_PUZZLE) sse print > $(TEST_CASE_SOLUTION)
	$(PYTHON) $(SOLUTION_CHECKER_SCRIPT) --log $(TEST_CASE_SOLUTION)
	time $(CELLS_DIAGONAL_TARGET) $(TEST_CASE_DIAGONAL_PUZZLE) c++ print > $(TEST_CASE_SOLUTION)
	$(PYTHON) $(SOLUTION_CHECKER_SCRIPT) --log $(TEST_CASE_SOLUTION)

time: $(TARGETS)
	ruby sudoku_check.rb

show:
	$(foreach v, $(ALL_UPDATED_VARIABLES), $(info $(v) = $($(v))))

#Local Variables:
#mode: Makefile
#coding: utf-8-dos
#End:
