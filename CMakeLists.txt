cmake_minimum_required(VERSION 4.0)
project("magent-winrt-musicstatus" C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)

add_library(${PROJECT_NAME} SHARED main.c hooks.c trampoline.c smtc.c)

if (MSVC)
add_compile_definitions(_UNICODE=1 UNICODE=1)
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
target_link_libraries(${PROJECT_NAME} mincore.lib runtimeobject.lib) 
target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)

else ()
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_compile_options(${PROJECT_NAME} PRIVATE -municode -Wall -Wextra -Werror -Wno-pointer-to-int-cast -O2)
target_link_libraries(${PROJECT_NAME} mincore runtimeobject) 
endif()