#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(mirage_horn)

#----------------------------------------------------------------------------
# 1. Installation Path Setup
#----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/exp/dune/app/users/wyjang" CACHE PATH "..." FORCE)
endif()
set(SHARE_DIR "share/dune/mirage/horn")

# settings for dunegpvm spack environment
set(EXPAT_INCLUDE_DIR "/usr/include")
set(EXPAT_LIBRARY "/usr/lib64/libexpat.so")

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(mirage_horn mirage_horn.cc ${sources} ${headers})
target_include_directories(mirage_horn PRIVATE include)
target_link_libraries(mirage_horn PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build MIRAGE. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(MIRAGE_HORN_SCRIPTS
  mirage_horn.in
  mirage_horn.out
  init_vis.mac
  run1.mac
  run2.mac
  vis.mac
  )

foreach(_script ${MIRAGE_HORN_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()
