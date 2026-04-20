cmake_minimum_required(VERSION 3.16)
project(FakeBot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(FakeBot
    src/main.cpp
    src/ChatBot.cpp
    src/RuleLoader.cpp
    src/TextUtils.cpp
)

target_include_directories(FakeBot PRIVATE src)
