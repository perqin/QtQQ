TEMPLATE = app
TARGET = QtQQ
VERSION = 0.0.1
DEFINES += VER=\\\"$$VERSION\\\"

#TRANSLATIONS = 9store_zh_CN.ts

QT += network webkit

#DEFINES += BUILDING_LIBCURL CURL_STATICLIB

SOURCES += main.cpp
#SOURCES += main.cpp \
#    src/Fileopt.cpp \
#    src/Qcurl.cpp

#HEADERS += \
#    src/Fileopt.h \
#    src/Qcurl.h

#include(curl-7.37.0/lib/curl.pri)
#include(curl-7.37.0/lib/vtls/vtls.pri)

#INCLUDEPATH += curl-7.37.0/include\
#               curl-7.37.0/lib

#folder_Meego.source = qml/meego
#folder_Meego.target = qml

#folder_Symbian3.source = qml/symbian3
#folder_Symbian3.target = qml

folder_Symbian1.source = qml/symbian1
folder_Symbian1.target = qml

#folder_pic.source = qml/pic
#folder_pic.target = qml

#folder_JS.source = qml/JavaScript
#folder_JS.target = qml

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

simulator{
    DEPLOYMENTFOLDERS += folder_Symbian1
}

#contains(MEEGO_EDITION,harmattan){
#    DEFINES += Q_OS_HARMATTAN
#    CONFIG += qdeclarative-boostable meegotouch
#    DEPLOYMENTFOLDERS += folder_Meego folder_pic folder_JS
#    OTHER_FILES += \
#        qtc_packaging/debian_harmattan/rules \
#        qtc_packaging/debian_harmattan/README \
#        qtc_packaging/debian_harmattan/manifest.aegis \
#        qtc_packaging/debian_harmattan/copyright \
#        qtc_packaging/debian_harmattan/control \
#        qtc_packaging/debian_harmattan/compat \
#        qtc_packaging/debian_harmattan/changelog
#}

symbian{
    #DEPLOYMENTFOLDERS = folder_Pic    -- for common resources
    contains(QT_VERSION, 4.7.3){
        DEFINES += Q_OS_S60V5
        DEPLOYMENTFOLDERS += folder_Symbian1
    } else {
        DEPLOYMENTFOLDERS += folder_Symbian3
    }
    vendorinfo = "%{\"Perqin\"}" ":\"Perqin\""
    my_deployment.pkg_prerules += vendorinfo
    DEPLOYMENT += my_deployment

    CONFIG += localize_deployment
    TARGET.UID3 = 0x205051AC
    TARGET.CAPABILITY += NetworkServices

    DEFINES -= VER=\\\"$$VERSION\\\"
    DEFINES += VER=\"$$VERSION\"
    #addFiles.sources = 9store_zh_CN.qm
    #addFiles.path = .
    #DEPLOYMENT += addFiles
}

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()
