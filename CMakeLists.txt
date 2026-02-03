cmake_minimum_required (VERSION 3.10)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Terrain")

find_package(PNG REQUIRED)
find_package(glm REQUIRED)
find_package(SDL3 REQUIRED)
find_package(Vulkan REQUIRED)

set(SHADER_COMPILER_EXEC "${Vulkan_GLSLC_EXECUTABLE}")
if (NOT SHADER_COMPILER_EXEC)
    set(SHADER_COMPILER_EXEC "${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}")
endif()

file(GLOB SRC_FILES "src/*.cpp")
file(GLOB RESOURCES_FILES "src/Resources/*.cpp")
file(GLOB RENDER_FILES "src/Render/*.cpp")
file(GLOB VULKAN_FILES "src/Render/Vulkan/*.cpp")
file(GLOB SHADER_FILES "src/Shaders/*.vert" "src/Shaders/*.frag")

# Compile shaders
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/spirv.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/spirv.cmake"
    @ONLY
)

set(SPV_FILES "")
foreach(SHADER ${SHADER_FILES})
    cmake_path(GET SHADER FILENAME shadername)
    set(SPV_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/${shadername}.h)
    list(APPEND SPV_FILES ${SPV_OUTPUT})

    add_custom_command(
        OUTPUT ${SPV_OUTPUT}
        DEPENDS ${SHADER}
        COMMAND ${CMAKE_COMMAND} -DINPUT_FILE=${SHADER} -DOUTPUT_FILE=${SPV_OUTPUT} -P ${CMAKE_CURRENT_BINARY_DIR}/spirv.cmake
        COMMENT "Compiling shader ${SHADER}"
)

endforeach()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${RESOURCES_FILES} ${RENDER_FILES} ${VULKAN_FILES} ${SPV_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC src/)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/generated/)

target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
endif()