QT       += core gui
QT      += serialbus widgets
QT      += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ModbusMaster/modbusmaster.cpp \
    Plots/ltr11.cpp \
    Plots/ltr22.cpp \
    Plots/plot.cpp \
    Plots/plotter.cpp \
    Plots/samplingthread.cpp \
    Plots/signaldata.cpp \
    Saving/filesaver.cpp \
    Settings/globalplotsettingsdialog.cpp \
    Settings/modbussettingsdialog.cpp \
    Settings/plotsettingsdialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ModbusMaster/modbusmaster.h \
    Plots/ltr11.h \
    Plots/ltr22.h \
    Plots/plot.h \
    Plots/plotter.h \
    Plots/samplingthread.h \
    Plots/signaldata.h \
    Saving/filesaver.h \
    Settings/globalplotsettingsdialog.h \
    Settings/modbussettingsdialog.h \
    Settings/plotsettingsdialog.h \
    mainwindow.h

FORMS += \
    Saving/filesaver.ui \
    Settings/globalplotsettingsdialog.ui \
    Settings/modbussettingsdialog.ui \
    Settings/plotsettingsdialog.ui \
    mainwindow.ui





# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr11api
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr11api
#else:unix: LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr11api

INCLUDEPATH += $$PWD/../Libs/ltr/include
DEPENDPATH += $$PWD/../Libs/ltr/include

#win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw64/ -lltr22api
#else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw64/ -lltr22api
#else:unix: LIBS += -L$$PWD/../Libs/ltr/lib/mingw64/ -lltr22api
LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr11api
LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr22api

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr22api
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr22api
#else:unix: LIBS += -L$$PWD/../Libs/ltr/lib/mingw/ -lltr22api


#win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw64/ -lltr11api
#else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../Libs/ltr/lib/mingw64/ -lltr11api
#else:unix: LIBS += -L$$PWD/../Libs/ltr/lib/mingw64/ -lltr11api

RESOURCES += \
    Resources/Resources.qrc \
    Saving/SaveParameters.qrc

CONFIG += qwt

DISTFILES +=

