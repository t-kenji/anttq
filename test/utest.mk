CONFIG_TEST_MEMPOOL := y
CONFIG_TEST_QUEUE := y
CONFIG_TEST_ANTTQ := y

test-$(CONFIG_TEST_MEMPOOL) += mempool.o
test-$(CONFIG_TEST_QUEUE) += queue.o
test-$(CONFIG_TEST_ANTTQ) += anttq.o

MODULE := utest
TEST := $(PROJECT)_utest
OBJS := main.o utils.o $(test-y)
EXTRA_CXXFLAGS += -I$(ROOTDIR)/src
EXTRA_CXXFLAGS += $(if $(CATCH2_DIR),-I$(CATCH2_DIR)/single_include)
