cmake_minimum_required(VERSION 3.15)
# call with cmake -P generateOffloadingCode from folder with mpi.h and *.in files

function(get_blocking_name NONBLOCKING_NAME outVar)
    if("${NONBLOCKING_NAME}" MATCHES "MPI_I(.)(.+)")
        string(TOUPPER "${CMAKE_MATCH_1}" FIRST_LETTER)
        set(REMAINDER "${CMAKE_MATCH_2}")
        set(${outVar} "MPI_${FIRST_LETTER}${REMAINDER}" PARENT_SCOPE)
    elseif("${NONBLOCKING_NAME}" MATCHES "MPI_([^_]+)_i(.+)")
        set(FRONT "${CMAKE_MATCH_1}")
        set(BACK "${CMAKE_MATCH_2}")
        set(${outVar} "MPI_${FRONT}_${BACK}" PARENT_SCOPE)
    endif()

endfunction()

function(get_function_bodies FUNCTION_SIGNATURE outVarNonBlocking outVarBlocking)
    string(REGEX MATCH "(MPI[^ \n\r(]+)\\(([^)]*)\\)" FUNCTION_NAME "${FUNCTION_SIGNATURE}")
    set(FUNCTION_NAME "${CMAKE_MATCH_1}")
    string(STRIP "${CMAKE_MATCH_2}" FUNCTION_PARAMETER_W_TYPES)
    string(REGEX MATCHALL "[^,]+" FUNCTION_PARAMETER_W_TYPES_LIST "${FUNCTION_PARAMETER_W_TYPES}") 

    set(FUNCTION_PARAMETER_LIST "")
    foreach(PARA IN LISTS FUNCTION_PARAMETER_W_TYPES_LIST)
        string(REGEX MATCH "([^* \[]+) ?\\[?\\]?$" PARA "${PARA}")
        list(APPEND FUNCTION_PARAMETER_LIST "${CMAKE_MATCH_1}")
    endforeach()
    # set(FUNCTION_PARAMETER_WO_REQ ${FUNCTION_PARAMETER_LIST})
    list(POP_BACK FUNCTION_PARAMETER_LIST FUNCTION_PARAMETER_REQ)
    list(JOIN FUNCTION_PARAMETER_LIST ", " FUNCTION_PARAMETER_WO_REQ)

    file(READ body_nonblocking.cpp.in NONBLOCKING_BODY)
    string(CONFIGURE "${NONBLOCKING_BODY}" NONBLOCKING_DEFINTION)

    # message("${FUNCTION_SIGNATURE}")
    # message("${FUNCTION_NAME}")
    # message("${FUNCTION_PARAMETER_W_TYPES_LIST}")
    # message("${FUNCTION_PARAMETER_LIST}")
    # message("${NONBLOCKING_DEFINTION}")

    get_blocking_name("${FUNCTION_NAME}" FUNCTION_NAME_BLOCKING)
    # message("${FUNCTION_NAME_BLOCKING}")

    list(POP_BACK FUNCTION_PARAMETER_W_TYPES_LIST)
    list(JOIN FUNCTION_PARAMETER_W_TYPES_LIST ", " FUNCTION_PARAMETER_W_TYPES_WO_REQ)

    file(READ body_blocking.cpp.in BLOCKING_BODY)
    string(CONFIGURE "${BLOCKING_BODY}" BLOCKING_DEFINTION)

    set(${outVarNonBlocking} "${NONBLOCKING_DEFINTION}" PARENT_SCOPE)
    set(${outVarBlocking} "${BLOCKING_DEFINTION}" PARENT_SCOPE)
endfunction()

file(READ mpi.h MPI_HEADER)
string(REGEX MATCHALL "int MPI_I[^ ]+\\([^;]*" MPI_NONBLOCK "${MPI_HEADER}")
# string(REGEX MATCHALL "int MPI_[^I][^ ]+\\([^;]*" MPI_BLOCK "${MPI_HEADER}")
string(REGEX MATCHALL "int MPI_Comm_i[^ ]+\\([^;]*" MPI_NONBLOCK_EXTRA "${MPI_HEADER}")
list(APPEND MPI_NONBLOCK ${MPI_NONBLOCK_EXTRA})
string(REGEX MATCHALL "int MPI_File_i[^ ]+\\([^;]*" MPI_NONBLOCK_EXTRA "${MPI_HEADER}")
list(APPEND MPI_NONBLOCK ${MPI_NONBLOCK_EXTRA})

# message("${MPI_BLOCK}")

set(FUNCTION_DEFINITIONS "")
foreach(MPI_SIG IN LISTS MPI_NONBLOCK)
    if("${MPI_SIG}" MATCHES "MPI_Info" OR
       "${MPI_SIG}" MATCHES "MPI_Init" OR
       "${MPI_SIG}" MATCHES "MPI_Is_" OR
       "${MPI_SIG}" MATCHES "MPI_Intercom" OR
       "${MPI_SIG}" MATCHES "MPI_Im?probe")
        continue()
    endif()
    get_function_bodies("${MPI_SIG}" NONBLOCKING_DEFINTION BLOCKING_DEFINTION)
    string(APPEND FUNCTION_DEFINITIONS "\n\n\n${NONBLOCKING_DEFINTION}")
    string(APPEND FUNCTION_DEFINITIONS "\n\n${BLOCKING_DEFINTION}")
    # break()
endforeach()

file(WRITE offloading_auto_generated.cpp "${FUNCTION_DEFINITIONS}")
