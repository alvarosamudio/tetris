#include "widget.h"
#include "ui_widget.h"
#include <QString>
#include <QVBoxLayout>
#include <QLayoutItem>

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget), m_muted(false), m_highScore(0) {
  ui->setupUi(this);

  // Set dark theme style for the whole window
  setStyleSheet(
    "QWidget { background-color: #1e1e1e; color: white; }"
    "QLabel { background-color: transparent; color: white; }"
    "QPushButton { background-color: #333333; color: white; border: 1px solid #555555; border-radius: 4px; padding: 5px; }"
    "QPushButton:hover { background-color: #444444; }"
    "#nextContainer { background-color: #232323; border-radius: 10px; }"
    "QPushButton { min-width: 90px; max-width: 90px; }"
  );

  m_gameBoard = new GameBoard(this);
  m_nextPieceWidget = new NextPieceWidget(this);
  m_soundManager = new SoundManager(this);

  // Main Game Container
  if (!ui->gameContainer->layout()) {
    ui->gameContainer->setLayout(new QVBoxLayout());
  }
  ui->gameContainer->layout()->addWidget(m_gameBoard);
  ui->gameContainer->layout()->setContentsMargins(0, 0, 0, 0);

  // Place NextPieceWidget
  if (!ui->nextContainer->layout()) {
    ui->nextContainer->setLayout(new QVBoxLayout());
  }
  ui->nextContainer->layout()->addWidget(m_nextPieceWidget);
  ui->nextContainer->layout()->setContentsMargins(5, 5, 5, 5);
  ui->nextContainer->layout()->setAlignment(Qt::AlignCenter);

  // Center all items in the side panel
  for (int i = 0; i < ui->sideLayout->count(); ++i) {
    if (auto *item = ui->sideLayout->itemAt(i)) {
      item->setAlignment(Qt::AlignHCenter);
    }
  }

  connect(m_gameBoard, &GameBoard::scoreChanged, this, &Widget::updateScore);
  connect(m_gameBoard, &GameBoard::levelChanged, this, &Widget::updateLevel);
  connect(m_gameBoard, &GameBoard::linesChanged, this, &Widget::updateLines);
  connect(m_gameBoard, &GameBoard::nextPieceChanged, this,
          &Widget::updateNextPiece);
  connect(m_gameBoard, &GameBoard::gameOver, this, &Widget::onGameOver);

  // Sound effects
  connect(m_gameBoard, &GameBoard::pieceRotated, m_soundManager, &SoundManager::playRotate);
  connect(m_gameBoard, &GameBoard::pieceDropped, m_soundManager, &SoundManager::playDrop);
  connect(m_gameBoard, &GameBoard::linesCleared, m_soundManager, &SoundManager::playLineClear);
  connect(m_gameBoard, &GameBoard::gameOver, m_soundManager, [this](int) { m_soundManager->playGameOver(); });
  connect(m_gameBoard, &GameBoard::gamePaused, this, &Widget::onGamePaused);
  connect(m_gameBoard, &GameBoard::gameResumed, this, &Widget::onGameResumed);

  connect(ui->startBtn, &QPushButton::clicked, this, &Widget::onStartClicked);
  connect(ui->pauseBtn, &QPushButton::clicked, this, &Widget::onPauseClicked);
  connect(ui->muteBtn, &QPushButton::clicked, this, &Widget::onMuteClicked);

  // Pause button hidden until game starts
  ui->pauseBtn->hide();

  // Custom styling for labels according to concept
  ui->scoreLabel->setText(tr("SCORE") + "\n000000");
  ui->scoreLabel->setStyleSheet(
      "font-size: 16pt; font-family: 'Monospace'; font-weight: bold; letter-spacing: 2px; color: #ffffff;");

  ui->highScoreLabel->setStyleSheet(
      "font-size: 11pt; font-family: 'Monospace'; font-weight: bold; color: #ffcc00;");

  ui->levelLabel->setText(tr("LEVEL") + "\n01");
  ui->levelLabel->setStyleSheet(
      "font-size: 11pt; font-family: 'Monospace'; font-weight: bold; color: #00ccff;");

  ui->linesLabel->setText(tr("LINES") + "\n000");
  ui->linesLabel->setStyleSheet(
      "font-size: 11pt; font-family: 'Monospace'; font-weight: bold; color: #00ff88;");

  loadHighScore();
}

Widget::~Widget() { delete ui; }

void Widget::loadHighScore() {
  QSettings settings("deepin-es", "Tetris");
  m_highScore = settings.value("highScore", 0).toInt();
  QString hsStr = QString("%1").arg(m_highScore, 6, 10, QChar('0'));
  ui->highScoreLabel->setText(tr("HI-SCORE") + "\n" + hsStr);
}

void Widget::saveHighScore(int score) {
  QSettings settings("deepin-es", "Tetris");
  settings.setValue("highScore", score);
  settings.sync();
}

void Widget::onGameOver(int score) {
  if (score > m_highScore) {
    m_highScore = score;
    saveHighScore(score);
    QString hsStr = QString("%1").arg(m_highScore, 6, 10, QChar('0'));
    ui->highScoreLabel->setText(tr("HI-SCORE") + "\n" + hsStr);
  }
}

void Widget::onStartClicked() {
  m_gameBoard->startGame();
  m_gameBoard->setFocus();
  ui->startBtn->setText(tr("RESTART"));
  ui->pauseBtn->show();
  ui->pauseBtn->setText(tr("Pause"));
  if (!m_muted)
    m_soundManager->startMusic();
}

void Widget::updateScore(int score) {
  QString scoreStr = QString("%1").arg(score, 6, 10, QChar('0'));
  ui->scoreLabel->setText(tr("SCORE") + "\n" + scoreStr);
}

void Widget::updateLevel(int level) {
  QString levelStr = QString("%1").arg(level, 2, 10, QChar('0'));
  ui->levelLabel->setText(tr("LEVEL") + "\n" + levelStr);
}

void Widget::updateLines(int lines) {
  QString linesStr = QString("%1").arg(lines, 3, 10, QChar('0'));
  ui->linesLabel->setText(tr("LINES") + "\n" + linesStr);
}

void Widget::updateNextPiece(const Tetromino &piece) {
  m_nextPieceWidget->setNextPiece(piece);
}

void Widget::onPauseClicked() {
  if (m_gameBoard->getGame().isPaused()) {
    m_gameBoard->resumeGame();
  } else {
    m_gameBoard->pauseGame();
  }
}

void Widget::onGamePaused() {
  ui->pauseBtn->setText(tr("Resume"));
  m_soundManager->pauseMusic();
}

void Widget::onGameResumed() {
  ui->pauseBtn->setText(tr("Pause"));
  m_soundManager->resumeMusic();
}

void Widget::onMuteClicked() {
  m_muted = !m_muted;
  ui->muteBtn->setText(m_muted ? tr("Unmute") : tr("Mute"));
  m_soundManager->setMuted(m_muted);
  emit musicToggled(m_muted);
}
