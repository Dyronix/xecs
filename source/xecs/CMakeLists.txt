# -------------------------------
# Macro that will define our source groups
# -------------------------------
MACRO(GROUPSOURCES curdir folder_name)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    FOREACH(child ${children})
           IF(IS_DIRECTORY ${curdir}/${child})
              GROUPSOURCES(${curdir}/${child} ${folder_name}/${child})
           ELSE()
              SOURCE_GROUP(${folder_name} FILES ${curdir}/${child})
           ENDIF()
    ENDFOREACH()
ENDMACRO(GROUPSOURCES)

# -------------------------------
# Setup exe
# -------------------------------
add_library(${PROJECT_NAME_LOWER})

if(MSVC)
    add_definitions(-D${PROJECT_NAME_UPPER}_COMPILER_MSVC)
endif()

# Sources
## private sources
target_sources(${PROJECT_NAME_LOWER} PRIVATE 
    ### xecs
    ${CMAKE_CURRENT_LIST_DIR}/private/xecs/ecs_world.cpp
    ${CMAKE_CURRENT_LIST_DIR}/private/xecs/ecs_entity.cpp
    ${CMAKE_CURRENT_LIST_DIR}/private/xecs/ecs.cpp)

## public sources
target_sources(${PROJECT_NAME_LOWER} PUBLIC 
    ### xsparse
    ${CMAKE_CURRENT_LIST_DIR}/public/xsparse/xsparse_set.h
    ${CMAKE_CURRENT_LIST_DIR}/public/xsparse/xsparse_map.h
    ### xecs
    ${CMAKE_CURRENT_LIST_DIR}/public/xecs/ecs_world.h
    ${CMAKE_CURRENT_LIST_DIR}/public/xecs/ecs_entity.h
    ${CMAKE_CURRENT_LIST_DIR}/public/xecs/ecs.h)

# Create the project filters
GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)
GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/public public)

# Private includes
target_include_directories(${PROJECT_NAME_LOWER} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/private)
target_include_directories(${PROJECT_NAME_LOWER} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/public)

# Compile definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME_LOWER} PRIVATE ${PROJECT_NAME_UPPER}_PLATFORM_WINDOWS)
endif()
                      
