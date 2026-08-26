QT += core gui widgets sql

TARGET = FridgeFusion
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp

INCLUDEPATH += \
    ../libs/core/src/ \
    ../libs/infrastructure/src/ \
    ../libs/application/src/ \
    ../libs/presentation/src/ \
    ../libs/utils/src/

macx {
    INCLUDEPATH += /opt/homebrew/opt/libpq/include
    LIBS += -L/opt/homebrew/opt/libpq/lib -lpq
    QMAKE_RPATHDIR += /opt/homebrew/opt/libpq/lib
    QMAKE_LFLAGS += -Wl,-rpath,/opt/homebrew/opt/libpq/lib
    QMAKE_LIBS_OPENGL = -framework OpenGL
}

LIBS += -L$$OUT_PWD/../lib -lpresentation -lapplication -linfrastructure -lcore -lutils

DESTDIR = $$OUT_PWD/../bin
