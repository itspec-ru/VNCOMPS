TEMPLATE = lib
TARGET = com_1c_StepCounter

CONFIG += plugin no_plugin_name_prefix

DEFINES += \
    __AURORA__

QT += sensors

SOURCES_PATH = ../..

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
    $${SOURCES_PATH}/stdafx.cpp \
    $${SOURCES_PATH}/AddInNative.cpp \
    $${SOURCES_PATH}/ConversionWchar.cpp \
    $${SOURCES_PATH}/StepCounter.cpp \
    src/StepCounterImpl.cpp
    $${SOURCES_PATH}/dllmain.cpp \

HEADERS += \
    $${SOURCES_PATH}/stdafx.h \
    $${SOURCES_PATH}/AddInNative.h \
    $${SOURCES_PATH}/ConversionWchar.h \
    $${SOURCES_PATH}/StepCounter.h \
    src/StepCounterImpl.h

DISTFILES += \
    $${SOURCES_PATH}/AddInNative.def \
    rpm/com.e1c.com_1c_StepCounter.spec
