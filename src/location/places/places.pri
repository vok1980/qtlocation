#TODO: remove when plugin is refactored out
INCLUDEPATH += places

PUBLIC_HEADERS += \
#data classes
    places/qplaceattribute.h \
    places/qplacecategory.h \
    places/qplacecontent.h \
    places/qplacecontentreply.h \
    places/qplacedescription.h \
    places/qplaceimage.h \
    places/qplacepaginationlist.h \
    places/qplaceperiod.h \
    places/qplacerating.h \
    places/qplacereview.h \
    places/qplacesupplier.h \
#result
    places/qplacesearchresult.h \
#query classes
    places/qplacerequest.h \
    places/qplacesearchrequest.h \
#reply classes
    places/qplacereply.h \
    places/qplacedetailsreply.h \
    places/qplacereviewreply.h \
    places/qplacesavereply.h \
    places/qplacesearchreply.h \
    places/qplacetextpredictionreply.h \
#manager and engine
    places/qplacemanager.h \
    places/qplacemanagerengine.h \
    places/qplacemanagerenginefactory.h

PRIVATE_HEADERS += \
    places/qplaceattribute_p.h \
    places/qplacecategory_p.h \
    places/qplacecontent_p.h \
    places/qplacedescription_p.h \
    places/qplaceimage_p.h \
    places/qplaceperiod_p.h \
    places/qplacerating_p.h \
    places/qplacereview_p.h \
    places/qplacesupplier_p.h \
    places/qplacesearchresult_p.h \
    places/qplacereply_p.h \
    places/qplacemanager_p.h \
    places/qplacemanagerengine_p.h

SOURCES += \
#data classes
    places/qplaceattribute.cpp \
    places/qplacecategory.cpp \
    places/qplacecontent.cpp \
    places/qplacecontentreply.cpp \
    places/qplacedescription.cpp \
#result
    places/qplaceimage.cpp \
    places/qplaceperiod.cpp \
    places/qplacerating.cpp \
    places/qplacereview.cpp \
    places/qplacesavereply.cpp \
    places/qplacesupplier.cpp \
#result
    places/qplacesearchresult.cpp \
#query classes
    places/qplacerequest.cpp \
    places/qplacesearchrequest.cpp \
#reply classes
    places/qplacereply.cpp \
    places/qplacedetailsreply.cpp \
    places/qplacereviewreply.cpp \
    places/qplacesearchreply.cpp \
    places/qplacetextpredictionreply.cpp \
#manager and engine
    places/qplacemanager.cpp \
    places/qplacemanagerengine.cpp \
    places/qplacemanager_p.cpp