#include "mainwindow.h"
#include <DAboutDialog>
#include <DApplication>
#include <DWidgetUtil>
#include <QApplication>
#include <QTranslator>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setOrganizationName("deepin en español");

    QTranslator appTranslator;
    QString locale = QLocale::system().name();
    if (!appTranslator.load("tetris_" + locale, ":/translations/"))
        if (!appTranslator.load("tetris_" + locale.left(2), ":/translations/"))
            [[maybe_unused]] bool ok = appTranslator.load("tetris_en", ":/translations/");
    a.installTranslator(&appTranslator);

    a.setProductIcon(QIcon(":/icons/tetris.svg"));
    a.setProductName(QApplication::translate("main", "Tetris"));

    DAboutDialog dialog;
    a.setAboutDialog(&dialog);
    dialog.setWindowTitle("Tetris");
    dialog.setProductName("<span>Tetris</span>");
    dialog.setProductIcon(QIcon(":/icons/tetris.svg"));
    dialog.setDescription(
        "<div style='text-align:center;'><img src=':/icons/Logo-Racoon.png' width='128'/></div>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin en Español </span>"
        "<a href='https://deepinenespanol.org'>https://deepinenespanol.org</a><br/>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin Latin Code - developers</span>");
    dialog.setVersion(DApplication::buildVersion("Version 0.2"));
    dialog.setWebsiteName("deepinenespanol.org");
    dialog.setWebsiteLink("https://deepinenespanol.org");

    MainWindow w;
    w.show();
    Dtk::Widget::moveToCenter(&w);

    return a.exec();
}
