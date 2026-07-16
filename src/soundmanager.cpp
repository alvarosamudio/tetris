#include "soundmanager.h"
#include <QBuffer>
#include <QtEndian>
#include <cmath>

static const double REST = 0.0;

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

  // Sound effects - classic 8-bit style
  // Rotate: quick high beep
  auto setupSink = [this](QByteArray &data, QBuffer *&buffer, QAudioSink *&sink, float defaultVolume) {
    buffer = new QBuffer(&data, this);
    buffer->open(QIODevice::ReadOnly);
    sink = new QAudioSink(m_audioFormat, this);
    sink->setVolume(defaultVolume);
  };

  // Sound effects
  m_rotateData = squareWave(880, 44100 * 40 / 1000, 44100);
  setupSink(m_rotateData, m_rotateBuffer, m_rotateSink, 0.3f);

  // Drop: low thud
  m_dropData = squareWave(150, 44100 * 30 / 1000, 44100);
  m_dropData.append(squareWave(100, 44100 * 70 / 1000, 44100));
  setupSink(m_dropData, m_dropBuffer, m_dropSink, 0.3f);

  // Line clear: ascending arpeggio
  int q = 44100 * 60 / 1000;
  m_lineClearData = squareWave(523, q, 44100);
  m_lineClearData.append(squareWave(659, q, 44100));
  m_lineClearData.append(squareWave(784, q, 44100));
  m_lineClearData.append(squareWave(1047, q * 2, 44100));
  setupSink(m_lineClearData, m_lineClearBuffer, m_lineClearSink, 0.3f);

  // Game over: descending sad notes
  int n = 44100 * 200 / 1000;
  m_gameOverData = squareWave(440, n, 44100);
  m_gameOverData.append(squareWave(370, n, 44100));
  m_gameOverData.append(squareWave(311, n, 44100));
  m_gameOverData.append(squareWave(262, n * 3, 44100));
  setupSink(m_gameOverData, m_gameOverBuffer, m_gameOverSink, 0.3f);

  buildMelody();
  setupSink(m_musicData, m_musicBuffer, m_musicSink, 0.2f);
}

void SoundManager::playRotate() {
  if (m_muted) return;
  m_rotateSink->stop();
  m_rotateBuffer->seek(0);
  m_rotateSink->start(m_rotateBuffer);
}

void SoundManager::playDrop() {
  if (m_muted) return;
  m_dropSink->stop();
  m_dropBuffer->seek(0);
  m_dropSink->start(m_dropBuffer);
}

void SoundManager::playLineClear() {
  if (m_muted) return;
  m_lineClearSink->stop();
  m_lineClearBuffer->seek(0);
  m_lineClearSink->start(m_lineClearBuffer);
}

void SoundManager::playGameOver() {
  stopMusic();
  if (m_muted) return;
  m_gameOverSink->stop();
  m_gameOverBuffer->seek(0);
  m_gameOverSink->start(m_gameOverBuffer);
}

void SoundManager::setMuted(bool muted) {
  m_muted = muted;
  float vol = muted ? 0.0f : 0.3f;
  m_rotateSink->setVolume(vol);
  m_dropSink->setVolume(vol);
  m_lineClearSink->setVolume(vol);
  m_gameOverSink->setVolume(vol);
  
  if (muted) {
    stopMusic();
  }
}

void SoundManager::startMusic() {
  if (m_muted) return;
  m_musicSink->stop();
  m_musicBuffer->seek(0);
  m_musicSink->start(m_musicBuffer);
}

void SoundManager::stopMusic() {
  m_musicSink->stop();
}
