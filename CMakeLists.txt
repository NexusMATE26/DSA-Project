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

# Milestone 4 HTTP transport layer and API server
add_library(internpath_api STATIC
    src/api/HttpServer.cpp
    src/api/Routes.cpp
)
target_include_directories(internpath_api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(internpath_api PUBLIC internpath_domain)
if(WIN32)
    target_link_libraries(internpath_api PUBLIC ws2_32 crypt32)
endif()

add_executable(internpath_server src/server/main.cpp)
target_link_libraries(internpath_server PRIVATE internpath_api)

# Standalone CLI version for terminal management, testing, and viva demos
add_executable(internpath_cli src/cli/main.cpp)
target_link_libraries(internpath_cli PRIVATE internpath_domain)


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

    add_executable(test_api_smoke tests/test_api_smoke.cpp)
    target_link_libraries(test_api_smoke PRIVATE internpath_api)
    add_test(NAME api_smoke COMMAND test_api_smoke)

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
