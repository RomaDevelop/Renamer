QT       += core gui widgets

CONFIG += c++17

SOURCES += \
    MainWidget.cpp \
    RowsUpdater.cpp \
    WidgetTable.cpp \
    main.cpp

HEADERS += \
    MainWidget.h \
    RowsUpdater.h \
    Settings.h \
    WidgetTable.h

INCLUDEPATH += \
    ../include

DEPENDPATH += \
    ../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
