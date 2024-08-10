# get all sources
file(GLOB SOURCES "*.cc" "*.hh")

# split off the file containing main()
get_filename_component(MAIN ${CMAKE_CURRENT_SOURCE_DIR}/lux.cc ABSOLUTE)
list(REMOVE_ITEM SOURCES "${MAIN}")

# define a library based on the non-main files
add_library(liblux STATIC ${SOURCES})

# define an executable based on the main file
add_executable(lux "${MAIN}")

# make the executable depend on the library
target_link_libraries(lux liblux)

# use SOFA if available

# find sofa.h
find_path(SOFA_INCLUDE sxofa.h)

if (NOT ${SOFA_INCLUDE} STREQUAL "SOFA_INCLUDE-NOTFOUND")
  message("sofa found")
  find_library(SOFA_LIBRARY sofa)
  target_link_libraries(liblux ${SOFA_LIBRARY})
  target_include_directories(liblux ${SOFA_INCLUDE})
endif()

# create config.hh file to be able to suppress code making use of unavailable
# libraries
configure_file(config.hh.in config.hh)

# make config.hh accessible via #include "config.hh"
set(CMAKE_INCLUDE_CURRENT_DIR ON)
