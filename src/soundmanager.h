#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QAudioSink>
#include <QAudioFormat>

class SoundManager : public QObject {
  Q_OBJECT
public:
  explicit SoundManager(QObject *parent = nullptr);

  void playRotate();
  void playDrop();
  void playLineClear();
  void playGameOver();
  void setMuted(bool muted);
  void startMusic();
  void stopMusic();

private:
  QAudioFormat m_audioFormat;

  QByteArray m_rotateData;
  QBuffer *m_rotateBuffer;
  QAudioSink *m_rotateSink;

  QByteArray m_dropData;
  QBuffer *m_dropBuffer;
  QAudioSink *m_dropSink;

  QByteArray m_lineClearData;
  QBuffer *m_lineClearBuffer;
  QAudioSink *m_lineClearSink;

  QByteArray m_gameOverData;
  QBuffer *m_gameOverBuffer;
  QAudioSink *m_gameOverSink;

  QByteArray m_musicData;
  QBuffer *m_musicBuffer;
  QAudioSink *m_musicSink;

  bool m_muted;

  struct MusicNote {
    double freq;
    int durationMs;
    MusicNote(double f, int d) : freq(f), durationMs(d) {}
  };
  void buildMelody();
};

#endif // SOUNDMANAGER_H
