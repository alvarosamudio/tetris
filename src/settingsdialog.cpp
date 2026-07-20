#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : DDialog(parent) {
  setTitle(tr("Settings"));
  setFixedSize(380, 420);
  setOnButtonClickedClose(true);

  QWidget *content = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(content);
  mainLayout->setSpacing(16);
  mainLayout->setContentsMargins(20, 10, 20, 10);

  // --- Gameplay section ---
  DFrame *gameplayFrame = new DFrame;
  QVBoxLayout *gameplayLayout = new QVBoxLayout(gameplayFrame);
  gameplayLayout->setSpacing(12);
  gameplayLayout->setContentsMargins(16, 14, 16, 14);

  DLabel *gameplayTitle = new DLabel(tr("Gameplay"));
  QFont titleFont = gameplayTitle->font();
  titleFont.setBold(true);
  titleFont.setPointSize(titleFont.pointSize() + 1);
  gameplayTitle->setFont(titleFont);
  gameplayLayout->addWidget(gameplayTitle);

  // Ghost piece toggle
  QHBoxLayout *ghostRow = new QHBoxLayout;
  DLabel *ghostLabel = new DLabel(tr("Ghost piece"));
  ghostLabel->setToolTip(tr("Show a preview of where the piece will land"));
  ghostRow->addWidget(ghostLabel);
  ghostRow->addStretch();
  m_ghostSwitch = new DSwitchButton;
  ghostRow->addWidget(m_ghostSwitch);
  gameplayLayout->addLayout(ghostRow);

  // Difficulty
  QHBoxLayout *diffRow = new QHBoxLayout;
  DLabel *diffLabel = new DLabel(tr("Difficulty"));
  diffRow->addWidget(diffLabel);
  diffRow->addStretch();
  m_diffCombo = new DComboBox;
  m_diffCombo->addItem(tr("Easy"));
  m_diffCombo->addItem(tr("Normal"));
  m_diffCombo->addItem(tr("Hard"));
  m_diffCombo->addItem(tr("Expert"));
  m_diffCombo->setFixedWidth(120);
  diffRow->addWidget(m_diffCombo);
  gameplayLayout->addLayout(diffRow);

  mainLayout->addWidget(gameplayFrame);

  // --- Audio section ---
  DFrame *audioFrame = new DFrame;
  QVBoxLayout *audioLayout = new QVBoxLayout(audioFrame);
  audioLayout->setSpacing(12);
  audioLayout->setContentsMargins(16, 14, 16, 14);

  DLabel *audioTitle = new DLabel(tr("Audio"));
  audioTitle->setFont(titleFont);
  audioLayout->addWidget(audioTitle);

  // Music toggle
  QHBoxLayout *musicRow = new QHBoxLayout;
  DLabel *musicLabel = new DLabel(tr("Music"));
  musicRow->addWidget(musicLabel);
  musicRow->addStretch();
  m_musicSwitch = new DSwitchButton;
  musicRow->addWidget(m_musicSwitch);
  audioLayout->addLayout(musicRow);

  // Volume slider
  QHBoxLayout *volRow = new QHBoxLayout;
  DLabel *volIcon = new DLabel(tr("Volume"));
  volRow->addWidget(volIcon);
  m_volumeSlider = new DSlider(Qt::Horizontal);
  m_volumeSlider->setMinimum(0);
  m_volumeSlider->setMaximum(100);
  m_volumeSlider->setValue(70);
  m_volumeSlider->setFixedWidth(180);
  volRow->addWidget(m_volumeSlider, 1);
  m_volValueLabel = new DLabel("70%");
  m_volValueLabel->setFixedWidth(36);
  m_volValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  volRow->addWidget(m_volValueLabel);
  audioLayout->addLayout(volRow);

  mainLayout->addWidget(audioFrame);
  mainLayout->addStretch();

  addContent(content);

  // Buttons
  addButton(tr("Cancel"));
  addButton(tr("Apply"), true, DDialog::ButtonRecommend);

  // Handle button clicks
  connect(this, &DDialog::buttonClicked, this, [this](int index, const QString &) {
    if (index == 0)
      reject();
    else if (index == 1)
      accept();
  });

  // Settings signals (live apply)
  connect(m_ghostSwitch, &DSwitchButton::toggled, this, &SettingsDialog::ghostPieceToggled);
  connect(m_musicSwitch, &DSwitchButton::toggled, this, &SettingsDialog::musicToggled);
  connect(m_volumeSlider, &DSlider::valueChanged, this, [this](int val) {
    m_volValueLabel->setText(QString("%1%").arg(val));
    emit volumeChanged(val);
  });
  connect(m_diffCombo, QOverload<int>::of(&DComboBox::currentIndexChanged), this,
          [this](int idx) { emit difficultyChanged(idx); });
}

bool SettingsDialog::ghostPieceEnabled() const { return m_ghostSwitch->isChecked(); }
bool SettingsDialog::musicEnabled() const { return m_musicSwitch->isChecked(); }
int SettingsDialog::volume() const { return m_volumeSlider->value(); }
int SettingsDialog::difficulty() const { return m_diffCombo->currentIndex(); }

void SettingsDialog::setGhostPiece(bool enabled) { m_ghostSwitch->setChecked(enabled); }
void SettingsDialog::setMusic(bool enabled) { m_musicSwitch->setChecked(enabled); }
void SettingsDialog::setVolume(int vol) {
  m_volumeSlider->setValue(vol);
  m_volValueLabel->setText(QString("%1%").arg(vol));
}
void SettingsDialog::setDifficulty(int diff) { m_diffCombo->setCurrentIndex(diff); }
