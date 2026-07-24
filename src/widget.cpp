#include "widget.h"
#include "ui_widget.h"
#include <QString>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QFont>

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget), m_muted(false), m_highScore(0),
      m_ghostPiece(true), m_volume(70), m_difficulty(1) {
  ui->setupUi(this);

  // Font styling
  QFont titleFont;
  titleFont.setPointSize(18);
  titleFont.setBold(true);
  titleFont.setFamily("Monospace");
  ui->titleLabel->setFont(titleFont);

  QFont labelFont;
  labelFont.setPointSize(10);
  labelFont.setBold(true);
  labelFont.setFamily("Monospace");

  QFont scoreFont;
  scoreFont.setPointSize(14);
  scoreFont.setBold(true);
  scoreFont.setFamily("Monospace");

  ui->nextLabel->setFont(labelFont);
  ui->scoreLabel->setFont(scoreFont);
  ui->highScoreLabel->setFont(labelFont);
  ui->levelLabel->setFont(labelFont);
  ui->linesLabel->setFont(labelFont);

  m_gameBoard = new GameBoard(this);
  m_nextPieceWidget = new NextPieceWidget(this);
  m_soundManager = new SoundManager(this);
  m_settingsDialog = new SettingsDialog(this);

  // Main Game Container
  QVBoxLayout *gameLayout = new QVBoxLayout();
  gameLayout->setContentsMargins(0, 0, 0, 0);
  gameLayout->addWidget(m_gameBoard, 1);
  ui->gameContainer->setLayout(gameLayout);

  // Place NextPieceWidget
  QVBoxLayout *nextLayout = new QVBoxLayout();
  nextLayout->setContentsMargins(4, 4, 4, 4);
  nextLayout->setAlignment(Qt::AlignCenter);
  nextLayout->addWidget(m_nextPieceWidget, 1);
  ui->nextContainer->setLayout(nextLayout);

  // Center all items in the side panel
  for (int i = 0; i < ui->sideLayout->count(); ++i) {
    if (auto *item = ui->sideLayout->itemAt(i)) {
      item->setAlignment(Qt::AlignHCenter);
    }
  }

  // Game signals
  connect(m_gameBoard, &GameBoard::scoreChanged, this, &Widget::updateScore);
  connect(m_gameBoard, &GameBoard::levelChanged, this, &Widget::updateLevel);
  connect(m_gameBoard, &GameBoard::linesChanged, this, &Widget::updateLines);
  connect(m_gameBoard, &GameBoard::nextPieceChanged, this,
          &Widget::updateNextPiece);
  connect(m_gameBoard, &GameBoard::gameOver, this, &Widget::onGameOver);

  // Sound signals
  connect(m_gameBoard, &GameBoard::pieceRotated, m_soundManager,
          &SoundManager::playRotate);
  connect(m_gameBoard, &GameBoard::pieceDropped, m_soundManager,
          &SoundManager::playDrop);
  connect(m_gameBoard, &GameBoard::linesCleared, m_soundManager,
          &SoundManager::playLineClear);
  connect(m_gameBoard, &GameBoard::gameOver, m_soundManager,
          [this](int) { m_soundManager->playGameOver(); });
  connect(m_gameBoard, &GameBoard::gamePaused, this, &Widget::onGamePaused);
  connect(m_gameBoard, &GameBoard::gameResumed, this, &Widget::onGameResumed);

  // Button signals
  connect(ui->startBtn, &QPushButton::clicked, this, &Widget::onStartClicked);
  connect(ui->pauseBtn, &QPushButton::clicked, this, &Widget::onPauseClicked);

  // Settings dialog signals
  connect(m_settingsDialog, &SettingsDialog::ghostPieceToggled, this,
          [this](bool enabled) {
            m_ghostPiece = enabled;
            m_gameBoard->setGhostPiece(enabled);
            saveSettings();
          });
  connect(m_settingsDialog, &SettingsDialog::musicToggled, this,
          [this](bool enabled) {
            m_muted = !enabled;
            m_soundManager->setMuted(m_muted);
            saveSettings();
          });
  connect(m_settingsDialog, &SettingsDialog::volumeChanged, this,
          [this](int vol) {
            m_volume = vol;
            m_soundManager->setVolume(vol / 100.0f);
            saveSettings();
          });
  connect(m_settingsDialog, &SettingsDialog::difficultyChanged, this,
          [this](int diff) {
            m_difficulty = diff;
            m_gameBoard->setDifficulty(diff);
            saveSettings();
          });

  // Pause button hidden until game starts
  ui->pauseBtn->hide();

  // Load persisted settings
  loadSettings();

  // Apply initial settings to widgets
  m_gameBoard->setGhostPiece(m_ghostPiece);
  m_gameBoard->setDifficulty(m_difficulty);
  m_soundManager->setVolume(m_volume / 100.0f);

  // Sync settings dialog and combo box
  m_settingsDialog->setGhostPiece(m_ghostPiece);
  m_settingsDialog->setMusic(!m_muted);
  m_settingsDialog->setVolume(m_volume);
  m_settingsDialog->setDifficulty(m_difficulty);

  // Initial label text
  ui->scoreLabel->setText(tr("SCORE") + "\n000000");
  ui->levelLabel->setText(tr("LEVEL") + "\n01");
  ui->linesLabel->setText(tr("LINES") + "\n000");

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

void Widget::loadSettings() {
  QSettings settings("deepin-es", "Tetris");
  m_ghostPiece = settings.value("ghostPiece", true).toBool();
  m_muted = settings.value("muted", false).toBool();
  m_volume = settings.value("volume", 70).toInt();
  m_difficulty = settings.value("difficulty", 1).toInt();
}

void Widget::saveSettings() {
  QSettings settings("deepin-es", "Tetris");
  settings.setValue("ghostPiece", m_ghostPiece);
  settings.setValue("muted", m_muted);
  settings.setValue("volume", m_volume);
  settings.setValue("difficulty", m_difficulty);
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

void Widget::onSettingsClicked() {
  openSettings();
}

void Widget::openSettings() {
  m_settingsDialog->setGhostPiece(m_ghostPiece);
  m_settingsDialog->setMusic(!m_muted);
  m_settingsDialog->setVolume(m_volume);
  m_settingsDialog->setDifficulty(m_difficulty);

  if (m_settingsDialog->exec() == QDialog::Accepted) {
    saveSettings();
  }
}
