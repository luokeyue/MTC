CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(centralized-lock-table)

#ADD_SUBDIRECTORY(../multi_net_io)

AUX_SOURCE_DIRECTORY(server LOCK_SERVER_SOURCE)
AUX_SOURCE_DIRECTORY(client LOCK_CLIENT_SOURCE)

SET(MESS_CONFIG_DIR "${CMAKE_SOURCE_DIR}/centralized-lock-table")

CONFIGURE_FILE("server/server_config.h.in" "server/server_config.h")

INCLUDE_DIRECTORIES("client")
INCLUDE_DIRECTORIES(include)

# Client Library
ADD_LIBRARY(centralized-lock-client ${LOCK_CLIENT_SOURCE})

IF(debug)
INCLUDE_DIRECTORIES("../multi_net_io/include")
INCLUDE_DIRECTORIES("../multi_net_io/libevent/include")
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/multi_net_io/libevent/include)

ADD_EXECUTABLE(client_test test/client_test.cc)
TARGET_LINK_LIBRARIES(client_test multi_net_io)
TARGET_LINK_LIBRARIES(client_test centralized-lock-client)
ENDIF()

#Server Library
ADD_EXECUTABLE(centralized-lock-server ${LOCK_SERVER_SOURCE})
TARGET_LINK_LIBRARIES(centralized-lock-server multi_net_io)