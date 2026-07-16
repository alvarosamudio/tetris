#ifndef WIDGET_H
#define WIDGET_H

#include "gameboard.h"
#include "nextpiecewidget.h"
#include "soundmanager.h"
#include <QSettings>
#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget {
  Q_OBJECT

public:
  explicit Widget(QWidget *parent = 0);
  ~Widget();
  SoundManager *soundManager() const { return m_soundManager; }

signals:
  void musicToggled(bool muted);

private slots:
  void onStartClicked();
  void onPauseClicked();
  void onMuteClicked();
  void updateScore(int score);
  void updateLevel(int level);
  void updateLines(int lines);
  void updateNextPiece(const Tetromino &piece);
  void onGameOver(int score);
  void onGamePaused();
  void onGameResumed();

private:
  void loadHighScore();
  void saveHighScore(int score);

  Ui::Widget *ui;
  GameBoard *m_gameBoard;
  NextPieceWidget *m_nextPieceWidget;
  SoundManager *m_soundManager;
  bool m_muted;
  int m_highScore;
};

#endif // WIDGET_H
