#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <QtCore/QPoint>
#include <QtCore/QVector>

enum class TetrominoType { None, I, J, L, O, S, T, Z };

struct Tetromino {
  TetrominoType type;
  QVector<QPoint> blocks;
  QPoint position;
  int rotation;

  static Tetromino create(TetrominoType type);
  void rotateClockwise();
};

class BagRandomizer {
public:
  BagRandomizer();
  TetrominoType next();
  void reset();

private:
  QVector<TetrominoType> bag;
  void refill();
};

class TetrisGame {
public:
  static const int Width = 10;
  static const int Height = 22;

  TetrisGame();

  void reset();
  bool step();
  void moveLeft();
  void moveRight();
  void rotate();
  void softDrop();
  void hardDrop();

  const QVector<QVector<TetrominoType>> &getGrid() const { return grid; }
  const Tetromino &getCurrentPiece() const { return currentPiece; }
  const Tetromino &getNextPiece() const { return nextPiece; }
  int getScore() const { return score; }
  int getLevel() const { return level; }
  int getTotalLinesCleared() const { return totalLinesCleared; }
  bool isGameOver() const { return gameState == GameState::GameOver; }
  bool isPaused() const { return gameState == GameState::Paused; }
  void setPaused(bool paused);
  int getTickInterval() const;
  QPoint getGhostPosition() const;
  void setDifficulty(int diff);

private:
  enum class GameState { Running, Paused, GameOver };

  void spawnPiece();
  bool isValidPosition(const Tetromino &piece, QPoint pos) const;
  void lockPiece();
  void clearLines();

  QVector<QVector<TetrominoType>> grid;
  Tetromino currentPiece;
  Tetromino nextPiece;
  GameState gameState;
  int score;
  int level;
  int totalLinesCleared;
  int difficulty;
  BagRandomizer bag;
};

#endif // TETRISGAME_H
