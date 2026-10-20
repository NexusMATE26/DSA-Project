cmake_minimum_required(VERSION 3.20)
project(InternPath VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(INTERNPATH_BUILD_TESTS "Build core unit tests" ON)
option(INTERNPATH_ENABLE_EXERCISE_TESTS "Register deliberately failing DSA exercise tests" OFF)
option(INTERNPATH_ENABLE_SANITIZERS "Enable ASan/UBSan in supported compilers" OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(INTERNPATH_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(internpath_domain STATIC
    src/domain/Matching.cpp
    src/algo/GraphAlgorithms.cpp
    src/core/Services.cpp
    src/core/Persistence.cpp
)
target_include_directories(internpath_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Developer smoke program; not part of the eventual product UI.
add_executable(internpath_smoke tools/smoke_main.cpp)
target_link_libraries(internpath_smoke PRIVATE internpath_domain)

if(INTERNPATH_BUILD_TESTS)
    enable_testing()
    add_executable(test_matching tests/test_matching.cpp)
    target_link_libraries(test_matching PRIVATE internpath_domain)
    add_test(NAME matching_core COMMAND test_matching)

    add_executable(test_headers tests/test_headers.cpp)
    target_link_libraries(test_headers PRIVATE internpath_domain)
    add_test(NAME header_contracts COMMAND test_headers)

    add_executable(test_date tests/test_date.cpp)
    target_link_libraries(test_date PRIVATE internpath_domain)
    add_test(NAME date_validation COMMAND test_date)

    add_executable(test_e2e_integration tests/test_e2e_integration.cpp)
    target_link_libraries(test_e2e_integration PRIVATE internpath_domain)
    add_test(NAME e2e_integration COMMAND test_e2e_integration)

    # These exercise tests intentionally fail until the team implements the TODOs.
    # They compile by default, but are not included in CTest until opted in.
    add_executable(exercise_dynamic_array tests/exercises/test_dynamic_array.cpp)
    target_link_libraries(exercise_dynamic_array PRIVATE internpath_domain)
    add_executable(exercise_sort_search tests/exercises/test_sort_search.cpp)
    target_link_libraries(exercise_sort_search PRIVATE internpath_domain)
    if(INTERNPATH_ENABLE_EXERCISE_TESTS)
        add_test(NAME exercise_dynamic_array COMMAND exercise_dynamic_array)
        add_test(NAME exercise_sort_search COMMAND exercise_sort_search)
    endif()
endif()

# Milestone 0 does not build a server. Antigravity should first vendor approved
# HTTP/JSON infrastructure and introduce the 'internpath_server' target in M1.
