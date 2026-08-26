TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    libs/core \
    libs/utils \
    libs/infrastructure \
    libs/application \
    libs/presentation \
    app

# Установка зависимостей
app.depends = libs/core libs/utils libs/infrastructure libs/application libs/presentation
libs/infrastructure.depends = libs/core libs/utils
libs/application.depends = libs/core libs/utils libs/infrastructure
libs/presentation.depends = libs/core libs/utils libs/infrastructure libs/application

# Пути для PostgreSQL для macOS
macx {
    INCLUDEPATH += /opt/homebrew/opt/libpq/include
    LIBS += -L/opt/homebrew/opt/libpq/lib -lpq
    QMAKE_RPATHDIR += /opt/homebrew/opt/libpq/lib
    QMAKE_LFLAGS += -Wl,-rpath,/opt/homebrew/opt/libpq/lib
    CONFIG += sdk_no_version_check
    QMAKE_MAC_SDK = macosx
}
