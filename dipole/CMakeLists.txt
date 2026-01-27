#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(mirage)

#----------------------------------------------------------------------------
# 1. Installation Path Setup
#----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/exp/dune/app/users/wyjang" CACHE PATH "..." FORCE)
endif()
set(SHARE_DIR "share/dune/mirage/dipole")

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
add_executable(mirage mirage.cc ${sources} ${headers})
target_include_directories(mirage PRIVATE include)
target_link_libraries(mirage PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build mirage. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(MIRAGE_MACROS
  macros/init_vis.mac
  macros/POT_100k.mac
  macros/POT_1000k.mac
  macros/run1.mac
  macros/run2.mac
  macros/vis.mac
  )

set(MIRAGE_SCRIPTS
  scripts/agent.sh
  scripts/submit_grid.sh
  scripts/agent_ana.sh
  scripts/submit_grid_ana.sh
  )

set(MIRAGE_ANALYZER
  analyzer/mirage_plot.C
  )

foreach(_script ${MIRAGE_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

foreach(_macro ${MIRAGE_MACROS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_macro}
    ${PROJECT_BINARY_DIR}/${_macro}
    COPYONLY
  )
endforeach()

foreach(_analyzer ${MIRAGE_ANALYZER})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_analyzer}
    ${PROJECT_BINARY_DIR}/${_analyzer}
    COPYONLY
  )
endforeach()

# 2. installation of binary files
install(TARGETS mirage DESTINATION bin)

# 3. installation of macro files
install(FILES ${MIRAGE_MACROS}
  DESTINATION ${SHARE_DIR}/macros)

# 4. installation of script files
install(FILES ${MIRAGE_SCRIPTS}
  DESTINATION ${SHARE_DIR}/scripts
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )

install(FILES ${MIRAGE_ANALYZER}
  DESTINATION ${SHARE_DIR}/analyzer
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
