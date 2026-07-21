#include "tetrisgame.h"
#include <QRandomGenerator>

Tetromino Tetromino::create(TetrominoType type) {
  Tetromino t;
  t.type = type;
  t.rotation = 0;
  t.position = QPoint(TetrisGame::Width / 2 - 1, 0);

  switch (type) {
  case TetrominoType::I:
    t.blocks = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};
    break;
  case TetrominoType::J:
    t.blocks = {{0, 0}, {0, 1}, {1, 1}, {2, 1}};
    break;
  case TetrominoType::L:
    t.blocks = {{2, 0}, {0, 1}, {1, 1}, {2, 1}};
    break;
  case TetrominoType::O:
    t.blocks = {{1, 0}, {2, 0}, {1, 1}, {2, 1}};
    break;
  case TetrominoType::S:
    t.blocks = {{1, 0}, {2, 0}, {0, 1}, {1, 1}};
    break;
  case TetrominoType::T:
    t.blocks = {{1, 0}, {0, 1}, {1, 1}, {2, 1}};
    break;
  case TetrominoType::Z:
    t.blocks = {{0, 0}, {1, 0}, {1, 1}, {2, 1}};
    break;
  default:
    break;
  }
  return t;
}

void Tetromino::rotateClockwise() {
  if (type == TetrominoType::O)
    return;
  for (int i = 0; i < blocks.size(); ++i) {
    int x = blocks[i].x();
    int y = blocks[i].y();
    blocks[i].setX(2 - y);
    blocks[i].setY(x);
  }
}

static const TetrominoType kAllTypes[] = {
    TetrominoType::I, TetrominoType::J, TetrominoType::L, TetrominoType::O,
    TetrominoType::S, TetrominoType::T, TetrominoType::Z};
static constexpr int kNumTypes = 7;

BagRandomizer::BagRandomizer() { refill(); }

void BagRandomizer::refill() {
  bag.clear();
  for (int i = 0; i < kNumTypes; ++i)
    bag.append(kAllTypes[i]);
  // Fisher-Yates shuffle
  for (int i = bag.size() - 1; i > 0; --i) {
    int j = QRandomGenerator::global()->bounded(i + 1);
    qSwap(bag[i], bag[j]);
  }
}

TetrominoType BagRandomizer::next() {
  if (bag.isEmpty())
    refill();
  return bag.takeLast();
}

void BagRandomizer::reset() {
  bag.clear();
  refill();
}

TetrisGame::TetrisGame() { difficulty = 1; reset(); }

void TetrisGame::reset() {
  grid.clear();
  grid.resize(Height);
  for (int i = 0; i < Height; ++i)
    grid[i].fill(TetrominoType::None, Width);
  score = 0;
  level = 1;
  totalLinesCleared = 0;
  gameState = GameState::Running;

  bag.reset();
  nextPiece = Tetromino::create(bag.next());
  spawnPiece();
}

void TetrisGame::spawnPiece() {
  currentPiece = nextPiece;
  nextPiece = Tetromino::create(bag.next());

  if (!isValidPosition(currentPiece, currentPiece.position)) {
    gameState = GameState::GameOver;
  }
}

bool TetrisGame::step() {
  if (gameState != GameState::Running)
    return false;

  QPoint nextPos = currentPiece.position + QPoint(0, 1);
  if (isValidPosition(currentPiece, nextPos)) {
    currentPiece.position = nextPos;
  } else {
    lockPiece();
    clearLines();
    spawnPiece();
  }
  return gameState != GameState::GameOver;
}

void TetrisGame::moveLeft() {
  if (gameState != GameState::Running)
    return;
  QPoint nextPos = currentPiece.position + QPoint(-1, 0);
  if (isValidPosition(currentPiece, nextPos))
    currentPiece.position = nextPos;
}

void TetrisGame::moveRight() {
  if (gameState != GameState::Running)
    return;
  QPoint nextPos = currentPiece.position + QPoint(1, 0);
  if (isValidPosition(currentPiece, nextPos))
    currentPiece.position = nextPos;
}

void TetrisGame::rotate() {
  if (gameState != GameState::Running)
    return;
  Tetromino rotated = currentPiece;
  rotated.rotateClockwise();
  if (isValidPosition(rotated, rotated.position))
    currentPiece = rotated;
}

void TetrisGame::softDrop() { step(); }

void TetrisGame::hardDrop() {
  if (gameState != GameState::Running)
    return;
  while (isValidPosition(currentPiece, currentPiece.position + QPoint(0, 1))) {
    currentPiece.position += QPoint(0, 1);
    score += 2;
  }
  step();
}

bool TetrisGame::isValidPosition(const Tetromino &piece, QPoint pos) const {
  for (const QPoint &block : piece.blocks) {
    int x = pos.x() + block.x();
    int y = pos.y() + block.y();
    if (x < 0 || x >= Width || y >= Height)
      return false;
    if (y >= 0 && grid[y][x] != TetrominoType::None)
      return false;
  }
  return true;
}

void TetrisGame::lockPiece() {
  for (const QPoint &block : currentPiece.blocks) {
    int x = currentPiece.position.x() + block.x();
    int y = currentPiece.position.y() + block.y();
    if (y >= 0 && y < Height && x >= 0 && x < Width) {
      grid[y][x] = currentPiece.type;
    }
  }
}

void TetrisGame::clearLines() {
  int linesCleared = 0;
  for (int y = Height - 1; y >= 0; --y) {
    bool full = true;
    for (int x = 0; x < Width; ++x) {
      if (grid[y][x] == TetrominoType::None) {
        full = false;
        break;
      }
    }
    if (full) {
      grid.remove(y);
      grid.prepend(QVector<TetrominoType>(Width, TetrominoType::None));
      y++;
      linesCleared++;
    }
  }
  if (linesCleared > 0) {
    totalLinesCleared += linesCleared;
    level = 1 + totalLinesCleared / 10;
    switch (linesCleared) {
    case 1: score += 100 * level; break;
    case 2: score += 300 * level; break;
    case 3: score += 500 * level; break;
    case 4: score += 800 * level; break;
    default: break;
    }
  }
}

int TetrisGame::getTickInterval() const {
  // Base interval: 800ms, decreases by ~60ms per level, min 80ms
  // Difficulty multiplier: 0=Easy(1.5x), 1=Normal(1.0x), 2=Hard(0.7x), 3=Expert(0.5x)
  static const double diffMultipliers[] = {1.5, 1.0, 0.7, 0.5};
  double mult = diffMultipliers[qBound(0, difficulty, 3)];
  int interval = static_cast<int>((800 - (level - 1) * 60) * mult);
  return qMax(interval, 40);
}

void TetrisGame::setDifficulty(int diff) {
  difficulty = qBound(0, diff, 3);
}

QPoint TetrisGame::getGhostPosition() const {
  if (gameState != GameState::Running)
    return currentPiece.position;
  QPoint ghostPos = currentPiece.position;
  while (isValidPosition(currentPiece, ghostPos + QPoint(0, 1))) {
    ghostPos += QPoint(0, 1);
  }
  return ghostPos;
}

void TetrisGame::setPaused(bool paused) {
  if (gameState == GameState::GameOver)
    return;
  gameState = paused ? GameState::Paused : GameState::Running;
}
