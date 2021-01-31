add_library(HITMAN3FIX SHARED Main.cpp)

target_compile_options(HITMAN3FIX PUBLIC /GS- /GR-)
target_compile_features(HITMAN3FIX PUBLIC cxx_std_17)

include_directories(HITMAN3FIX ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(HITMAN3FIX PROPERTIES OUTPUT_NAME dinput8)