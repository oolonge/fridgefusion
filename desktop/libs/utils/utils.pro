QT -= gui
CONFIG += c++17 staticlib

TARGET = utils
TEMPLATE = lib

INCLUDEPATH += src/ \
    src/utils/

SOURCES += \
    src/utils/logger.cpp \
    src/utils/logging/BaseLogger.cpp \
    src/utils/logging/FileLogger.cpp \
    src/utils/logging/ConsoleLogger.cpp \
    src/utils/logging/LoggerService.cpp

HEADERS += \
    src/utils/logger.h \
    src/utils/exceptions.h \
    src/utils/helpers.h \
    src/utils/logging/ILogger.h \
    src/utils/logging/BaseLogger.h \
    src/utils/logging/FileLogger.h \
    src/utils/logging/ConsoleLogger.h \
    src/utils/logging/LoggerService.h

# Установка выходного каталога
DESTDIR = $$OUT_PWD/../../lib
