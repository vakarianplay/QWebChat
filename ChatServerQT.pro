QT = core network websockets

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        chatserver.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    chatserver.h

DISTFILES += \
    index.html

win {
    build_extra_files.target = $$OUT_PWD/index.html
    build_extra_files.commands = copy /Y $$PWD/index.html $$OUT_PWD\
    QMAKE_EXTRA_TARGETS += build_extra_files
    PRE_TARGETDEPS += $$build_extra_files.target
}

unix {
    build_extra_files.target = $$OUT_PWD/index.html
    build_extra_files.commands = cp /Y index.html $$OUT_PWD\
    QMAKE_EXTRA_TARGETS += build_extra_files
    PRE_TARGETDEPS += $$build_extra_files.target
}

