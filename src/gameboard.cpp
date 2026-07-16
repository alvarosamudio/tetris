#include "gameboard.h"
#include "tetriscolors.h"
#include <QKeyEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QResizeEvent>

GameBoard::GameBoard(QWidget *parent) : QWidget(parent), blockSize(35) {
  setFocusPolicy(Qt::StrongFocus);
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &GameBoard::gameStep);

  int rows = TetrisGame::Height - 2;
  int minW = TetrisGame::Width * 20;
  int minH = rows * 20;
  setMinimumSize(minW, minH);
}

void GameBoard::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  int rows = TetrisGame::Height - 2;
  int bw = width() / TetrisGame::Width;
  int bh = height() / rows;
  blockSize = qMin(bw, bh);
  if (blockSize < 10) blockSize = 10;
  update();
}

void GameBoard::startGame() {
  game.reset();
  timer->start(game.getTickInterval());
  emit scoreChanged(game.getScore());
  emit nextPieceChanged(game.getNextPiece());
  emit levelChanged(game.getLevel());
  emit linesChanged(game.getTotalLinesCleared());
  update();
}

void GameBoard::pauseGame() {
  game.setPaused(true);
  timer->stop();
  emit gamePaused();
  update();
}

void GameBoard::resumeGame() {
  game.setPaused(false);
  timer->start(game.getTickInterval());
  emit gameResumed();
  update();
}

void GameBoard::gameStep() {
  int oldScore = game.getScore();
  int oldLevel = game.getLevel();
  int oldLines = game.getTotalLinesCleared();
  TetrominoType oldNextType = game.getNextPiece().type;

  if (!game.step()) {
    timer->stop();
    emit gameOver(game.getScore());
  }

  if (game.getScore() != oldScore)
    emit scoreChanged(game.getScore());
  if (game.getLevel() != oldLevel)
    emit levelChanged(game.getLevel());
  if (game.getTotalLinesCleared() != oldLines) {
    emit linesChanged(game.getTotalLinesCleared());
    emit linesCleared(game.getTotalLinesCleared() - oldLines);
  }
  if (game.getNextPiece().type != oldNextType)
    emit nextPieceChanged(game.getNextPiece());

  // Update timer interval for current level
  if (timer->isActive())
    timer->setInterval(game.getTickInterval());

  update();
}

void GameBoard::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Background - Concept Dark theme
  painter.fillRect(rect(), QColor(25, 25, 25));

  // Watermark "deepin"
  painter.setOpacity(0.05);
  painter.setPen(Qt::white);
  QFont font = painter.font();
  font.setPointSize(45);
  font.setBold(true);
  painter.setFont(font);
  painter.drawText(rect(), Qt::AlignCenter, "deepin");
  painter.setOpacity(1.0);

  int rows = TetrisGame::Height - 2;

  // Grid lines
  painter.setPen(QPen(QColor(255, 255, 255, 15), 1));
  for (int x = 0; x <= TetrisGame::Width; ++x) {
    int lx = x * blockSize;
    painter.drawLine(lx, 0, lx, rows * blockSize);
  }
  for (int y = 0; y <= rows; ++y) {
    int ly = y * blockSize;
    painter.drawLine(0, ly, TetrisGame::Width * blockSize, ly);
  }

  const auto &grid = game.getGrid();
  for (int y = 2; y < TetrisGame::Height; ++y) {
    for (int x = 0; x < TetrisGame::Width; ++x) {
      if (grid[y][x] != TetrominoType::None) {
        drawBlock(painter, x, y - 2, grid[y][x]);
      }
    }
  }

  const auto &piece = game.getCurrentPiece();
  for (const QPoint &block : piece.blocks) {
    int x = piece.position.x() + block.x();
    int y = piece.position.y() + block.y();
    if (y >= 2) {
      drawBlock(painter, x, y - 2, piece.type);
    }
  }

  if (game.isGameOver()) {
    painter.fillRect(rect(), QColor(0, 0, 0, 180));
    painter.setPen(Qt::white);
    QFont ofont = painter.font();
    QFont f = ofont;
    f.setPointSize(24);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(rect(), Qt::AlignCenter, tr("GAME OVER"));
    painter.setFont(ofont);
  } else if (game.isPaused()) {
    painter.fillRect(rect(), QColor(0, 0, 0, 120));
    painter.setPen(Qt::white);
    QFont ofont = painter.font();
    QFont f = ofont;
    f.setPointSize(24);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(rect(), Qt::AlignCenter, tr("PAUSED"));
    painter.setFont(ofont);
  }
}

void GameBoard::drawBlock(QPainter &painter, int x, int y, TetrominoType type) {
  QColor color = getColorForType(type);
  QRectF blockRect(x * blockSize + 2, y * blockSize + 2, blockSize - 4,
                   blockSize - 4);

  QLinearGradient gradient(blockRect.topLeft(), blockRect.bottomLeft());
  gradient.setColorAt(0, color.lighter(130));
  gradient.setColorAt(1, color.darker(120));

  painter.setPen(QPen(color.darker(140), 1));
  painter.setBrush(gradient);

  // Square with rounded corners
  painter.drawRoundedRect(blockRect, 5, 5);

  // Top-left highlight
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(255, 255, 255, 100));
  QRectF highlightRect(blockRect.left() + 3,
                       blockRect.top() + 2,
                       blockRect.width() * 0.4, blockRect.height() * 0.25);
  painter.drawRoundedRect(highlightRect, 2, 2);
}

void GameBoard::keyPressEvent(QKeyEvent *event) {
  int oldScore = game.getScore();
  int oldLevel = game.getLevel();
  int oldLines = game.getTotalLinesCleared();
  TetrominoType oldNext = game.getNextPiece().type;

  switch (event->key()) {
  case Qt::Key_Left:
    game.moveLeft();
    break;
  case Qt::Key_Right:
    game.moveRight();
    break;
  case Qt::Key_Up: {
    int oldLines2 = game.getTotalLinesCleared();
    game.rotate();
    if (game.getTotalLinesCleared() == oldLines2 &&
        game.getScore() == oldScore)
      emit pieceRotated();
    break;
  }
  case Qt::Key_Down:
    game.softDrop();
    break;
  case Qt::Key_Space:
    game.hardDrop();
    emit pieceDropped();
    break;
  case Qt::Key_P:
    if (game.isPaused())
      resumeGame();
    else
      pauseGame();
    break;
  default:
    QWidget::keyPressEvent(event);
    return;
  }

  if (game.getScore() != oldScore)
    emit scoreChanged(game.getScore());
  if (game.getLevel() != oldLevel)
    emit levelChanged(game.getLevel());
  if (game.getTotalLinesCleared() != oldLines) {
    emit linesChanged(game.getTotalLinesCleared());
    emit linesCleared(game.getTotalLinesCleared() - oldLines);
  }
  if (game.getNextPiece().type != oldNext)
    emit nextPieceChanged(game.getNextPiece());
  if (game.isGameOver())
    emit gameOver(game.getScore());

  update();
}
