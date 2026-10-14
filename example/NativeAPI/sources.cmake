#--source files-------------------
set(SOURCES
    localization.cpp
    dllmain.cpp
    AddInNative.cpp
    stdafx.cpp
    PipeChannel.cpp
)

set(HEADERS
    AddInNative.h
    stdafx.h
    localization.h
    PipeChannel.h
)

set(SRC ${SOURCES})
list(APPEND SRC ${HEADERS})

if (WIN32)
    LIST(APPEND SRC ${CMAKE_CURRENT_BINARY_DIR}/AddInNative.rc ${CMAKE_CURRENT_BINARY_DIR}/AddInNative.def)
endif()
#--source files end---------------