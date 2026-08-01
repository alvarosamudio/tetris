#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>
#include <QMap>
#include <QString>

class AudioStreamer;

class SoundManager : public QObject {
  Q_OBJECT
public:
  enum class Genre {
    Chiptune,
    Rock,
    Electronic,
    Jazz,
    Classical,
    GenreCount
  };

  enum class Theme {
    Korobeiniki,
    Kalinka,
    Troika,
    ThemeCount
  };

  explicit SoundManager(QObject *parent = nullptr);
  ~SoundManager();

  void playRotate();
  void playDrop();
  void playLineClear();
  void playGameOver();
  void setMuted(bool muted);
  void setVolume(float vol);
  void setGenre(Genre genre);
  void setTheme(Theme theme);
  Genre genre() const { return m_currentGenre; }
  Theme theme() const { return m_currentTheme; }
  void startMusic();
  void stopMusic();
  void pauseMusic();
  void resumeMusic();

private:
  QAudioFormat m_audioFormat;
  int m_bufferSize;

  QByteArray m_rotateData;
  AudioStreamer *m_rotateStreamer = nullptr;
  QAudioSink *m_rotateSink = nullptr;

  QByteArray m_dropData;
  AudioStreamer *m_dropStreamer = nullptr;
  QAudioSink *m_dropSink = nullptr;

  QByteArray m_lineClearData;
  AudioStreamer *m_lineClearStreamer = nullptr;
  QAudioSink *m_lineClearSink = nullptr;

  QByteArray m_gameOverData;
  AudioStreamer *m_gameOverStreamer = nullptr;
  QAudioSink *m_gameOverSink = nullptr;

  QByteArray m_musicData;
  AudioStreamer *m_musicStreamer = nullptr;
  QAudioSink *m_musicSink = nullptr;

  bool m_muted = false;
  Genre m_currentGenre = Genre::Chiptune;
  Theme m_currentTheme = Theme::Korobeiniki;

  struct MusicNote {
    double freq;
    int durationMs;
    MusicNote(double f, int d) : freq(f), durationMs(d) {}
  };

  struct MelodyTrack {
    QVector<MusicNote> melody;
    QVector<MusicNote> bass;
  };

  QMap<Theme, MelodyTrack> m_melodies;

  static QByteArray squareWave(double freq, int samples, int sampleRate, double duty, double amp);
  static QByteArray sawWave(double freq, int samples, int sampleRate, double amp);
  static QByteArray triangleWave(double freq, int samples, int sampleRate, double amp);
  static QByteArray sineWave(double freq, int samples, int sampleRate, double amp);
  static QByteArray noise(int samples, double amp);
  static QByteArray distort(const QByteArray &pcm, double gain);

  void buildMelodies();
  void convertToPCM(const QVector<MusicNote> &notes, QByteArray &out, int waveType,
                    int sampleRate, double duty, double amp);
  void rebuildMusicData();
  void rebuildMusicSink();

  void buildKorobeinikiExtended();
  void buildKalinka();
  void buildTroika();

  QAudioSink *makeMusicSink();
};

#endif // SOUNDMANAGER_H