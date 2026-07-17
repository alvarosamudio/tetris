#include "mainwindow.h"
#ifdef Q_OS_LINUX
#include <DAboutDialog>
#include <DApplication>
#include <DWidgetUtil>
DWIDGET_USE_NAMESPACE
#endif
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QScreen>
#include <QTranslator>

static void showAboutDialog()
{
    QMessageBox about;
    about.setWindowTitle("Tetris");
    about.setIconPixmap(QPixmap(":/icons/tetris-deepin.svg").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    about.setText("<div style='text-align:center;'><img src=':/icons/Logo-Racoon.png' width='128'/></div>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin en Español </span>"
        "<a href='https://deepinenespanol.org'>https://deepinenespanol.org</a><br/>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin Latin Code - developers</span><br/><br/>"
        "<span>Version 0.2</span>");
    about.setStandardButtons(QMessageBox::Ok);
    about.exec();
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    DApplication a(argc, argv);
    a.loadTranslator();
#else
    QApplication a(argc, argv);
#endif
    a.setOrganizationName("deepin en español");

    QTranslator appTranslator;
    QString locale = QLocale::system().name();
    [[maybe_unused]] bool loaded = appTranslator.load(
        QLatin1String("tetris_") + locale.left(2),
        QLatin1String(":/translations/"));
    a.installTranslator(&appTranslator);
    a.setWindowIcon(QIcon(":/icons/tetris-deepin.svg"));
    a.setApplicationName("Tetris");

    MainWindow w;
    w.show();

#ifdef Q_OS_LINUX
    a.setProductIcon(QIcon(":/icons/tetris-deepin.svg"));
    a.setProductName(QApplication::translate("main", "Tetris"));

    DAboutDialog dialog;
    a.setAboutDialog(&dialog);
    dialog.setWindowTitle("Tetris");
    dialog.setProductName("<span>Tetris</span>");
    dialog.setProductIcon(QIcon(":/icons/tetris-deepin.svg"));
    dialog.setDescription(
        "<div style='text-align:center;'><img src=':/icons/Logo-Racoon.png' width='128'/></div>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin en Español </span>"
        "<a href='https://deepinenespanol.org'>https://deepinenespanol.org</a><br/>"
        "<span style=' font-size:8pt; font-weight:600;'>Deepin Latin Code - developers</span>");
    dialog.setVersion(DApplication::buildVersion("Version 0.2"));
    dialog.setWebsiteName("deepinenespanol.org");
    dialog.setWebsiteLink("https://deepinenespanol.org");

    Dtk::Widget::moveToCenter(&w);
#else
    QObject::connect(&w, &MainWindow::aboutRequested, &a, [](bool) {
        showAboutDialog();
    });
    w.move(QGuiApplication::primaryScreen()->geometry().center() - w.rect().center());
#endif

    return a.exec();
}
