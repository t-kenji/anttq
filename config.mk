# configuration for anttq.
export

# Project options.
PROJECT := anttq
MAJOR_VERSION := 2
MINOR_VERSION := 0
REVISION := 0
VERSION := $(MAJOR_VERSION).$(MINOR_VERSION).$(REVISION)
DESTDIR ?=

# Build target.
#  native: Build for native.
PLATFORM ?= native
ENABLE_STATIC := 1
ENABLE_SHARED := 0

# Build type.
#  release: No debuggable.
#  debug: Debuggable.
BUILD_TYPE ?= release

# Test options.
HAS_TEST := 1
INTERNAL_TESTABLE ?= 1

# Example options.
HAS_EXAMPLE := 1

# Documentation options.
HAS_DOC := 1
DOXY_PROJECT := "AntTQ"
DOXY_BRIEF := "AntTQ: Task Queue for Embedded"
DOXY_VERSION := "2.00"
DOXY_OUTPUT := doxygen
DOXY_SOURCES := include src

# Dependencies.
CATCH2_DIR ?=

# Debug options.
NODEBUG ?= 0
WARN_AS_ERROR ?= 0
ENABLE_SANITIZER ?= 0
DISABLE_CCACHE ?= 0

# Compile & Link options.
CSTANDARD ?= c11
CXXSTANDARD ?= c++14
EXTRA_CPPFLAGS ?=
EXTRA_CFLAGS ?=
EXTRA_CXXFLAGS ?=
EXTRA_LDFLAGS ?=
EXTRA_INCS ?= -I$(ROOTDIR)/include
EXTRA_LDLIBS ?= -lpthread -lrt

# Verbose options.
V ?= 0
