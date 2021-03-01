cmake_minimum_required(VERSION 3.10)

project(shared_asset)

add_contract(shared_asset shared_asset ${CMAKE_CURRENT_SOURCE_DIR}/src/shared_asset.cpp)

target_include_directories(shared_asset PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
