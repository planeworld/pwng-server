find_package(Threads)
find_package(Eigen3)
find_package(RapidJSON)

set(HEADERS
  ${LIB_NOISE_HEADERS}
  components/acceleration_component.hpp
  components/body_component.hpp
  components/name_component.hpp
  components/position_component.hpp
  components/radius_component.hpp
  components/subscription_components.hpp
  components/velocity_component.hpp
  managers/network_manager.hpp
  managers/simulation_manager.hpp
  systems/gravity_system.hpp
  systems/integrator_system.hpp
  math_types.hpp
  message_handler.hpp
  network_message.hpp
  star_definitions.hpp
  timer.hpp
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
  ${BOX2D_LIBRARY_LOCAL}
  ${LIBNOISE_LIBRARY_LOCAL}
)


set_property(TARGET pwng-server PROPERTY CXX_STANDARD 17)

install(TARGETS pwng-server DESTINATION bin)
