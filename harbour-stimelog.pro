# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-stimelog

CONFIG += sailfishapp

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += src/harbour-stimelog.cpp \
    src/stimelog.cpp

DISTFILES += qml/harbour-stimelog.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    qml/pages/SecondPage.qml \
    rpm/harbour-stimelog.changes.in \
    rpm/harbour-stimelog.changes.run.in \
    rpm/harbour-stimelog.spec \
    rpm/harbour-stimelog.yaml \
    translations/*.ts \
    harbour-stimelog.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-timelog-de.ts

HEADERS += \
    src/stimelog.h