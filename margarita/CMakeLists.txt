cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

project(Margarita)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

# ---- ROOT support ----
find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Tree)

# Sources & headers
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(g4marg marg.cc ${sources} ${headers})
target_include_directories(g4marg PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(g4marg PRIVATE
  Geant4::G4analysis
  ${Geant4_LIBRARIES}
  ROOT::Core
  ROOT::RIO
  ROOT::Hist
  ROOT::Tree
)
#add_custom_target(Simulation DEPENDS g4marg)

# ---- Minimal Histogram support ----
target_link_libraries(g4marg PRIVATE Geant4::G4analysis ${Geant4_LIBRARIES})
add_custom_target(Simulation DEPENDS g4marg)


# ---- Minimal macro support ----
# Always-out-of-date target that re-syncs macros and the plot script on
# every `make` invocation, even if g4marg itself doesn't need to relink.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/macros)
add_custom_target(sync_runtime ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/macros
          ${CMAKE_BINARY_DIR}/macros
  COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_SOURCE_DIR}/plotstoppingmuons.C
          ${CMAKE_BINARY_DIR}/plotstoppingmuons.C
  COMMENT "Syncing macros and plot script to build directory"
)
add_dependencies(g4marg sync_runtime)

# (expects run.mac)
add_custom_target(run
  COMMAND $<TARGET_FILE:g4marg> ${CMAKE_BINARY_DIR}/macros/run.mac
  DEPENDS g4marg
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
