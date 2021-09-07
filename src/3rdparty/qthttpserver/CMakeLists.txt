project(QtHttpServer)

set(CMAKE_AUTOMOC ON)

find_package(Qt5
    REQUIRED
    COMPONENTS Core Network Concurrent
)

add_library(QtSslServer SHARED
    qthttpserver/src/sslserver/qsslserver.cpp
)
set_target_properties(QtSslServer
    PROPERTIES
    DEFINE_SYMBOL "QT_BUILD_SSLSERVER_LIB"
)

target_link_libraries(QtSslServer
    PUBLIC Qt5::Core Qt5::Network Qt5::CorePrivate
)

target_include_directories(QtSslServer
    PUBLIC
        qthttpserver/src/sslserver/
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB QtSslServer_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/qthttpserver/src/sslserver/*.h"
)
list(FILTER QtHttpServer_HEADERS EXCLUDE REGEX "_p.h")

file(GLOB QtSslServer_private_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/qthttpserver/src/sslserver/*_p.h"
)

file(
    COPY ${QtSslServer_HEADERS}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/QtSslServer/"
)
file(
    COPY ${QtSslServer_private_HEADERS}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/private/"
)
file(
    COPY "${CMAKE_CURRENT_SOURCE_DIR}/headers/QtSslServer"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/QtSslServer/"
)

add_library(QtHttpServer SHARED
    qthttpserver/src/httpserver/qabstracthttpserver.cpp
    qthttpserver/src/httpserver/qhttpserver.cpp
    qthttpserver/src/httpserver/qhttpserverliterals.cpp
    qthttpserver/src/httpserver/qhttpserverrequest.cpp
    qthttpserver/src/httpserver/qhttpserverresponder.cpp
    qthttpserver/src/httpserver/qhttpserverresponse.cpp
    qthttpserver/src/httpserver/qhttpserverfutureresponse.cpp
    qthttpserver/src/httpserver/qhttpserverrouter.cpp
    qthttpserver/src/httpserver/qhttpserverrouterrule.cpp
    qthttpserver/src/3rdparty/http-parser/http_parser.c
)
set_target_properties(QtHttpServer
    PROPERTIES
    DEFINE_SYMBOL "QT_BUILD_HTTPSERVER_LIB"
)

target_link_libraries(QtHttpServer
    PUBLIC Qt5::Core Qt5::Concurrent Qt5::Network Qt5::CorePrivate QtSslServer
)

target_include_directories(QtHttpServer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/qthttpserver/
        qthttpserver/src/httpserver/
        qthttpserver/src/3rdparty/http-parser/
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB QtHttpServer_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/qthttpserver/src/httpserver/*.h"
)
list(FILTER QtHttpServer_HEADERS EXCLUDE REGEX "_p.h")

file(GLOB QtHttpServer_private_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/qthttpserver/src/httpserver/*_p.h"
)

file(
    COPY ${QtHttpServer_HEADERS}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/QtHttpServer/"
)
file(
    COPY ${QtHttpServer_private_HEADERS}
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/private/"
)
file(
    COPY "${CMAKE_CURRENT_SOURCE_DIR}/headers/QtHttpServer"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/QtHttpServer/"
)
