cmake_minimum_required(VERSION 3.16)

project(fraud_detection_dsa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main
    main.cpp
    fraud_engine.cpp
)
