QT       += core gui quick quickwidgets network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qmltypes

QML_IMPORT_NAME = DataService
QML_IMPORT_MAJOR_VERSION = 1
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    api_caller.cpp \
    dataservice.cpp \
    graphNode.cpp \
    graphWindow.cpp \
    linegraphwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    navbarwidget.cpp \
    testwindow.cpp \
    verbosewidget.cpp \
    websocket_client.cpp \
    zoomgraphicsview.cpp

HEADERS += \
    api_caller.h \
    dataservice.h \
    graphNode.h \
    graphWindow.h \
    linegraphwindow.h \
    mainwindow.h \
    navbarwidget.h \
    testwindow.h \
    verbosewidget.h \
    websocket_client.h \
    zoomgraphicsview.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    MyItem.qml \
    main.qml

RESOURCES += \
    qml.qrc
