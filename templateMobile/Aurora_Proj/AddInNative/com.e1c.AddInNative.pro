TEMPLATE = lib
TARGET = com_1c_AddInNative

CONFIG += plugin no_plugin_name_prefix

DEFINES += \
    __AURORA__

SRC = ../..

OUT_DIR_NAME = OUT

equals(QT_ARCH, i386) {
    OUT_DIR_NAME = i386
}
equals(QT_ARCH, x86_64) {
    OUT_DIR_NAME = x86_64
}
equals(QT_ARCH, arm64) {
    OUT_DIR_NAME = ARM64
}
equals(QT_ARCH, arm) {
    OUT_DIR_NAME = ARM
}

OUT_PATH = $${PWD}/../build/$${OUT_DIR_NAME}
system($$QMAKE_MKDIR $${OUT_PATH})
QMAKE_POST_LINK += cp $${OUT_PWD}/$${TARGET}.so $${OUT_PATH}/$${TARGET}.so

SOURCES += \
    $${SRC}/stdafx.cpp \
    $${SRC}/AddInNative.cpp \
    $${SRC}/ConversionWchar.cpp \
    $${SRC}/dllmain.cpp

HEADERS += \
    $${SRC}/AddInNative.h \
    $${SRC}/ConversionWchar.h

DISTFILES += \
    rpm/com.e1c.AddInNative.spec \
    $${SRC}/AddInNative.def
