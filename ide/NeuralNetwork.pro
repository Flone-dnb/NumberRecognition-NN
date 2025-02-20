QT       += core gui
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../ext/qcustomplot/qcustomplot.cpp \
    ../src/Controller/networkcontroller.cpp \
    ../src/Model/Connection/connection.cpp \
    ../src/Model/Layer/layer.cpp \
    ../src/Model/NeuralNetwork/neuralnetwork.cpp \
    ../src/Model/Neuron/neuron.cpp \
    ../src/View/ImageWidget/imagewidget.cpp \
    ../src/main.cpp \
    ../src/View/MainWindow/mainwindow.cpp

HEADERS += \
    ../ext/qcustomplot/qcustomplot.h \
    ../src/Controller/networkcontroller.h \
    ../src/Model/Connection/connection.h \
    ../src/Model/Layer/layer.h \
    ../src/Model/NeuralNetwork/neuralnetwork.h \
    ../src/Model/Neuron/neuron.h \
    ../src/View/MainWindow/mainwindow.h \
    ../src/View/ImageWidget/imagewidget.h

FORMS += \
    ../src/View/MainWindow/mainwindow.ui \
    ../src/View/ImageWidget/imagewidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
