ifeq ($(TEXC),)
	TEXC = pdflatex
endif

all: docs-dev docs-usr pres

COMMON_FILES=$(shell find common -type f -name *.tex -print)
COMMON_DOC_FILES=$(shell find common -type f -name *.tex -print)
DEVDOC_FILES=$(shell find docs/dev -type f -name *.tex -print)
USRDOC_FILES=$(shell find docs/usr -type f -name *.tex -print)
PRES_FILES=$(shell find presentation -type f -name *.tex -print)

docs-dev: docs/dev/main.pdf
docs/dev/main.pdf: $(COMMON_FILES) $(COMMON_DOC_FILES) $(DEVDOC_FILES)
	cd docs/dev; $(TEXC) -interaction nonstopmode main.tex 

docs-usr: docs/usr/main.pdf
docs/usr/main.pdf: $(COMMON_FILES) $(COMMON_DOC_FILES) $(USRDOC_FILES)
	cd docs/usr; $(TEXC) -interaction nonstopmode main.tex

pres: presentation/main.pdf
presentation/main.pdf: $(COMMON_FILES) $(PRES_FILES)
	cd presentation; $(TEXC) -interaction nonstopmode main.tex

clean:
	@echo "Weak cleaning..."
	rm -f $(shell find . -type f -name *.pdf -print)
