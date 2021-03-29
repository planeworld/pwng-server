find_package(Threads)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Eigen3)

set(HEADERS
  ${LIB_NOISE_HEADERS}
  components/acceleration_component.hpp
  components/body_component.hpp
  components/name_component.hpp
  components/position_component.hpp
  components/radius_component.hpp
  components/velocity_component.hpp
  managers/network_manager.hpp
  managers/simulation_manager.hpp
  systems/gravity_system.hpp
  systems/integrator_system.hpp
  math_types.hpp
  message_handler.hpp
)

set(SOURCES
  managers/network_manager.cpp
  managers/simulation_manager.cpp
  pwng_server.cpp
)

add_executable(pwng-server ${HEADERS} ${SOURCES})

target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/components")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/managers")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/src/systems")
target_include_directories(pwng-server PRIVATE "${PROJECT_SOURCE_DIR}/install/include/")

target_link_libraries(pwng-server PRIVATE
  Eigen3::Eigen
  Threads::Threads
  nlohmann_json::nlohmann_json
  ${BOX2D_LIBRARY_LOCAL}
  ${LIBNOISE_LIBRARY_LOCAL}
)


set_property(TARGET pwng-server PROPERTY CXX_STANDARD 17)

install(TARGETS pwng-server DESTINATION bin)
