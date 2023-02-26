cmake_minimum_required(VERSION 2.8)
project(FTP_Project)
SET ( CMAKE_C_COMPILER "g++" )
ADD_EXECUTABLE (test src/server_main.cpp src/socket_thread.cpp src/socket.cpp src/socket_process.cpp )
ADD_EXECUTABLE (test_client src/test_client_main.cpp src/socket_process.cpp )
INCLUDE_DIRECTORIES ( /raid/test/inc )
TARGET_LINK_LIBRARIES ( test pthread )


#cmake CMakeLists.txt

#old
###g++ -o test server_main.c socket_thread.c socket.c socket_process.c  -lpthread
###g++ -o test_client test_client_main.c socket_process.c 