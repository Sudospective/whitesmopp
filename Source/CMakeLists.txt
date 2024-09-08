add_subdirectory(Libs)

set(Source
	main.cpp
	game-manager.cpp
	server.cpp
	server-manager.cpp
)

add_executable(${PROJECT_NAME} ${Source})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME} 
	socket
)

target_include_directories(${PROJECT_NAME} PUBLIC
	Libs/socket-cpp/Socket/
)
