QT       += core gui widgets

CONFIG += c++17

SOURCES += \
    DialogConfirmReplace.cpp \
    MainWidget.cpp \
    RowsUpdater.cpp \
    main.cpp

HEADERS += \
    DialogConfirmReplace.h \
    MainWidget.h \
    RowsUpdater.h \
    Settings.h

INCLUDEPATH += \
    ../include

DEPENDPATH += \
    ../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
