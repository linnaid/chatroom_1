cmake_minimum_required(VERSION 3.10)
project(ChatApp)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加目录到包含路径中
include_directories(${PROJECT_SOURCE_DIR}/Proto)
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/Server/Redis)
include_directories(${PROJECT_SOURCE_DIR}/src/Client/include)
include_directories(${PROJECT_SOURCE_DIR}/src/Server/include)
include_directories(${PROJECT_SOURCE_DIR}/src/Server/Mail)
include_directories(${PROJECT_SOURCE_DIR}/src/Server/FTP_server/include)



# 查找 protobuf（需要你已安装）
find_package(Protobuf REQUIRED)
find_package(CURL REQUIRED)


# 查找 proto 生成的文件
file(GLOB PROTO_SRCS "${PROJECT_SOURCE_DIR}/Proto/*.pb.cc")
file(GLOB PROTO_HDRS "${PROJECT_SOURCE_DIR}/Proto/*.pb.h")

# Client 源文件
file(GLOB CLIENT_SRCS
    "${PROJECT_SOURCE_DIR}/src/Client/UserAuth/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Client/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Client/UserChat/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Client/UserFile/*.cc"
    "${PROJECT_SOURCE_DIR}/src/tools/*.cc"
    "${PROJECT_SOURCE_DIR}/src/include/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Client/Heart/*.cc"

)

# Server 源文件
file(GLOB SERVER_SRCS
    "${PROJECT_SOURCE_DIR}/src/Server/UserAuth/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/Redis/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/Mail/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/Manage/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/net/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/Reactor/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/FTP_server/thread_pool/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/FTP_server/reactor/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/FTP_server/connect/*.cc"
    "${PROJECT_SOURCE_DIR}/src/Server/FTP_server/command/*.cc"
    "${PROJECT_SOURCE_DIR}/src/tools/*.cc"
    "${PROJECT_SOURCE_DIR}/src/include/*.cc"

)

# 添加 Client 可执行文件
add_executable(client
    ${CLIENT_SRCS}
    ${PROTO_SRCS}
)

# 添加 Server 可执行文件
add_executable(server
    ${SERVER_SRCS}
    ${PROTO_SRCS}
)

# 链接库
target_link_libraries(client
    ${Protobuf_LIBRARIES}
    redis++ hiredis
)

target_link_libraries(server
    ${Protobuf_LIBRARIES}
    redis++ hiredis
    CURL::libcurl
)

# 查找 redis++ 和 hiredis 头文件
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# # 在 client/server 可执行文件里加上 redis++ 依赖
# target_link_libraries(client
#     ${Protobuf_LIBRARIES}
#     redis++ hiredis
# )

# target_link_libraries(server
#     ${Protobuf_LIBRARIES}
#     redis++ hiredis
# )



