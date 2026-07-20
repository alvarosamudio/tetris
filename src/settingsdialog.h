#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <DDialog>
#include <DSwitchButton>
#include <DSlider>
#include <DComboBox>
#include <DLabel>
#include <DFrame>

DWIDGET_USE_NAMESPACE

class SettingsDialog : public DDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget *parent = nullptr);

  bool ghostPieceEnabled() const;
  bool musicEnabled() const;
  int volume() const;
  int difficulty() const;

  void setGhostPiece(bool enabled);
  void setMusic(bool enabled);
  void setVolume(int vol);
  void setDifficulty(int diff);

signals:
  void ghostPieceToggled(bool enabled);
  void musicToggled(bool enabled);
  void volumeChanged(int vol);
  void difficultyChanged(int diff);

private:
  DSwitchButton *m_ghostSwitch;
  DSwitchButton *m_musicSwitch;
  DSlider *m_volumeSlider;
  DComboBox *m_diffCombo;
  DLabel *m_volValueLabel;
};

#endif // SETTINGSDIALOG_H
