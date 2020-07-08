
cmake_minimum_required(VERSION 2.8)
ADD_CUSTOM_TARGET(pl)
Set(FLEX_EXECUTABLE flex)
Set(BISON_EXECUTABLE bison)
SET(CMAKE_C_COMPILER "/usr/bin/gcc")
SET(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_CXX_FLAGS "  -Wall -g  -Wno-unused -Wno-sign-compare ")
LINK_LIBRARIES(-lc -lm -ll)
find_program(LEX_EXE
        flex
        )
find_program(YACC_EXE
        bison
        )


if(LEX_EXE STREQUAL "LEX_EXE-NOTFOUND")
    message(FATAL_ERROR "dear user, plase install flex!")
endif(LEX_EXE STREQUAL "LEX_EXE-NOTFOUND")

find_program(YACC_EXE
        bison
        )
if(YACC_EXE STREQUAL "YACC_EXE-NOTFOUND")
    message(FATAL_ERROR "dear user, plase install bison!")
endif(YACC_EXE STREQUAL "YACC_EXE-NOTFOUND")


ADD_CUSTOM_COMMAND(
        SOURCE ${PROJECT_SOURCE_DIR}/scanner.l
        COMMAND ${FLEX_EXECUTABLE}
        ARGS -d ${PROJECT_SOURCE_DIR}/scanner.l
        OUTPUT lex.yy.c
)

configure_file(${CMAKE_CURRENT_BINARY_DIR}/y.tab.h ${PROJECT_SOURCE_DIR}/include/y.tab.h  COPYONLY)
add_custom_command(
        OUTPUT y.tab.h
        COMMAND ${CMAKE_COMMAND} -E copy
        y.tab.h
        ${PROJECT_SOURCE_DIR}/y.tab.h)


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
set_source_files_properties(  y.tab.c PROPERTIES LANGUAGE CXX )
set_source_files_properties(  lex.yy.c PROPERTIES LANGUAGE CXX )
add_library(
        ext
        y.tab.c
        lex.yy.c
        y.tab.h)


add_executable(
        dcc
        ast.cc
        codegen.cc
        mips.cc
        tac.cc
        ast_decl.cc
        ast_expr.cc
        ast_stmt.cc
        ast_type.cc
        errors.cc
        utility.cc
        main.cc
)

target_link_libraries(dcc ext)

configure_file(${PROJECT_BINARY_DIR}/dcc ${PROJECT_SOURCE_DIR}/dcc  COPYONLY)

#add_custom_command(
#        TARGET dcc POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        ${PROJECT_BINARY_DIR}/dcc
#        ${PROJECT_SOURCE_DIR}/dcc)


#add_custom_command(TARGET dcc POST_BUILD

 #       COMMAND ${CMAKE_COMMAND} -E

  #      copy $<TARGET_FILE:dcc>  ${PROJECT_SOURCE_DIR})

