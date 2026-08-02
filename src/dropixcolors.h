#ifndef DROPIXCOLORS_H
#define DROPIXCOLORS_H

#include "dropixgame.h"
#include <QColor>

inline QColor getColorForType(TetrominoType type) {
  switch (type) {
  case TetrominoType::I: return QColor(255, 170, 40);   // amber
  case TetrominoType::J: return QColor(70, 200, 140);   // mint
  case TetrominoType::L: return QColor(110, 130, 255);  // periwinkle
  case TetrominoType::O: return QColor(250, 90, 90);    // coral
  case TetrominoType::S: return QColor(200, 90, 240);   // orchid
  case TetrominoType::T: return QColor(255, 210, 70);   // golden
  case TetrominoType::Z: return QColor(90, 200, 255);   // sky
  default: return Qt::black;
  }
}

#endif // DROPIXCOLORS_H
