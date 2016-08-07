# Makefile
THIS_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include $(THIS_DIR)Makefile_vars

# Compile parallel in a sub makefile
MAKEFILE_SUB_COMPILE=$(THIS_DIR)Makefile_compile
MAKEFILE_PARALLEL=-j 5

ALL_UPDATED_VARIABLES+= THIS_DIR MAKEFILE_SUB_COMPILE MAKEFILE_PARALLEL

.PHONY: all clean show FORCE

all: $(TARGETS)

# Compile if source files are changed
$(TARGETS): FORCE
	$(MAKE) $(MAKEFILE_PARALLEL) -f $(MAKEFILE_SUB_COMPILE)

clean:
	$(MAKE) -f $(MAKEFILE_SUB_COMPILE) clean

show:
	$(foreach v, $(ALL_UPDATED_VARIABLES), $(info $(v) = $($(v))))

#Local Variables:
#mode: Makefile
#coding: utf-8-dos
#End:
