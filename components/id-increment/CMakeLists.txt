cmake_minimum_required(VERSION 2.8.10)

project("id-increment")

# list(APPEND CMAKE_INCLUDE_PATH $ENV{BRPC_LEVELDB_INCLUDE})
# list(APPEND CMAKE_INCLUDE_PATH $ENV{BRPC_PROTOBUF_INCLUDE})
# list(APPEND CMAKE_INCLUDE_PATH $ENV{BRPC_GFLAGS_INCLUDE})
# list(APPEND CMAKE_INCLUDE_PATH $ENV{BRPC_SSL_INCLUDE})
# list(APPEND CMAKE_INCLUDE_PATH $ENV{BRPC_INCLUDE})

# list(APPEND CMAKE_LIBRARY_PATH $ENV{BRPC_LEVELDB_LIB})
# list(APPEND CMAKE_LIBRARY_PATH $ENV{BRPC_PROTOBUF_LIB})
# list(APPEND CMAKE_LIBRARY_PATH $ENV{BRPC_GFLAGS_LIB})
# list(APPEND CMAKE_LIBRARY_PATH $ENV{BRPC_SSL_LIB})
# list(APPEND CMAKE_LIBRARY_PATH $ENV{BRPC_LIB})

include_directories($ENV{BRPC_LEVELDB_INCLUDE})
include_directories($ENV{BRPC_PROTOBUF_INCLUDE})
include_directories($ENV{BRPC_GFLAGS_INCLUDE})
include_directories($ENV{BRPC_SSL_INCLUDE})
include_directories($ENV{BRPC_INCLUDE})

# SET(BRPC_STATIC_DEP_LIB
#     ${DEP_STATIC_LIB_DIR}/libprotobuf.a
#     ${DEP_STATIC_LIB_DIR}/libprotoc.a
#     ${DEP_STATIC_LIB_DIR}/libleveldb.a
#     ${DEP_STATIC_LIB_DIR}/libgflags.a
#     #${DEP_STATIC_LIB_DIR}/libssl.a
#     #${DEP_STATIC_LIB_DIR}/libcrypto.a
#     libssl.so
#     libcrypto.so
#     snappy
# )

SET(BRPC_DYNAMIC_DEP_LIB
    $ENV{BRPC_PROTOBUF_LIB}
   # ${DEP_STATIC_LIB_DIR}/libprotobuf.a
    $ENV{BRPC_LEVELDB_LIB}
    $ENV{BRPC_GFLAGS_LIB}
    libssl.so
    libcrypto.so
)

SET(BRPC_DYNAMIC_LIB
    $ENV{BRPC_LIB}
)


# SET(BRPC_STATIC_LIB
#     ${DEP_STATIC_LIB_DIR}/libbrpc.a
# )

set(BRPC_LINK_LIB
    ${BRPC_DYNAMIC_LIB}
    ${BRPC_DYNAMIC_DEP_LIB}
)


add_executable(client client.cpp ${PROTO_HEADER} id.pb.cc)
add_executable(server server.cpp ${PROTO_HEADER} id.pb.cc)

target_link_libraries(client  ${BRPC_LINK_LIB})
target_link_libraries(server  ${BRPC_LINK_LIB})
