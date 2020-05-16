cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

project(indexing_tbb)

find_package(TBB REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Boost COMPONENTS locale system filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

file(GLOB SOURCES
        include/*.h
        src/*.cpp
        )

set(LIBS ${LibArchive_LIBRARIES} ${Boost_LIBRARIES} TBB::tbb)

add_executable(indexing_tbb main.cpp ${SOURCES})
target_link_libraries(indexing_tbb ${LIBS} -ljsoncpp)