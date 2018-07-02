# @file  Makefile
# @brief Reports makefile for the 16174prog03 project.
#
# @note
# Copyright (C) 2017, Digitrol Ltd, Swansea, Wales. All rights reserved.
#

CLOC := C:/programs/cloc/cloc-1.56.exe
#CLOC := C:/programs/cloc/cloc-1.64.exe
NLOC := C:/programs/nloc/NLOC_v1.0/bin/NLOC.exe

################################################################################

.PHONY: all clean clobber rebuild debug
all: .starting .nloc .cloc .finished

clean:
	$(RM) *.bak

clobber: clean
	$(RM) 16174prog03_line_count.cloc.generated.txt
	$(RM) 16174prog03_line_count.nloc.generated.txt

rebuild: clobber all

debug:
	@$(CLOC) --help
#	@$(NLOC) -help

################################################################################

.cloc:
	@echo
	@echo Running cloc...
	@$(CLOC) \
		--out=./16174prog03_line_count.cloc.generated.txt \
		--by-file-by-lang \
		--exclude-lang=D,HTML,XML,"DOS Batch",make,Javascript,CSS \
		--exclude-dir=dox \
		../
	@echo
	@echo Finished


.nloc:
	@echo
	@echo Running nloc...
	@$(NLOC) \
		-out:./16174prog03_line_count.nloc.generated.txt \
		-recurse:../*.h \
		-recurse:../*.c \
		-recurse:../*.cpp
	@echo
	@echo Finished


.starting:
	@echo Starting


.finished:
	@echo
	@echo Finished

################################################################################
