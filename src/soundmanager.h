#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>

class AudioStreamer;

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
  void pauseMusic();
  void resumeMusic();

private:
  QAudioFormat m_audioFormat;

  QByteArray m_rotateData;
  AudioStreamer *m_rotateStreamer;
  QAudioSink *m_rotateSink;

  QByteArray m_dropData;
  AudioStreamer *m_dropStreamer;
  QAudioSink *m_dropSink;

  QByteArray m_lineClearData;
  AudioStreamer *m_lineClearStreamer;
  QAudioSink *m_lineClearSink;

  QByteArray m_gameOverData;
  AudioStreamer *m_gameOverStreamer;
  QAudioSink *m_gameOverSink;

  QByteArray m_musicData;
  AudioStreamer *m_musicStreamer;
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
