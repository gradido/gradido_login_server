cmake_minimum_required(VERSION 3.0)
project(Gradido_LoginServer C CXX)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin" )

SET ( CMAKE_CXX_FLAGS "-std=c++17" )

include_directories(
	"dependencies" 
	"dependencies/tinf/src/" 
	"dependencies/iroha-ed25519/include"
	"dependencies/mariadb-connector-c/include"
	"dependencies/mariadb-connector-c/build/include"
	"dependencies/spirit-po/include"
	"src/cpp/proto"
	#"dependencies/mariadb-connector-c/build/include"
	#"dependencies/mariadb-connector-c/include"
	#"import/mariadb/include"
)


FILE(GLOB CONTROLLER "src/cpp/controller/*.cpp" "src/cpp/controller/*.h")
FILE(GLOB TINF "dependencies/tinf/src/*.c" "dependencies/tinf/src/*.h")
FILE(GLOB HTTPInterface "src/cpp/HTTPInterface/*.h" "src/cpp/HTTPInterface/*.cpp")
FILE(GLOB JSONInterface "src/cpp/JSONInterface/*.h" "src/cpp/JSONInterface/*.cpp")
FILE(GLOB TASKS "src/cpp/tasks/*.cpp" "src/cpp/tasks/*.h")
FILE(GLOB SINGLETON_MANAGER "src/cpp/SingletonManager/*.h" "src/cpp/SingletonManager/*.cpp")
FILE(GLOB LIB_SRC "src/cpp/lib/*.h" "src/cpp/lib/*.cpp")
FILE(GLOB MODEL "src/cpp/model/*.h" "src/cpp/model/*.cpp")
FILE(GLOB MODEL_TABLE "src/cpp/model/table/*.h" "src/cpp/model/table/*.cpp")
FILE(GLOB MODEL_EMAIL "src/cpp/model/email/*.h" "src/cpp/model/email/*.cpp")
FILE(GLOB CRYPTO "src/cpp/Crypto/*.h" "src/cpp/Crypto/*.cpp")
FILE(GLOB MAIN "src/cpp/*.cpp" "src/cpp/*.c"  "src/cpp/*.h")
FILE(GLOB MYSQL "src/cpp/MySQL/*.cpp" "src/cpp/MySQL/*.h" "src/cpp/MySQL/Poco/*.h")
FILE(GLOB PROTO_GRADIDO "src/cpp/proto/gradido/*.cc" "src/cpp/proto/gradido/*.h")
FILE(GLOB PROTO_HEDERA "src/cpp/proto/hedera/*.cc" "src/cpp/proto/hedera/*.h")

# used only for test project 
FILE(GLOB TEST "src/cpp/test/*.cpp" "src/cpp/test/*.h")
FILE(GLOB TEST_CRYPTO "src/cpp/test/crypto/*.cpp" "src/cpp/test/crypto/*.h")
FILE(GLOB TEST_MODEL "src/cpp/test/model/*.cpp" "src/cpp/test/model/*.h")
FILE(GLOB TEST_MODEL_TABLE "src/cpp/test/model/table/*.cpp" "src/cpp/test/model/table/*.h")
FILE(GLOB TEST_CONTROLLER "src/cpp/test/controller/*.cpp" "src/cpp/test/controller/*.h")

SET(LOCAL_SRCS 
	${CONTROLLER} ${TINF} ${MAIN} ${HTTPInterface} 
	${JSONInterface} ${CRYPTO} ${MODEL} ${MODEL_TABLE} ${MODEL_EMAIL}
	${SINGLETON_MANAGER} ${LIB_SRC} ${MYSQL} ${TASKS}
	${PROTO_GRADIDO} ${PROTO_HEDERA}
)
SET(LOCAL_TEST_SRC
	${TEST} ${TEST_CRYPTO} ${TEST_MODEL} ${TEST_MODEL_TABLE} ${TEST_CONTROLLER}
)
aux_source_directory("src/cpp" LOCAL_SRCS)

if(MSVC)
# src
source_group("controller" FILES ${CONTROLLER})
source_group("proto\\gradido" FILES ${PROTO_GRADIDO})
source_group("proto\\hedera" FILES ${PROTO_HEDERA})
source_group("tinf" FILES ${TINF})
source_group("Crypto" FILES ${CRYPTO})
source_group("tasks" FILES ${TASKS})
source_group("model\\table" FILES ${MODEL_TABLE})
source_group("model\\email" FILES ${MODEL_EMAIL})
source_group("model" FILES ${MODEL})
source_group("mysql" FILES ${MYSQL})
source_group("SingletonManager" FILES ${SINGLETON_MANAGER})
source_group("lib" FILES ${LIB_SRC})
source_group("HTTP-Interface" FILES ${HTTPInterface})
source_group("Json-Interface" FILES ${JSONInterface})
source_group("Test\\crypto" FILES ${TEST_CRYPTO})
source_group("Test\\model\\table" FILES ${TEST_MODEL_TABLE})
source_group("Test\\model" FILES ${TEST_MODEL})
source_group("Test\\controller" FILES ${TEST_CONTROLLER})
source_group("Test" FILES ${TEST})
endif(MSVC)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

#add_subdirectory("dependencies/curl")
#add_subdirectory("dependencies/mariadb-connector-c")


add_executable(Gradido_LoginServer ${LOCAL_SRCS})
#SUBDIRS("src/test")

if(WIN32)

find_library(MYSQL_LIBRARIES mariadbclient.lib PATHS "dependencies/mariadb-connector-c/build/libmariadb/Release" REQUIRED)
#find_library(MYSQL_LIBRARIES_DEBUG  mariadbclient.lib PATHS "import/mariadb/lib/debug")
find_library(COMPILED_MARIADB_CLIENT_DEBUG mariadbclient PATHS "dependencies/mariadb-connector-c/build/libmariadb/Debug" REQUIRED)
find_library(IROHA_ED25519 ed25519 PATHS "dependencies/iroha-ed25519/build/Debug" REQUIRED)
set(MYSQL_INCLUDE_DIR "dependencies/mariadb-connector-c/include")

#set(POCO_DEBUG_PATH "I:/FremdCode/C++/poco/win64/lib/Debug")

#find_library(POCO_DEBUG_FOUNDATION PocoFoundationd PocoFoundation PATHS ${POCO_DEBUG_PATH} REQUIRED)
#find_library(POCO_DEBUG_DATA PocoDatad PocoData PATHS ${POCO_DEBUG_PATH} REQUIRED)
#find_library(POCO_DEBUG_NET PocoNetd PocoNet PATHS ${POCO_DEBUG_PATH} REQUIRED)
#find_library(POCO_DEBUG_NET_SSL PocoNetSSLd PocoNetSSL PATHS ${POCO_DEBUG_PATH} REQUIRED)
#find_library(POCO_DEBUG_UTIL PocoUtild PocoUtil PATHS ${POCO_DEBUG_PATH} REQUIRED)
#find_library(POCO_DEBUG_CRYPTO PocoCryptod PocoCrypto PATHS ${POCO_DEBUG_PATH} REQUIRED)

#set(POCO_DEBUG_LIBS ${POCO_DEBUG_FOUNDATION} ${POCO_DEBUG_UTIL} ${POCO_DEBUG_DATA} ${POCO_DEBUG_NET} ${POCO_DEBUG_NET_SSL} ${POCO_DEBUG_CRYPTO})
#include_directories(
#  "I:/FremdCode/C++/poco/Foundation/include"
#  "I:/FremdCode/C++/poco/Data/include"
#  "I:/FremdCode/C++/poco/Net/include"
  #"I:/FremdCode/C++/poco/NetSSL_Win/include"
#  "I:/FremdCode/C++/poco/NetSSL_OpenSSL/include"
#  "I:/FremdCode/C++/poco/Crypto/include"
#  "I:/FremdCode/C++/poco/Util/include"
#  "I:/FremdCode/C++/ssl/include"
#)

set(CMAKE_CXX_FLAGS         "/MP /EHsc")
#set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

else(WIN32)
find_library(IROHA_ED25519 ed25519 PATHS "dependencies/iroha-ed25519/build" REQUIRED)
# set vars for mariadb cmake files 
set(INSTALL_BINDIR "bin")
set(INSTALL_PLUGINDIR "bin")
add_subdirectory("dependencies/mariadb-connector-c")


include_directories(
	"dependencies/mariadb-connector-c/include"
	"build/dependencies/mariadb-connector-c/include"
)



endif(WIN32)

target_link_libraries(Gradido_LoginServer ${CONAN_LIBS} ${IROHA_ED25519})
if(WIN32)
TARGET_LINK_LIBRARIES(Gradido_LoginServer optimized  ${MYSQL_LIBRARIES} Shlwapi)
TARGET_LINK_LIBRARIES(Gradido_LoginServer debug ${COMPILED_MARIADB_CLIENT_DEBUG} Shlwapi)
else(WIN32)
target_link_libraries(Gradido_LoginServer libmariadb protoc protobuf -pthread)
endif(WIN32)

# install 
if(UNIX)
install(TARGETS Gradido_LoginServer RUNTIME DESTINATION /usr/local/bin)
#install(FILES lib/libmariadb /usr/local/lib)
install(FILES DESTINATION lib COMPONENT libmariadb)
install(DIRECTORY src/LOCALE DESTINATION /etc/grd_login/
        FILES_MATCHING PATTERN "*.po(t)")


endif(UNIX)

enable_testing()

#  ----------------------    Test -----------------------------------------
#project(Gradido_LoginServer_Test C CXX)
#_TEST_BUILD


add_executable(Gradido_LoginServer_Test ${LOCAL_SRCS} ${LOCAL_TEST_SRC})
target_compile_definitions(Gradido_LoginServer_Test PUBLIC "_TEST_BUILD")

target_link_libraries(Gradido_LoginServer_Test ${CONAN_LIBS} ${IROHA_ED25519})

if(WIN32)
	TARGET_LINK_LIBRARIES(Gradido_LoginServer_Test optimized  ${MYSQL_LIBRARIES} Shlwapi)
	TARGET_LINK_LIBRARIES(Gradido_LoginServer_Test debug ${COMPILED_MARIADB_CLIENT_DEBUG} Shlwapi)
else(WIN32)
	target_link_libraries(Gradido_LoginServer_Test libmariadb protoc protobuf -pthread)
endif(WIN32)

add_test(NAME main COMMAND Gradido_LoginServer_Test)
