TARGET = qtposition_android_gms
QT = core positioning

PLUGIN_TYPE = position
PLUGIN_CLASS_NAME = QGeoPositionInfoSourceFactoryAndroidGms
load(qt_plugin)


HEADERS = \
    positionfactory_android_gms.h \
    qgeopositioninfosource_android_gms_p.h \


SOURCES = \
	positionfactory_android_gms.cpp \
	qgeopositioninfosource_android_gms.cpp \


OTHER_FILES = plugin.json
