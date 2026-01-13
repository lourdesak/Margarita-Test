cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

project(Margaritathree)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

# Sources & headers
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(g4marg marg.cc ${sources} ${headers})
target_include_directories(g4marg PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

#target_link_libraries(g4marg ${Geant4_LIBRARIES})
#add_custom_target(Simulation DEPENDS g4marg)

# ---- Minimal Histogram support ----
target_link_libraries(g4marg PRIVATE Geant4::G4analysis ${Geant4_LIBRARIES})
add_custom_target(Simulation DEPENDS g4marg)


# ---- Minimal macro support ----
# Copy all macros from <repo>/macros to build/macros
file(GLOB MACRO_FILES ${CMAKE_SOURCE_DIR}/macros/*.mac)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/macros)
foreach(f ${MACRO_FILES})
  get_filename_component(name ${f} NAME)
  configure_file(${f} ${CMAKE_BINARY_DIR}/macros/${name} COPYONLY)
endforeach()

# (expects run.mac)
add_custom_target(run
  COMMAND $<TARGET_FILE:g4marg> ${CMAKE_BINARY_DIR}/macros/run.mac
  DEPENDS g4marg
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
