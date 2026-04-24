cmake_minimum_required(VERSION 3.10)
project(worm-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES 
    "src/DataSend.cpp"
    "src/NetUtils.cpp"
    "src/worm.cpp"
    "src/main.cpp"
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

if(WIN32 OR MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32          # Для работы сокетов (Winsock2)
        iphlpapi        # Для GetAdaptersAddresses
        user32          # Для MessageBox и работы с окнами (WndProc)
        shell32         # Для SHGetFolderPath
        advapi32        # Для работы с реестром (RegOpenKeyEx)
    )
endif()

add_definitions(-DUNICODE -D_UNICODE)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doxygen
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()