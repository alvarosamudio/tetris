#include "nextpiecewidget.h"
#include "tetriscolors.h"
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainter>

NextPieceWidget::NextPieceWidget(QWidget *parent) : QWidget(parent) {
  setMinimumSize(90, 90);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  setAutoFillBackground(true);
}

void NextPieceWidget::setNextPiece(const Tetromino &piece) {
  m_piece = piece;
  update();
}

void NextPieceWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QPalette pal = palette();
  QColor baseColor = pal.color(QPalette::Base);
  QColor midColor = pal.color(QPalette::Mid);

  // Background
  painter.setBrush(baseColor);
  painter.setPen(QPen(midColor, 1));
  painter.drawRoundedRect(rect(), 10, 10);

  if (m_piece.type == TetrominoType::None)
    return;

  int blockSize = qMin(width() / 4, height() / 3);
  if (blockSize < 6) blockSize = 6;
  int offsetX = (width() - 4 * blockSize) / 2;
  int offsetY = (height() - 2 * blockSize) / 2;

  QColor color = getColorForType(m_piece.type);
  for (const QPoint &block : m_piece.blocks) {
    QRectF blockRect(offsetX + block.x() * blockSize + 1,
                     offsetY + block.y() * blockSize + 1, blockSize - 2,
                     blockSize - 2);

    // Glow
    QRadialGradient glow(blockRect.center(), blockRect.width() * 0.6);
    glow.setColorAt(0, QColor(color.red(), color.green(), color.blue(), 40));
    glow.setColorAt(1, QColor(color.red(), color.green(), color.blue(), 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(glow);
    painter.drawEllipse(QRectF(blockRect.x() - 1, blockRect.y() - 1,
                                blockRect.width() + 2, blockRect.height() + 2));

    // Block gradient
    QLinearGradient gradient(blockRect.topLeft(), blockRect.bottomRight());
    gradient.setColorAt(0, color.lighter(140));
    gradient.setColorAt(0.5, color);
    gradient.setColorAt(1, color.darker(130));

    painter.setPen(QPen(color.darker(150), 1));
    painter.setBrush(gradient);
    painter.drawRoundedRect(blockRect, 4, 4);

    // Highlight
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 90));
    QRectF highlight(blockRect.left() + 2, blockRect.top() + 1,
                     blockRect.width() * 0.35, blockRect.height() * 0.22);
    painter.drawRoundedRect(highlight, 2, 2);
  }
}
