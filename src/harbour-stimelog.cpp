#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>
#include "timelog.h"

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    app->setApplicationDisplayName("Time Log");
    app->setApplicationName("harbour-timelog");
    app->setApplicationVersion(APP_VERSION);
    app->setOrganizationDomain("https://github.com/scharel");
    app->setOrganizationName("harbour-timelog");

    qDebug() << app->applicationDisplayName() << app->applicationVersion();

    TimeLog timeLog;
    QQuickView* view = SailfishApp::createView();
#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif
    view->rootContext()->setContextProperty("timeLog", &timeLog);
    view->setSource(SailfishApp::pathTo("qml/harbour-timelog.qml"));
    view->show();

    return app->exec();
}
