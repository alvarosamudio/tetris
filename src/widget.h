#ifndef WIDGET_H
#define WIDGET_H

#include "gameboard.h"
#include "nextpiecewidget.h"
#include "soundmanager.h"
#include "settingsdialog.h"
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
  void openSettings();

signals:
  void musicToggled(bool muted);

private slots:
  void onStartClicked();
  void onPauseClicked();
  void onSettingsClicked();
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
  void loadSettings();
  void saveSettings();

  Ui::Widget *ui;
  GameBoard *m_gameBoard;
  NextPieceWidget *m_nextPieceWidget;
  SoundManager *m_soundManager;
  SettingsDialog *m_settingsDialog;
  bool m_muted;
  int m_highScore;
  bool m_ghostPiece;
  int m_volume;
  int m_difficulty;
  int m_musicTheme;
  int m_musicGenre;
};

#endif // WIDGET_H