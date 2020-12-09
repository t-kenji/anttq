# makefile for anttq.

ifeq ($(MAKELEVEL),0)

export ROOTDIR := $(PWD)

include $(ROOTDIR)/config.mk

TARGETS := all build install clean distclean
ifeq ($(HAS_TEST),1)
TARGETS += build-test clean-test test
endif
ifeq ($(HAS_EXAMPLE),1)
TARGETS += build-example clean-example example
endif
ifeq ($(HAS_DOC),1)
TARGETS += doc
endif
ifneq ($(shell which cppcheck),)
TARGETS += cppcheck
endif

define MAKE_TARGET
	@make -f $(ROOTDIR)/Makefile -C $1 --no-print-directory MAKE_OBJS=$2 $3
endef

.PHONY: $(TARGETS)

default: build

all: build build-test build-example

build:
	@$(foreach tgt,$(shell ls src/*.mk 2>/dev/null),$(call MAKE_TARGET,src,$(notdir $(tgt)));)

ifeq ($(HAS_TEST),1)
build-test: build
	@$(foreach tgt,$(shell ls test/*.mk 2>/dev/null),$(call MAKE_TARGET,test,$(notdir $(tgt)));)
else
build-test: ;
endif

ifeq ($(HAS_EXAMPLE),1)
build-example: build
	@$(foreach tgt,$(shell ls example/*.mk 2>/dev/null),$(call MAKE_TARGET,example,$(notdir $(tgt)));)
else
build-example: ;
endif

ifeq ($(HAS_TEST),1)
test: build-test
	@test/$(PROJECT)_utest -r compact $(if $(filter $(V),1),-s --durations yes) $(shell echo "$(TAG)" | grep -o -E -e "\w+" | sed -e "s/\(\w\+\)/[\1]/" | tr -d "\n")
else
test: ;
endif

ifeq ($(HAS_EXAMPLE),1)
example: build-example
else
example: ;
endif

ifeq ($(HAS_DOC),1)
doc:
	@sed -e 's|@PROJECT@|$(DOXY_PROJECT)|' \
	     -e 's|@VERSION@|$(DOXY_VERSION)|' \
	     -e 's|@BRIEF@|$(DOXY_BRIEF)|' \
	     -e 's|@OUTPUT@|$(DOXY_OUTPUT)|' \
	     -e 's|@SOURCES@|$(DOXY_SOURCES)|' \
	     -e 's|@EXCLUDE@|$(DOXY_EXCLUDE)|' \
	     -e 's|@EXCLUDE_SYMBOLS@|$(DOXY_EXCLUDE_SYMBOLS)|' \
	     -e 's|@PREDEFINED@|$(DOXY_PREDEFINED)|' \
	     -e 's|@README@|README.md|' \
	     -e 's|@EXTRACT_STATIC@|YES|' \
	     -e 's|@SHOW_FILES@|YES|' \
	     -e 's|@SOURCE_BROWSER@|YES|' \
	     Doxyfile.in > Doxyfile
	@doxygen Doxyfile
endif

ifneq ($(shell which cppcheck),)
cppcheck:
	@make -f $(ROOTDIR)/Makefile -C src --no-print-directory cppcheck
endif

install: install-test install-example

ifeq ($(HAS_TEST),1)
install-test: build-test
	@install -d $(DESTDIR)/bin
	install -m 0755 ./test/$(UTEST) $(DESTDIR)/bin
else
install-test: ;
endif

ifeq ($(HAS_EXAMPLE),1)
install-example: build-example
	@install -d $(DESTDIR)/bin
	install -m 0755 ./example/sample $(DESTDIR)/bin
else
install-example: ;
endif

clean: clean-test clean-example
	@$(foreach tgt,$(shell ls src/*.mk 2>/dev/null),$(call MAKE_TARGET,src,$(notdir $(tgt)),clean);)
	@rm -rf Doxyfile $(DOXY_OUTPUT)

ifeq ($(HAS_TEST),1)
clean-test:
	@$(foreach tgt,$(shell ls test/*.mk 2>/dev/null),$(call MAKE_TARGET,test,$(notdir $(tgt)),clean);)
else
clean-test: ;
endif

ifeq ($(HAS_EXAMPLE),1)
clean-example:
	@$(foreach tgt,$(shell ls example/*.mk 2>/dev/null),$(call MAKE_TARGET,example,$(notdir $(tgt)),clean);)
else
clean-example: ;
endif

distclean: clean-test clean-tool
	@$(foreach tgt,$(shell ls src/*.mk 2>/dev/null),$(call MAKE_TARGET,src,$(notdir $(tgt)),distclean);)
	@rm -rf Doxyfile $(DOXY_OUTPUT)

else
include $(MAKE_OBJS)
include $(ROOTDIR)/rules.mk
endif
