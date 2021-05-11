set(target_name Engine)

set(core_files
    "src/Core/Types.h"
    "src/Core/Logger.h"
    "src/Core/Logger.cpp")

set(graphics_files
    "src/Graphics/Model.h"
    "src/Graphics/Model.cpp"
    "src/Graphics/SwapChain.h"
    "src/Graphics/SwapChain.cpp"
    "src/Graphics/Device.h"
    "src/Graphics/Device.cpp"
    "src/Graphics/Pipeline.h"
    "src/Graphics/Pipeline.cpp"
    "src/Graphics/Window.h"
    "src/Graphics/Window.cpp")

set(src_files
    "src/GameObject.h")

set(all_files
    ${src_files}
    ${core_files}
    ${graphics_files})

add_library(${target_name} STATIC ${all_files} ${SHADERS})

compile_shaders(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_HOME_DIRECTORY}/bin/shaders)

target_precompile_headers(${target_name}
PRIVATE
  <string>
  <vector>
  <unordered_map>
  <memory>
  <algorithm>
  <functional>
  <queue>
  <cmath>
  <random>
  <filesystem>
  <variant>
  <execution>

  "src/Core/Logger.h"
)

target_link_libraries(${target_name} Vulkan::Vulkan glfw glm color-console compiler_features compiler_warnings)
target_include_directories(${target_name} PUBLIC "src")

source_group("src" FILES ${src_files})
source_group("src\\Core" FILES ${core_files})
source_group("src\\Graphics" FILES ${graphics_files})

set_target_properties(${target_name} PROPERTIES FOLDER ${PROJECT_NAME})
set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)