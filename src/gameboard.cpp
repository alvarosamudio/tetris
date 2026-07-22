#include "gameboard.h"
#include "tetriscolors.h"
#include <QKeyEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainter>
#include <QResizeEvent>

GameBoard::GameBoard(QWidget *parent)
    : QWidget(parent), blockSize(35), m_lineFlashRow(-1), m_flashTimer(0),
      m_showGhost(true), m_difficulty(1) {
  setFocusPolicy(Qt::StrongFocus);
  setAutoFillBackground(true);
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
  m_lineFlashRow = -1;
  m_flashTimer = 0;
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

void GameBoard::setGhostPiece(bool enabled) { m_showGhost = enabled; update(); }

void GameBoard::setDifficulty(int diff) {
  m_difficulty = diff;
  if (timer->isActive()) {
    timer->setInterval(game.getTickInterval());
  }
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

  if (timer->isActive())
    timer->setInterval(game.getTickInterval());

  update();
}

void GameBoard::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QPalette pal = palette();
  QColor bgColor = pal.color(QPalette::Window);
  QColor textColor = pal.color(QPalette::Text);
  QColor midColor = pal.color(QPalette::Mid);
  QColor shadowColor = pal.color(QPalette::Shadow);
  QColor baseColor = pal.color(QPalette::Base);

  int rows = TetrisGame::Height - 2;
  int boardW = TetrisGame::Width * blockSize;
  int boardH = rows * blockSize;

  // Background
  painter.fillRect(rect(), baseColor);

  // Subtle watermark
  painter.setOpacity(0.05);
  painter.setPen(textColor);
  QFont wmfont = painter.font();
  wmfont.setPointSize(40);
  wmfont.setBold(true);
  painter.setFont(wmfont);
  painter.drawText(QRectF(0, 0, boardW, boardH), Qt::AlignCenter, "TETRIS");
  painter.setOpacity(1.0);

  // Grid lines
  painter.setPen(QPen(midColor, 1));
  for (int x = 0; x <= TetrisGame::Width; ++x) {
    int lx = x * blockSize;
    painter.drawLine(lx, 0, lx, boardH);
  }
  for (int y = 0; y <= rows; ++y) {
    int ly = y * blockSize;
    painter.drawLine(0, ly, boardW, ly);
  }

  // Draw locked blocks
  const auto &grid = game.getGrid();
  for (int y = 2; y < TetrisGame::Height; ++y) {
    for (int x = 0; x < TetrisGame::Width; ++x) {
      if (grid[y][x] != TetrominoType::None) {
        drawBlock(painter, x, y - 2, grid[y][x]);
      }
    }
  }

  // Draw ghost piece
  if (m_showGhost && !game.isGameOver() && !game.isPaused()) {
    QPoint ghostPos = game.getGhostPosition();
    const auto &piece = game.getCurrentPiece();
    if (ghostPos != piece.position) {
      for (const QPoint &block : piece.blocks) {
        int gx = ghostPos.x() + block.x();
        int gy = ghostPos.y() + block.y();
        if (gy >= 2) {
          drawGhostBlock(painter, gx, gy - 2, piece.type);
        }
      }
    }
  }

  // Draw current piece
  const auto &piece = game.getCurrentPiece();
  for (const QPoint &block : piece.blocks) {
    int x = piece.position.x() + block.x();
    int y = piece.position.y() + block.y();
    if (y >= 2) {
      drawBlock(painter, x, y - 2, piece.type);
    }
  }

  // Game over overlay
  if (game.isGameOver()) {
    painter.fillRect(QRectF(0, 0, boardW, boardH),
                     QColor(shadowColor.red(), shadowColor.green(),
                            shadowColor.blue(), 200));

    QFont titleFont = painter.font();
    titleFont.setPointSize(26);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(pal.color(QPalette::Highlight));
    painter.drawText(QRectF(0, boardH * 0.25, boardW, 50), Qt::AlignCenter,
                     tr("GAME OVER"));

    QFont scoreFont = painter.font();
    scoreFont.setPointSize(13);
    scoreFont.setBold(false);
    painter.setFont(scoreFont);
    painter.setPen(textColor);
    painter.drawText(QRectF(0, boardH * 0.40, boardW, 30), Qt::AlignCenter,
                     tr("SCORE: %1").arg(game.getScore()));

    QFont hintFont = painter.font();
    hintFont.setPointSize(10);
    painter.setFont(hintFont);
    painter.setPen(QColor(textColor.red(), textColor.green(), textColor.blue(), 120));
    painter.drawText(QRectF(0, boardH * 0.55, boardW, 30), Qt::AlignCenter,
                     tr("Press START to play again"));
  } else if (game.isPaused()) {
    painter.fillRect(QRectF(0, 0, boardW, boardH),
                     QColor(shadowColor.red(), shadowColor.green(),
                            shadowColor.blue(), 160));

    QFont pfont = painter.font();
    pfont.setPointSize(28);
    pfont.setBold(true);
    painter.setFont(pfont);
    painter.setPen(textColor);
    painter.drawText(QRectF(0, boardH * 0.40, boardW, 50), Qt::AlignCenter,
                     tr("PAUSED"));

    QFont hintFont = painter.font();
    hintFont.setPointSize(10);
    hintFont.setBold(false);
    painter.setFont(hintFont);
    painter.setPen(QColor(textColor.red(), textColor.green(), textColor.blue(), 120));
    painter.drawText(QRectF(0, boardH * 0.52, boardW, 30), Qt::AlignCenter,
                     tr("Press P to resume"));
  }
}

void GameBoard::drawBlock(QPainter &painter, int x, int y, TetrominoType type) {
  QColor color = getColorForType(type);
  QRectF blockRect(x * blockSize + 1.5, y * blockSize + 1.5, blockSize - 3,
                   blockSize - 3);

  // Glow shadow
  QRadialGradient glow(blockRect.center(), blockRect.width() * 0.7);
  glow.setColorAt(0, QColor(color.red(), color.green(), color.blue(), 50));
  glow.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
  painter.setPen(Qt::NoPen);
  painter.setBrush(glow);
  painter.drawEllipse(QRectF(blockRect.x() - 2, blockRect.y() - 2,
                              blockRect.width() + 4, blockRect.height() + 4));

  // Main block gradient
  QLinearGradient gradient(blockRect.topLeft(), blockRect.bottomRight());
  gradient.setColorAt(0, color.lighter(140));
  gradient.setColorAt(0.5, color);
  gradient.setColorAt(1, color.darker(130));

  painter.setPen(QPen(color.darker(150), 1));
  painter.setBrush(gradient);
  painter.drawRoundedRect(blockRect, 4, 4);

  // Inner shine
  painter.setPen(Qt::NoPen);
  painter.setBrush(QColor(255, 255, 255, 90));
  QRectF highlight(blockRect.left() + 3, blockRect.top() + 2,
                   blockRect.width() * 0.35, blockRect.height() * 0.22);
  painter.drawRoundedRect(highlight, 2, 2);

  // Bottom-right subtle shadow
  painter.setBrush(QColor(0, 0, 0, 40));
  QRectF shadow(blockRect.right() - blockRect.width() * 0.3,
                blockRect.bottom() - blockRect.height() * 0.18,
                blockRect.width() * 0.25, blockRect.height() * 0.14);
  painter.drawRoundedRect(shadow, 1, 1);
}

void GameBoard::drawGhostBlock(QPainter &painter, int x, int y,
                                TetrominoType type) {
  QColor color = getColorForType(type);
  QRectF blockRect(x * blockSize + 2, y * blockSize + 2, blockSize - 4,
                   blockSize - 4);

  painter.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 60), 1.5,
                       Qt::DashLine));
  painter.setBrush(QColor(color.red(), color.green(), color.blue(), 20));
  painter.drawRoundedRect(blockRect, 4, 4);
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
