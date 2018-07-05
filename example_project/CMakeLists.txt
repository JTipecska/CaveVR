cmake_minimum_required(VERSION 2.8)
# Project Name
PROJECT(MyProject)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/modules/") 


set(project_LIBRARIES)
##################################################################################################################
#list(APPEND project_LIBRARIES pthread)
find_package(synch REQUIRED)
list(APPEND project_LIBRARIES synch )
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/exe/)


find_package(Magnum REQUIRED
	GL
	GlfwApplication
    MeshTools
    Shaders
    SceneGraph
    Trade
    Shapes
    Primitives)
	
list(APPEND project_LIBRARIES
	Magnum::GlfwApplication
	Magnum::GL

    Magnum::Magnum
    Magnum::Shaders
    Magnum::MeshTools
    Magnum::SceneGraph
    Magnum::Trade
    Magnum::Shapes
    Magnum::Primitives)

   
##################################################################################################################

include_directories(${CMAKE_SOURCE_DIR} )

add_executable(${PROJECT_NAME} src/MyProject.cpp)
target_link_libraries(${PROJECT_NAME} ${project_LIBRARIES})

add_executable(${PROJECT_NAME}_server src/MyProject_server.cpp)
target_link_libraries(${PROJECT_NAME}_server ${project_LIBRARIES})



