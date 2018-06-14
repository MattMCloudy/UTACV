project (cv_containers)
cmake_minimum_required(VERSION 2.8)

# configure OpenCV
find_package(OpenCV REQUIRED)

# create create individual projects
add_executable(homework1 homework1.cpp)
target_link_libraries(homework1 ${OpenCV_LIBS})

