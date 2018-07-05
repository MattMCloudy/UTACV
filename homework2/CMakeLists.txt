project (cv_model)
cmake_minimum_required(VERSION 2.8)

# explicitly set c++11 
set(CMAKE_CXX_STANDARD 11)

# configure OpenCV
find_package(OpenCV REQUIRED)

# create create individual projects
add_executable(lab2 lab2.cpp)
target_link_libraries(lab2 ${OpenCV_LIBS})

