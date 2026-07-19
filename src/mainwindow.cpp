#include "mainwindow.h"
#include <DMainWindow>
#include <DTitlebar>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
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
