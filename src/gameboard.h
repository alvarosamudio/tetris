#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include "dropixgame.h"
#include <QKeyEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>
#include <QWidget>

class GameBoard : public QWidget {
  Q_OBJECT
public:
  explicit GameBoard(QWidget *parent = nullptr);
  void startGame();
  void pauseGame();
  void resumeGame();
  void setGhostPiece(bool enabled);
  void setDifficulty(int diff);

  const DropixGame &getGame() const { return game; }

signals:
  void scoreChanged(int score);
  void levelChanged(int level);
  void linesChanged(int lines);
  void linesCleared(int count);
  void gamePaused();
  void gameResumed();
  void pieceDropped();
  void pieceRotated();
  void nextPieceChanged(const Tetromino &piece);
  void gameOver(int score);

protected:
  void paintEvent(QPaintEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void gameStep();

private:
  void drawBlock(QPainter &painter, int x, int y, TetrominoType type);
  void drawGhostBlock(QPainter &painter, int x, int y, TetrominoType type);

  DropixGame game;
  QTimer *timer;
  int blockSize;
  int m_lineFlashRow;
  int m_flashTimer;
  bool m_showGhost;
  int m_difficulty;
};

#endif // GAMEBOARD_H
