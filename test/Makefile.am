check_PROGRAMS = test-live-cells

TESTS = test-live-cells

test_live_cells_SOURCES = constant_cell.cpp \
	mutable_cell.cpp \
	equality.cpp \
	compute_cell.cpp \
	dynamic_compute_cell.cpp \
	store_cell.cpp \
	numeric.cpp \
	boolean.cpp \
	errors.cpp \
	consistency.cpp \
	peek_cell.cpp \
	init_and_cleanup.cpp \
	mutable_compute_cell.cpp \
	dynamic_mutable_compute_cell.cpp \
	live_cells_test.cpp

test_live_cells_CPPFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS)
test_live_cells_LDFLAGS = $(BOOST_LDFLAGS)
test_live_cells_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
	../src/live_cells.$(OBJEXT)
