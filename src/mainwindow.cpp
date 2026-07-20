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

    // Game: 10 cols × 20 rows, blockSize=35
    // Width:  12(margin) + 350(10×35) + 12(spacing) + 100(side) + 12(margin) = 486
    // Height: 12(margin) + 700(20×35) + 12(margin) = 724
    resize(486, 724);
    setFixedSize(486, 724);
    setWindowTitle("Tetris");

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
