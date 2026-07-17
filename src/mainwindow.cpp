#include "mainwindow.h"
#ifdef Q_OS_LINUX
#include <DMainWindow>
DWIDGET_USE_NAMESPACE
#endif
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : MAINWINDOW_BASE(parent)
{
    w = new Widget;

    resize(410, 630);
    setMinimumSize(410, 630);
    setMaximumSize(410, 630);

    setCentralWidget(w);
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);

    connect(w, &Widget::musicToggled, this, [this](bool muted) {
        w->soundManager()->setMuted(muted);
    });
}

MainWindow::~MainWindow()
{
}
