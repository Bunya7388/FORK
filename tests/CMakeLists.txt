enable_testing()

# Example test framework
add_executable(test_worker_pool ${CMAKE_CURRENT_SOURCE_DIR}/test_worker_pool.cpp ${CMAKE_SOURCE_DIR}/src/worker_pool.cpp)
target_include_directories(test_worker_pool PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_worker_pool Threads::Threads)

add_test(NAME WorkerPoolTest COMMAND test_worker_pool)

add_executable(test_session_manager ${CMAKE_CURRENT_SOURCE_DIR}/test_session_manager.cpp ${CMAKE_SOURCE_DIR}/src/session_manager.cpp)
target_include_directories(test_session_manager PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_session_manager Threads::Threads)

add_test(NAME SessionManagerTest COMMAND test_session_manager)

add_executable(test_pubkey ${CMAKE_CURRENT_SOURCE_DIR}/test_pubkey.cpp ${CMAKE_SOURCE_DIR}/src/pubkey_generator.cpp)
target_include_directories(test_pubkey PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_pubkey OpenSSL::SSL OpenSSL::Crypto)

add_test(NAME PubKeyTest COMMAND test_pubkey)
