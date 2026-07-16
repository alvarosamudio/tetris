#include "soundmanager.h"
#include <QBuffer>
#include <QtEndian>
#include <cmath>
#include <cstring>

static const double REST = 0.0;

struct AudioStreamer : public QIODevice {
    const QByteArray &m_data;
    qint64 m_pos = 0;
    bool m_playing = false;
    bool m_muted = false;
    bool m_looping = false;

    AudioStreamer(const QByteArray &data, bool looping, QObject *parent = nullptr) 
        : QIODevice(parent), m_data(data), m_looping(looping) { open(ReadOnly); }

    void setPlaying(bool p) { m_playing = p; }
    void setMuted(bool m) { m_muted = m; }
    void seekToStart() { m_pos = 0; }

    qint64 readData(char *data, qint64 maxlen) override {
        if (!m_playing || m_data.isEmpty()) {
            memset(data, 0, maxlen);
            return maxlen;
        }
        
        qint64 bytesRead = 0;
        while (bytesRead < maxlen) {
            qint64 chunk = qMin(maxlen - bytesRead, (qint64)m_data.size() - m_pos);
            if (chunk == 0) break;
            
            if (m_muted) {
                memset(data + bytesRead, 0, chunk);
            } else {
                memcpy(data + bytesRead, m_data.constData() + m_pos, chunk);
            }
            
            m_pos = (m_pos + chunk);
            if (m_pos >= m_data.size()) {
                if (m_looping) {
                    m_pos = 0;
                } else {
                    m_playing = false;
                    memset(data + bytesRead + chunk, 0, maxlen - (bytesRead + chunk));
                    return maxlen;
                }
            }
            bytesRead += chunk;
        }
        return maxlen;
    }
    qint64 writeData(const char *, qint64) override { return 0; }
    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return m_data.size() + QIODevice::bytesAvailable(); }
    bool atEnd() const override { return false; }
};

// Note frequencies (Hz)
static const double NOTE_C4 = 261.63, NOTE_D4 = 293.66, NOTE_E4 = 329.63;
static const double NOTE_F4 = 349.23, NOTE_G4 = 392.00, NOTE_A4 = 440.00;
static const double NOTE_B4 = 493.88;
static const double NOTE_C5 = 523.25, NOTE_D5 = 587.33, NOTE_E5 = 659.25;
static const double NOTE_F5 = 698.46, NOTE_G5 = 783.99, NOTE_GS5 = 830.61;
static const double NOTE_A5 = 880.00;
static const double NOTE_B5 = 987.77;
static const double NOTE_C3 = 130.81, NOTE_D3 = 146.83, NOTE_E3 = 164.81;
static const double NOTE_F3 = 174.61, NOTE_G3 = 196.00, NOTE_A3 = 220.00;
static const double NOTE_GS3 = 207.65;
static const double NOTE_B3 = 246.94;

// Generate a square wave (classic 8-bit sound)
static QByteArray squareWave(double freq, int numSamples, int sampleRate) {
  QByteArray pcm;
  pcm.resize(numSamples * 2);
  for (int i = 0; i < numSamples; ++i) {
    double t = static_cast<double>(i) / sampleRate;
    double val = (sin(2.0 * M_PI * freq * t) >= 0.0) ? 0.4 : -0.4;
    // Simple envelope
    double progress = static_cast<double>(i) / numSamples;
    double env = 1.0;
    if (progress < 0.05) env = progress / 0.05;
    else if (progress > 0.85) env = (1.0 - progress) / 0.15;
    val *= env;
    qint16 s = static_cast<qint16>(val * 32767);
    qToLittleEndian<qint16>(s, reinterpret_cast<uchar*>(pcm.data() + i * 2));
  }
  return pcm;
}

void SoundManager::buildMelody() {
  // Korobeiniki (Tetris Theme A) - classic 8-bit arrangement
  // Quarter note = 200ms, eighth = 100ms, half = 400ms, whole = 800ms
  int q = 200, e = 100, h = 400, w = 800, dq = 300;

  QVector<MusicNote> melody = {
    // Line 1
    {NOTE_E5, q}, {NOTE_B4, e}, {NOTE_C5, e}, {NOTE_D5, q}, {NOTE_C5, e}, {NOTE_B4, e},
    {NOTE_A4, q}, {NOTE_A4, e}, {NOTE_C5, e}, {NOTE_E5, q}, {NOTE_D5, e}, {NOTE_C5, e},
    {NOTE_B4, dq}, {NOTE_C5, e}, {NOTE_D5, q}, {NOTE_E5, q},
    {NOTE_C5, q}, {NOTE_A4, q}, {NOTE_A4, q}, {REST, e},

    // Line 2
    {REST, e}, {NOTE_D5, q}, {NOTE_F5, e}, {NOTE_A5, q}, {NOTE_G5, e}, {NOTE_F5, e},
    {NOTE_E5, dq}, {NOTE_C5, e}, {NOTE_E5, q}, {NOTE_D5, e}, {NOTE_C5, e},
    {NOTE_B4, q}, {NOTE_B4, e}, {NOTE_C5, e}, {NOTE_D5, q}, {NOTE_E5, q},
    {NOTE_C5, q}, {NOTE_A4, q}, {NOTE_A4, q}, {REST, q},

    // Line 3 (bridge)
    {NOTE_E5, h}, {NOTE_C5, h},
    {NOTE_D5, h}, {NOTE_B4, h},
    {NOTE_C5, h}, {NOTE_A4, h},
    {NOTE_B4, w},

    {NOTE_E5, h}, {NOTE_C5, h},
    {NOTE_D5, h}, {NOTE_B4, h},
    {NOTE_C5, q}, {NOTE_E5, q}, {NOTE_A5, h},
    {NOTE_GS5, w},

    // Line 4 (repeat of line 1)
    {NOTE_E5, q}, {NOTE_B4, e}, {NOTE_C5, e}, {NOTE_D5, q}, {NOTE_C5, e}, {NOTE_B4, e},
    {NOTE_A4, q}, {NOTE_A4, e}, {NOTE_C5, e}, {NOTE_E5, q}, {NOTE_D5, e}, {NOTE_C5, e},
    {NOTE_B4, dq}, {NOTE_C5, e}, {NOTE_D5, q}, {NOTE_E5, q},
    {NOTE_C5, q}, {NOTE_A4, q}, {NOTE_A4, q}, {REST, e},

    // Line 5 (repeat of line 2)
    {REST, e}, {NOTE_D5, q}, {NOTE_F5, e}, {NOTE_A5, q}, {NOTE_G5, e}, {NOTE_F5, e},
    {REST, e}, {NOTE_E5, q}, {NOTE_C5, e}, {NOTE_E5, q}, {NOTE_D5, e}, {NOTE_C5, e},
    {REST, e}, {NOTE_B4, q}, {NOTE_C5, e}, {NOTE_D5, q}, {NOTE_E5, q},
    {REST, e}, {NOTE_C5, q}, {NOTE_A4, e}, {NOTE_A4, q}, {REST, q},
  };

  // Simple bass line following the melody
  QVector<MusicNote> bass = {
    {NOTE_E3, q}, {NOTE_E3, e}, {NOTE_E3, e}, {NOTE_E3, q}, {NOTE_E3, e}, {NOTE_E3, e},
    {NOTE_A3, q}, {NOTE_A3, e}, {NOTE_A3, e}, {NOTE_A3, q}, {NOTE_A3, e}, {NOTE_A3, e},
    {NOTE_GS3, dq}, {NOTE_GS3, e}, {NOTE_GS3, q}, {NOTE_GS3, q},
    {NOTE_A3, q}, {NOTE_A3, q}, {NOTE_A3, q}, {REST, e},

    {REST, e}, {NOTE_D3, q}, {NOTE_D3, e}, {NOTE_D3, q}, {NOTE_D3, e}, {NOTE_D3, e},
    {NOTE_C3, dq}, {NOTE_C3, e}, {NOTE_C3, q}, {NOTE_C3, e}, {NOTE_C3, e},
    {NOTE_G3, q}, {NOTE_G3, e}, {NOTE_G3, e}, {NOTE_G3, q}, {NOTE_G3, q},
    {NOTE_A3, q}, {NOTE_A3, q}, {NOTE_A3, q}, {REST, q},

    {NOTE_E3, h}, {NOTE_E3, h},
    {NOTE_GS3, h}, {NOTE_GS3, h},
    {NOTE_A3, h}, {NOTE_A3, h},
    {NOTE_E3, w},

    {NOTE_E3, h}, {NOTE_E3, h},
    {NOTE_GS3, h}, {NOTE_GS3, h},
    {NOTE_A3, q}, {NOTE_E3, q}, {NOTE_E3, h},
    {NOTE_E3, w},

    {NOTE_E3, q}, {NOTE_E3, e}, {NOTE_E3, e}, {NOTE_E3, q}, {NOTE_E3, e}, {NOTE_E3, e},
    {NOTE_A3, q}, {NOTE_A3, e}, {NOTE_A3, e}, {NOTE_A3, q}, {NOTE_A3, e}, {NOTE_A3, e},
    {NOTE_GS3, dq}, {NOTE_GS3, e}, {NOTE_GS3, q}, {NOTE_GS3, q},
    {NOTE_A3, q}, {NOTE_A3, q}, {NOTE_A3, q}, {REST, e},

    {REST, e}, {NOTE_D3, q}, {NOTE_D3, e}, {NOTE_D3, q}, {NOTE_D3, e}, {NOTE_D3, e},
    {REST, e}, {NOTE_C3, q}, {NOTE_C3, e}, {NOTE_C3, q}, {NOTE_C3, e}, {NOTE_C3, e},
    {REST, e}, {NOTE_G3, q}, {NOTE_G3, e}, {NOTE_G3, q}, {NOTE_G3, q},
    {REST, e}, {NOTE_A3, q}, {NOTE_A3, e}, {NOTE_A3, q}, {REST, q},
  };

  QByteArray melodyData;
  for (const auto& note : melody) {
      if (note.freq > 0) {
          QByteArray p = squareWave(note.freq, 44100 * note.durationMs / 1000 * 80 / 100, 44100);
          melodyData.append(p);
          int restSamples = 44100 * note.durationMs / 1000 * 20 / 100;
          if (restSamples > 0) {
            melodyData.append(QByteArray(restSamples * 2, 0));
          }
      } else {
          int samples = 44100 * note.durationMs / 1000;
          melodyData.append(QByteArray(samples * 2, 0));
      }
  }
  
  QByteArray bassData;
  for (const auto& note : bass) {
      if (note.freq > 0) {
          QByteArray p = squareWave(note.freq, 44100 * note.durationMs / 1000 * 80 / 100, 44100);
          bassData.append(p);
          int restSamples = 44100 * note.durationMs / 1000 * 20 / 100;
          if (restSamples > 0) {
            bassData.append(QByteArray(restSamples * 2, 0));
          }
      } else {
          int samples = 44100 * note.durationMs / 1000;
          bassData.append(QByteArray(samples * 2, 0));
      }
  }
  
  int minSize = qMin(melodyData.size(), bassData.size()) / 2;
  m_musicData.resize(minSize * 2);
  const qint16* m = reinterpret_cast<const qint16*>(melodyData.constData());
  const qint16* b = reinterpret_cast<const qint16*>(bassData.constData());
  qint16* out = reinterpret_cast<qint16*>(m_musicData.data());
  
  for (int i = 0; i < minSize; ++i) {
      out[i] = static_cast<qint16>((m[i] * 0.6) + (b[i] * 0.4));
  }
}

SoundManager::SoundManager(QObject *parent)
    : QObject(parent), m_muted(false) {

  m_audioFormat.setSampleRate(44100);
  m_audioFormat.setChannelCount(1);
  m_audioFormat.setSampleFormat(QAudioFormat::Int16);

  int bufferSize = 44100 * 2 * 1 * 50 / 1000; // 50ms buffer for low latency

  auto setupSink = [this, bufferSize](QByteArray &data, AudioStreamer *&streamer, QAudioSink *&sink, float defaultVolume) {
    streamer = new AudioStreamer(data, false, this);
    sink = new QAudioSink(m_audioFormat, this);
    sink->setBufferSize(bufferSize);
    sink->setVolume(defaultVolume);
    sink->start(streamer);
  };

  // Sound effects - classic 8-bit style
  // Rotate: quick high beep
  m_rotateData = squareWave(880, 44100 * 40 / 1000, 44100);
  setupSink(m_rotateData, m_rotateStreamer, m_rotateSink, 0.3f);

  // Drop: low thud
  m_dropData = squareWave(150, 44100 * 30 / 1000, 44100);
  m_dropData.append(squareWave(100, 44100 * 70 / 1000, 44100));
  setupSink(m_dropData, m_dropStreamer, m_dropSink, 0.3f);

  // Line clear: ascending arpeggio
  int q = 44100 * 60 / 1000;
  m_lineClearData = squareWave(523, q, 44100);
  m_lineClearData.append(squareWave(659, q, 44100));
  m_lineClearData.append(squareWave(784, q, 44100));
  m_lineClearData.append(squareWave(1047, q * 2, 44100));
  setupSink(m_lineClearData, m_lineClearStreamer, m_lineClearSink, 0.3f);

  // Game over: descending sad notes
  int n = 44100 * 200 / 1000;
  m_gameOverData = squareWave(440, n, 44100);
  m_gameOverData.append(squareWave(370, n, 44100));
  m_gameOverData.append(squareWave(311, n, 44100));
  m_gameOverData.append(squareWave(262, n * 3, 44100));
  setupSink(m_gameOverData, m_gameOverStreamer, m_gameOverSink, 0.3f);

  buildMelody();
  m_musicStreamer = new AudioStreamer(m_musicData, true, this);
  m_musicSink = new QAudioSink(m_audioFormat, this);
  m_musicSink->setBufferSize(bufferSize);
  m_musicSink->setVolume(0.2f);
  m_musicSink->start(m_musicStreamer);
}

void SoundManager::playRotate() {
  if (m_muted) return;
  m_rotateStreamer->seekToStart();
  m_rotateStreamer->setPlaying(true);
}

void SoundManager::playDrop() {
  if (m_muted) return;
  m_dropStreamer->seekToStart();
  m_dropStreamer->setPlaying(true);
}

void SoundManager::playLineClear() {
  if (m_muted) return;
  m_lineClearStreamer->seekToStart();
  m_lineClearStreamer->setPlaying(true);
}

void SoundManager::playGameOver() {
  stopMusic();
  if (m_muted) return;
  m_gameOverStreamer->seekToStart();
  m_gameOverStreamer->setPlaying(true);
}

void SoundManager::setMuted(bool muted) {
  m_muted = muted;
  if (muted) {
    if (m_rotateStreamer) m_rotateStreamer->setPlaying(false);
    if (m_dropStreamer) m_dropStreamer->setPlaying(false);
    if (m_lineClearStreamer) m_lineClearStreamer->setPlaying(false);
    if (m_gameOverStreamer) m_gameOverStreamer->setPlaying(false);
  }
  float vol = muted ? 0.0f : 0.3f;
  m_rotateSink->setVolume(vol);
  m_dropSink->setVolume(vol);
  m_lineClearSink->setVolume(vol);
  m_gameOverSink->setVolume(vol);
  
  if (m_musicStreamer) {
    m_musicStreamer->setMuted(muted);
  }
  if (m_musicSink) {
    m_musicSink->setVolume(muted ? 0.0f : 0.2f);
  }
}

void SoundManager::startMusic() {
  m_musicStreamer->seekToStart();
  m_musicStreamer->setPlaying(true);
  if (m_musicSink) {
    m_musicSink->setVolume(m_muted ? 0.0f : 0.2f);
  }
}

void SoundManager::stopMusic() {
  if (m_musicStreamer) {
    m_musicStreamer->setPlaying(false);
  }
}

void SoundManager::pauseMusic() {
  if (m_musicStreamer) {
    m_musicStreamer->setPlaying(false);
  }
  if (m_musicSink) {
    m_musicSink->setVolume(0.0f);
  }
}

void SoundManager::resumeMusic() {
  if (m_musicStreamer) {
    m_musicStreamer->setPlaying(true);
  }
  if (m_musicSink) {
    m_musicSink->setVolume(m_muted ? 0.0f : 0.2f);
  }
}
