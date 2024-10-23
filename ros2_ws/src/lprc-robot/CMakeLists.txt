cmake_minimum_required(VERSION 3.5)
project(lprc-robot)

find_package(ament_cmake REQUIRED)
# find_package(urdf REQUIRED)

install(
  DIRECTORY config description launch worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
