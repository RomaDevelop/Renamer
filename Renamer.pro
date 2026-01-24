QT       += core gui widgets

CONFIG += c++17

SOURCES += \
    MainWidget.cpp \
    main.cpp

HEADERS += \
    MainWidget.h

INCLUDEPATH += \
    ../include

DEPENDPATH += \
    ../include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
