#include "mainwindow.h"
#include <DMainWindow>
#include <DTitlebar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    w = new Widget;

    // Game: 12 cols × 20 rows, blockSize=35
    // Width:  12(margin) + 420(12×35) + 12(spacing) + 100(side) + 12(margin) = 556
    // Height: 12(margin) + 700(20×35) + 12(margin) = 724
    resize(556, 724);
    setFixedSize(556, 724);
    setWindowTitle("Dropix");

    setCentralWidget(w);
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);

    // Add Settings to the title bar menu
    QMenu *menu = new QMenu(this);
    QAction *settingsAction = menu->addAction(tr("Settings"));
    connect(settingsAction, &QAction::triggered, w, &Widget::openSettings);
    titlebar()->setMenu(menu);

    connect(w, &Widget::musicToggled, this, [this](bool muted) {
        w->soundManager()->setMuted(muted);
    });
}

MainWindow::~MainWindow()
{
}
