# Create directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated/shaders/)

# Extract filename
cmake_path(GET INPUT_FILE FILENAME shadername)

# Compile the shader to a temporary binary file
execute_process(
    COMMAND glslc ${INPUT_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/${shadername}.spv
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Read the binary data in HEX format
file(READ ${CMAKE_CURRENT_BINARY_DIR}/Shaders/${shadername}.spv filedata HEX)

# Convert hex data for C compatibility (insert 0x prefix and commas)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})

# Shader binary blob name
string(REGEX REPLACE "[.]" "_" variable_name ${shadername})
set(variable_name g_${variable_name})

# Write the data to the output header file
file(WRITE ${OUTPUT_FILE} "") # Create/empty the file
file(APPEND ${OUTPUT_FILE} "#include \"Render/Vulkan/Pipeline.h\"\n\n")
file(APPEND ${OUTPUT_FILE} "const uint8_t ${variable_name}_code[] = {${filedata}};\n")
file(APPEND ${OUTPUT_FILE} "const Render::ShaderCode ${variable_name} = { ${variable_name}_code, sizeof(${variable_name}_code) };\n")