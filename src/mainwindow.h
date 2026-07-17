#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef Q_OS_LINUX
#include <DMainWindow>
DWIDGET_USE_NAMESPACE
#define MAINWINDOW_BASE DMainWindow
#else
#include <QMainWindow>
#define MAINWINDOW_BASE QMainWindow
#endif

#include "widget.h"

class MainWindow : public MAINWINDOW_BASE
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void aboutRequested(bool);

private:
    Widget *w;
};

#endif // MAINWINDOW_H
