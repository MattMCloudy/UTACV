project (cv_model)
cmake_minimum_required(VERSION 2.8)

# explicitly set c++11 
set(CMAKE_CXX_STANDARD 11)

# configure OpenCV
find_package(OpenCV REQUIRED)

# create create individual projects
add_executable(lab3 lab3.cpp)
target_link_libraries(lab3 ${OpenCV_LIBS})