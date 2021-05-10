set(target_name App)

set(src_files
    "src/App.h"
    "src/App.cpp"
    "src/main.cpp")

set(all_files
    ${src_files})
    
source_group("src" FILES ${src_files})

add_executable(${target_name} ${all_files})

target_link_libraries(${target_name} Engine Vulkan::Vulkan glfw glm compiler_features compiler_warnings)

set_target_properties(${target_name} PROPERTIES FOLDER ${PROJECT_NAME})
set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)