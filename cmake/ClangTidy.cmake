option(RUN_CLANG_TIDY "Enable checks with clang-tidy" OFF)

if(RUN_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
    )
    if(NOT CLANG_TIDY_EXE)
        message(STATUS "clang-tidy not found.")
    else()
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_C_CLANG_TIDY
        clang-tidy;
        -header-filter=.
        --use-color;
        )
        set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        --use-color;
        -extra-arg=-std=c++17;
        -format-style='file';
        -header-filter=${CMAKE_CURRENT_SOURCE_DIR};
        )
    endif()
endif()
