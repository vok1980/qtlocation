TARGET = qtposition_android_gps
QT = core positioning

PLUGIN_TYPE = position
PLUGIN_CLASS_NAME = QGeoPositionInfoSourceFactoryAndroidGps
load(qt_plugin)


HEADERS = \
    positionfactory_android_gps.h \
    qgeopositioninfosource_android_gps_p.h \


SOURCES = \
	positionfactory_android_gps.cpp \
	qgeopositioninfosource_android_gps.cpp \


OTHER_FILES = plugin.json
