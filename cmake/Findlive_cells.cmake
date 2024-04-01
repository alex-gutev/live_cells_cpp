include(FindPackageHandleStandardArgs)

find_library(LIVE_CELLS_LIBRARY NAMES live_cells)
find_path(LIVE_CELLS_INCLUDE_DIR NAMES live_cells/live_cells.hpp)

find_package_handle_standard_args(LIVE_CELLS REQUIRED_VARS LIVE_CELLS_LIBRARY LIVE_CELLS_INCLUDE_DIR)

if (LIVE_CELLS_FOUND)
  mark_as_advanced(LIVE_CELLS_INCLUDE_DIR)
  mark_as_advanced(LIVE_CELLS_LIBRARY)
endif()

if (LIVE_CELLS_FOUND AND NOT TARGET live_cells::LIVE_CELLS)
  add_library(live_cells::LIVE_CELLS STATIC IMPORTED)
  set_property(TARGET live_cells::LIVE_CELLS PROPERTY IMPORTED_LOCATION ${LIVE_CELLS_LIBRARY})
  target_include_directories(live_cells::LIVE_CELLS INTERFACE ${LIVE_CELLS_INCLUDE_DIR})
endif()
