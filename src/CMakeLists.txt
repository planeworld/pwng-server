find_package(Threads)
find_package(nlohmann_json 3.2.0 REQUIRED)

set(HEADERS
  components/acceleration_component.hpp
  components/position_component.hpp
  components/velocity_component.hpp
  managers/network_manager.hpp
  managers/simulation_manager.hpp
  systems/integrator_system.hpp
)

set(SOURCES
  managers/network_manager.cpp
  managers/simulation_manager.cpp
  pwng_server.cpp
)

add_executable(pwng-server ${LIB_NOISE_HEADERS} ${HEADERS} ${SOURCES})

target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/components")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/managers")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/systems")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/install/include/")

target_link_libraries(pwng-server PRIVATE
  Threads::Threads
  nlohmann_json::nlohmann_json
  ${BOX2D_LIBRARY_LOCAL}
  ${LIBNOISE_LIBRARY_LOCAL}
)


set_property(TARGET pwng-server PROPERTY CXX_STANDARD 17)

install(TARGETS pwng-server DESTINATION bin)
