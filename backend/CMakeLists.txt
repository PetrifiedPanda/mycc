set(This mycc-backend)

file(GLOB Sources src/*.c)

add_library(${This} OBJECT ${Sources})
target_include_directories(${This} PUBLIC ./include)
target_link_libraries(${This} ${Util})
