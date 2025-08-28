add_library(publish
    src/redis_publisher.cpp
)

find_path(HIREDIS_HEADER hiredis REQUIRED)
find_path(REDIS_PLUS_PLUS_HEADER sw REQUIRED)
find_library(HIREDIS_LIB hiredis REQUIRED)
find_library(REDIS_PLUS_PLUS_LIB redis++ REQUIRED)

cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED)

target_include_directories(publish PUBLIC include ${HIREDIS_HEADER} ${REDIS_PLUS_PLUS_HEADER} ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS})

target_link_libraries(publish PUBLIC common ${HIREDIS_LIB} ${REDIS_PLUS_PLUS_LIB})