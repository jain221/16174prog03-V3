#
# Makefile for release notes documentation.
#
# (C) 2016, Digitrol Ltd., Swansea, Wales.
#
#
# This makefile can generate word and pdf versions of release notes written in
# reStructuredtext.
#
# Using pandoc to create documents...
#
# See http://pandoc.org/README.html#creating-a-pdf
#

RST_SOURCES = $(wildcard *.rst)

FILES_OUT += $(RST_SOURCES:.rst=.docx)
#FILES_OUT += $(RST_SOURCES:.rst=.pdf)


all: $(FILES_OUT)
	@echo Sources: $(RST_SOURCES)
	@echo Outputs: $(FILES_OUT)


clean:
	$(RM) *.bak


clobber: clean
	$(RM) $(FILES_OUT)


rebuild: clobber all


debug:
	pandoc --version


%.docx: %.rst
	-attrib -R $@
	pandoc -o $@ $<
	attrib +R $@


%.pdf: %.rst
	pandoc -o $@ $<
