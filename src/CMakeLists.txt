set(UTILITY_SRC
    utility/Address.hpp
    utility/Address.cpp
    utility/Config.hpp
    utility/Config.cpp
    utility/Memory.hpp
    utility/Memory.cpp
    utility/Module.hpp
    utility/Module.cpp
    utility/Patch.hpp
    utility/Patch.cpp
    utility/Pattern.hpp
    utility/Pattern.cpp
    utility/RTTI.hpp
    utility/RTTI.cpp
    utility/Scan.hpp
    utility/Scan.cpp
    utility/String.hpp
    utility/String.cpp
)

source_group("Utility" FILES ${UTILITY_SRC})

add_library(HITMAN3FIX SHARED Main.cpp ${UTILITY_SRC})

target_compile_options(HITMAN3FIX PUBLIC /GS- /GR-)
target_compile_features(HITMAN3FIX PUBLIC cxx_std_17)
target_link_libraries(HITMAN3FIX PUBLIC shlwapi)

include_directories(HITMAN3FIX ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(HITMAN3FIX PROPERTIES OUTPUT_NAME dinput8)