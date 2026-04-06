cmake_minimum_required(VERSION 3.10)
project(dnstt-udp VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O3 -march=native -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -O3 -march=native -fPIC")

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Source files
set(SERVER_SOURCES
    src/main.cpp
    src/udp_server.cpp
    src/worker_pool.cpp
    src/session_manager.cpp
    src/performance_monitor.cpp
    src/pubkey_generator.cpp
    src/mtu_manager.cpp
    src/config.cpp
)

set(SERVER_HEADERS
    include/udp_server.h
    include/worker_pool.h
    include/session_manager.h
    include/performance_monitor.h
    include/pubkey_generator.h
    include/mtu_manager.h
    include/config.h
    include/common.h
)

# Create executable
add_executable(dnstt-udp ${SERVER_SOURCES} ${SERVER_HEADERS})

# Link libraries
target_link_libraries(dnstt-udp
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# Include directories
target_include_directories(dnstt-udp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Compiler-specific optimizations
if(MSVC)
    target_compile_options(dnstt-udp PRIVATE /O2 /W4)
else()
    target_compile_options(dnstt-udp PRIVATE -O3 -ffast-math)
endif()

# Installation
install(TARGETS dnstt-udp DESTINATION bin)
install(FILES README.md DESTINATION share/doc/dnstt-udp)
install(DIRECTORY include/ DESTINATION include/dnstt-udp)

# Testing
enable_testing()
add_subdirectory(tests)

# Packaging
set(CPACK_PACKAGE_NAME "dnstt-udp")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VENDOR "DNS Tunneling")
include(CPack)
