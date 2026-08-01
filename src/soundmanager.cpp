#include "soundmanager.h"
#include <QBuffer>
#include <QtEndian>
#include <cmath>
#include <cstring>
#include <random>

static const double REST = 0.0;

static const double NOTE_C2 = 65.41, NOTE_D2 = 73.42, NOTE_E2 = 82.41;
static const double NOTE_G2 = 98.00;

static const double NOTE_C3 = 130.81, NOTE_D3 = 146.83, NOTE_E3 = 164.81;
static const double NOTE_F3 = 174.61, NOTE_G3 = 196.00, NOTE_GS3 = 207.65;
static const double NOTE_A3 = 220.00, NOTE_B3 = 246.94;

static const double NOTE_C4 = 261.63, NOTE_D4 = 293.66, NOTE_E4 = 329.63;
static const double NOTE_F4 = 349.23, NOTE_G4 = 392.00, NOTE_A4 = 440.00;
static const double NOTE_B4 = 493.88;

static const double NOTE_C5 = 523.25, NOTE_D5 = 587.33, NOTE_E5 = 659.25;
static const double NOTE_F5 = 698.46, NOTE_G5 = 783.99, NOTE_GS5 = 830.61;
static const double NOTE_A5 = 880.00, NOTE_B5 = 987.77;

static const double NOTE_C6 = 1046.50, NOTE_D6 = 1174.66, NOTE_E6 = 1318.51;
static const double NOTE_F6 = 1396.91, NOTE_G6 = 1567.98;

struct AudioStreamer : public QIODevice {
    const QByteArray *m_data;
    qint64 m_pos = 0;
    bool m_playing = false;
    bool m_muted = false;
    bool m_looping = false;

    AudioStreamer(const QByteArray *data, bool looping, QObject *parent = nullptr)
        : QIODevice(parent), m_data(data), m_looping(looping) { open(ReadOnly); }

    void setPlaying(bool p) { m_playing = p; }
    void setMuted(bool m) { m_muted = m; }
    void seekToStart() { m_pos = 0; }
    void setData(const QByteArray *data) { m_data = data; m_pos = 0; }

    qint64 readData(char *data, qint64 maxlen) override {
        if (!m_playing || !m_data || m_data->isEmpty()) {
            memset(data, 0, maxlen);
            return maxlen;
        }
        qint64 bytesRead = 0;
        while (bytesRead < maxlen) {
            qint64 chunk = qMin(maxlen - bytesRead, (qint64)m_data->size() - m_pos);
            if (chunk == 0) break;
            if (m_muted)
                memset(data + bytesRead, 0, chunk);
            else
                memcpy(data + bytesRead, m_data->constData() + m_pos, chunk);
            m_pos += chunk;
            if (m_pos >= (qint64)m_data->size()) {
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
    qint64 bytesAvailable() const override {
        return m_data ? m_data->size() + QIODevice::bytesAvailable() : QIODevice::bytesAvailable();
    }
    bool atEnd() const override { return false; }
};

static QByteArray genWave(double freq, int samples, int sampleRate, int waveType,
                          double duty, double amp) {
    QByteArray pcm;
    pcm.resize(samples * 2);
    for (int i = 0; i < samples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double phase = fmod(2.0 * M_PI * freq * t, 2.0 * M_PI);
        double val = 0.0;

        switch (waveType) {
        case 0: { // Square
            double threshold = 2.0 * M_PI * (1.0 - duty);
            val = (phase >= threshold) ? amp : -amp;
            break;
        }
        case 1: { // Sawtooth
            val = (2.0 * amp * (phase / (2.0 * M_PI))) - amp;
            break;
        }
        case 2: { // Triangle
            double p = phase / (2.0 * M_PI);
            val = (4.0 * amp * (p < 0.5 ? p : 1.0 - p)) - amp;
            break;
        }
        case 3: // Sine
        default:
            val = amp * sin(phase);
            break;
        }

        double progress = static_cast<double>(i) / samples;
        double env = 1.0;
        if (progress < 0.05) env = progress / 0.05;
        else if (progress > 0.85) env = (1.0 - progress) / 0.15;
        val *= env;

        qint16 s = static_cast<qint16>(qBound(-32767.0, val * 32767.0, 32767.0));
        qToLittleEndian<qint16>(s, reinterpret_cast<uchar*>(pcm.data() + i * 2));
    }
    return pcm;
}

static QByteArray kickDrum(int sampleRate, int durationMs) {
    int samples = sampleRate * durationMs / 1000;
    QByteArray pcm;
    pcm.resize(samples * 2);
    for (int i = 0; i < samples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double val = sin(2.0 * M_PI * 150.0 * exp(-t * 12.0) * t) * exp(-t * 8.0) * 0.5;
        qint16 s = static_cast<qint16>(val * 32767);
        qToLittleEndian<qint16>(s, reinterpret_cast<uchar*>(pcm.data() + i * 2));
    }
    return pcm;
}
static QByteArray snareDrum(int sampleRate, int durationMs) {
    int samples = sampleRate * durationMs / 1000;
    QByteArray pcm;
    pcm.resize(samples * 2);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < samples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double val = (dist(gen) * 2.0 - 1.0) * exp(-t * 15.0) * 0.35;
        qint16 s = static_cast<qint16>(val * 32767);
        qToLittleEndian<qint16>(s, reinterpret_cast<uchar*>(pcm.data() + i * 2));
    }
    return pcm;
}
static QByteArray hiHat(int sampleRate, int durationMs) {
    int samples = sampleRate * durationMs / 1000;
    QByteArray pcm;
    pcm.resize(samples * 2);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (int i = 0; i < samples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double val = (dist(gen) * 2.0 - 1.0) * exp(-t * 40.0) * 0.25;
        qint16 s = static_cast<qint16>(val * 32767);
        qToLittleEndian<qint16>(s, reinterpret_cast<uchar*>(pcm.data() + i * 2));
    }
    return pcm;
}

void SoundManager::convertToPCM(const QVector<MusicNote> &notes, QByteArray &out,
                                int waveType, int sampleRate, double duty, double amp) {
    for (const auto &note : notes) {
        if (note.freq > 0) {
            QByteArray p = genWave(note.freq, sampleRate * note.durationMs / 1000 * 80 / 100,
                                   sampleRate, waveType, duty, amp);
            out.append(p);
            int restSamples = sampleRate * note.durationMs / 1000 * 20 / 100;
            if (restSamples > 0)
                out.append(QByteArray(restSamples * 2, 0));
        } else {
            int samples = sampleRate * note.durationMs / 1000;
            out.append(QByteArray(samples * 2, 0));
        }
    }
}

void SoundManager::rebuildMusicSink() {
    bool wasPlaying = m_musicStreamer && m_musicStreamer->m_playing;
    if (m_musicSink) {
        m_musicSink->stop();
        delete m_musicSink;
        m_musicSink = nullptr;
    }
    m_musicSink = new QAudioSink(m_audioFormat, this);
    m_musicSink->setBufferSize(m_bufferSize);
    m_musicSink->setVolume(m_muted ? 0.0f : 0.2f);
    m_musicStreamer->seekToStart();
    m_musicStreamer->setData(&m_musicData);
    m_musicSink->start(m_musicStreamer);
    if (wasPlaying)
        m_musicStreamer->setPlaying(true);
}

void SoundManager::rebuildMusicData() {
    int sampleRate = m_audioFormat.sampleRate();
    MelodyTrack track = m_melodies.value(m_currentTheme);
    Genre g = m_currentGenre;

    int waveTypeMel = 0, waveTypeBass = 0;
    double ampMel = 0.6, ampBass = 0.4;
    double duty = 0.5;

    switch (g) {
    case Genre::Chiptune:
        waveTypeMel = 0; waveTypeBass = 0; ampMel = 0.6; ampBass = 0.4; duty = 0.5;
        break;
    case Genre::Rock:
        waveTypeMel = 1; waveTypeBass = 1; ampMel = 0.5; ampBass = 0.3; duty = 0.45;
        break;
    case Genre::Electronic:
        waveTypeMel = 0; waveTypeBass = 1; ampMel = 0.45; ampBass = 0.5; duty = 0.3;
        break;
    case Genre::Jazz:
        waveTypeMel = 3; waveTypeBass = 3; ampMel = 0.55; ampBass = 0.35; duty = 0.5;
        break;
    case Genre::Classical:
        waveTypeMel = 2; waveTypeBass = 3; ampMel = 0.5; ampBass = 0.5; duty = 0.5;
        break;
    default:
        break;
    }

    QByteArray melodyPCM, bassPCM;
    convertToPCM(track.melody, melodyPCM, waveTypeMel, sampleRate, duty, ampMel);
    convertToPCM(track.bass, bassPCM, waveTypeBass, sampleRate, duty, ampBass);

    int minSize = qMin(melodyPCM.size(), bassPCM.size()) / 2;
    m_musicData.resize(minSize * 2);
    const qint16 *m = reinterpret_cast<const qint16*>(melodyPCM.constData());
    const qint16 *b = reinterpret_cast<const qint16*>(bassPCM.constData());
    qint16 *out = reinterpret_cast<qint16*>(m_musicData.data());

    for (int i = 0; i < minSize; ++i)
        out[i] = static_cast<qint16>(qBound(-32768.0, m[i] * ampMel + b[i] * ampBass, 32767.0));

    rebuildMusicSink();
}

void SoundManager::setGenre(Genre genre) {
    if (m_currentGenre == genre) return;
    m_currentGenre = genre;
    rebuildMusicData();
}

void SoundManager::setTheme(Theme theme) {
    if (m_currentTheme == theme) return;
    m_currentTheme = theme;
    rebuildMusicData();
}

// =================== Extended Korobeiniki ===================

void SoundManager::buildKorobeinikiExtended() {
    int q = 200, e = 100, h = 400, w = 800, dq = 300;

    QVector<MusicNote> melody = {
        // A1
        {NOTE_E5,q}, {NOTE_B4,e}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_C5,e}, {NOTE_B4,e},
        {NOTE_A4,q}, {NOTE_A4,e}, {NOTE_C5,e}, {NOTE_E5,q}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_B4,dq}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_E5,q},
        {NOTE_C5,q}, {NOTE_A4,q}, {NOTE_A4,q}, {REST,e},

        // A2
        {REST,e}, {NOTE_D5,q}, {NOTE_F5,e}, {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_F5,e},
        {NOTE_E5,dq}, {NOTE_C5,e}, {NOTE_E5,q}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_B4,q}, {NOTE_B4,e}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_E5,q},
        {NOTE_C5,q}, {NOTE_A4,q}, {NOTE_A4,q}, {REST,q},

        // Bridge 1
        {NOTE_E5,h}, {NOTE_C5,h},
        {NOTE_D5,h}, {NOTE_B4,h},
        {NOTE_C5,h}, {NOTE_A4,h},
        {NOTE_B4,w},

        // Bridge 2
        {NOTE_E5,h}, {NOTE_C5,h},
        {NOTE_D5,h}, {NOTE_B4,h},
        {NOTE_C5,q}, {NOTE_E5,q}, {NOTE_A5,h},
        {NOTE_GS5,w},

        // A1 reprise
        {NOTE_E5,q}, {NOTE_B4,e}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_C5,e}, {NOTE_B4,e},
        {NOTE_A4,q}, {NOTE_A4,e}, {NOTE_C5,e}, {NOTE_E5,q}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_B4,dq}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_E5,q},
        {NOTE_C5,q}, {NOTE_A4,q}, {NOTE_A4,q}, {REST,e},

        // A2 reprise
        {REST,e}, {NOTE_D5,q}, {NOTE_F5,e}, {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_F5,e},
        {REST,e}, {NOTE_E5,q}, {NOTE_C5,e}, {NOTE_E5,q}, {NOTE_D5,e}, {NOTE_C5,e},
        {REST,e}, {NOTE_B4,q}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_E5,q},
        {REST,e}, {NOTE_C5,q}, {NOTE_A4,e}, {NOTE_A4,q}, {REST,q},

        // Section B (developmental)
        {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_A5,e}, {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_F5,e},
        {NOTE_E5,h}, {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_F5,e},
        {NOTE_E5,w}, {REST,q},

        {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_A5,e}, {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_F5,e},
        {NOTE_E5,h}, {NOTE_D5,q}, {NOTE_C5,e}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_E5,w}, {REST,q},

        // Climax
        {NOTE_C6,q}, {NOTE_B5,e}, {NOTE_C6,e}, {NOTE_D6,q}, {NOTE_C6,e}, {NOTE_B5,e},
        {NOTE_A5,h}, {NOTE_A5,q}, {NOTE_G5,e}, {NOTE_F5,e},
        {NOTE_E5,w}, {REST,q},

        {NOTE_C6,q}, {NOTE_B5,e}, {NOTE_C6,e}, {NOTE_D6,q}, {NOTE_E6,q},
        {NOTE_D6,q}, {NOTE_C6,q},
        {REST,w},

        // Final A1
        {NOTE_E5,q}, {NOTE_B4,e}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_C5,e}, {NOTE_B4,e},
        {NOTE_A4,q}, {NOTE_A4,e}, {NOTE_C5,e}, {NOTE_E5,q}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_B4,dq}, {NOTE_C5,e}, {NOTE_D5,q}, {NOTE_E5,q},
        {NOTE_C5,q}, {NOTE_A4,q}, {NOTE_A4,q}, {REST,w},
    };

    QVector<MusicNote> bass = {
        // A1
        {NOTE_E3,q}, {NOTE_E3,e}, {NOTE_E3,e}, {NOTE_E3,q}, {NOTE_E3,e}, {NOTE_E3,e},
        {NOTE_A3,q}, {NOTE_A3,e}, {NOTE_A3,e}, {NOTE_A3,q}, {NOTE_A3,e}, {NOTE_A3,e},
        {NOTE_GS3,dq}, {NOTE_GS3,e}, {NOTE_GS3,q}, {NOTE_GS3,q},
        {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q}, {REST,e},

        // A2
        {REST,e}, {NOTE_D3,q}, {NOTE_D3,e}, {NOTE_D3,q}, {NOTE_D3,e}, {NOTE_D3,e},
        {NOTE_C3,dq}, {NOTE_C3,e}, {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_C3,e},
        {NOTE_G3,q}, {NOTE_G3,e}, {NOTE_G3,e}, {NOTE_G3,q}, {NOTE_G3,q},
        {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q}, {REST,q},

        // Bridge
        {NOTE_E3,h}, {NOTE_A3,h},
        {NOTE_D3,h}, {NOTE_B3,h},
        {NOTE_C3,h}, {NOTE_A3,h},
        {NOTE_E3,w},

        {NOTE_E3,h}, {NOTE_E3,h},
        {NOTE_D3,h}, {NOTE_D3,h},
        {NOTE_C3,q}, {NOTE_E3,q}, {NOTE_E3,h},
        {NOTE_E3,w},

        // A1 reprise
        {NOTE_E3,q}, {NOTE_E3,e}, {NOTE_E3,e}, {NOTE_E3,q}, {NOTE_E3,e}, {NOTE_E3,e},
        {NOTE_A3,q}, {NOTE_A3,e}, {NOTE_A3,e}, {NOTE_A3,q}, {NOTE_A3,e}, {NOTE_A3,e},
        {NOTE_GS3,dq}, {NOTE_GS3,e}, {NOTE_GS3,q}, {NOTE_GS3,q},
        {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q}, {REST,e},

        // A2 reprise
        {REST,e}, {NOTE_D3,q}, {NOTE_D3,e}, {NOTE_D3,q}, {NOTE_D3,e}, {NOTE_D3,e},
        {REST,e}, {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_C3,e},
        {REST,e}, {NOTE_G3,q}, {NOTE_G3,e}, {NOTE_G3,q}, {NOTE_G3,q},
        {REST,e}, {NOTE_A3,q}, {NOTE_A3,e}, {NOTE_A3,q}, {REST,q},

        // B
        {REST,e}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q},
        {REST,q}, {NOTE_A3,q}, {NOTE_A3,q},
        {REST,q}, {NOTE_E3,q}, {REST,q}, {NOTE_E3,q},

        {REST,e}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q},
        {REST,q}, {NOTE_G3,q},
        {REST,q}, {NOTE_E3,q}, {REST,q}, {NOTE_E3,q},

        // Climax
        {REST,e}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q},
        {REST,q}, {NOTE_A3,q}, {NOTE_A3,q},
        {REST,q}, {NOTE_E3,q}, {REST,q}, {NOTE_E3,q},

        {REST,e}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q}, {NOTE_A3,q},
        {REST,q}, {NOTE_D3,q},
        {REST,q}, {REST,q},

        // Outro
        {NOTE_E3,q}, {NOTE_E3,e}, {NOTE_E3,e}, {NOTE_E3,q}, {NOTE_E3,e}, {NOTE_E3,e},
        {REST,q}, {REST,q}, {REST,q}, {REST,q},
        {REST,q}, {REST,q}, {REST,q}, {REST,q},
        {REST,e}, {NOTE_A3,q}, {NOTE_A3,e}, {NOTE_A3,q}, {REST,w},
    };

    m_melodies[Theme::Korobeiniki] = {melody, bass};
}

// =================== Kalinka ===================

void SoundManager::buildKalinka() {
    int q = 200, e = 100, h = 400, w = 800;

    QVector<MusicNote> melody = {
        {REST,q}, {NOTE_E4,q}, {NOTE_F4,q}, {NOTE_G4,w}, {REST,q},
        {NOTE_E4,h}, {NOTE_D4,q}, {NOTE_C4,h},
        {NOTE_E4,q}, {NOTE_D4,q}, {NOTE_C4,q}, {NOTE_D4,q},
        {NOTE_E4,h}, {NOTE_D4,q}, {NOTE_C4,q},
        {NOTE_D4,w},

        {REST,q}, {NOTE_G4,q}, {NOTE_A4,q}, {NOTE_B4,w}, {REST,q},
        {NOTE_G4,h}, {NOTE_F4,q}, {NOTE_E4,h},
        {NOTE_G4,q}, {NOTE_F4,q}, {NOTE_E4,q}, {NOTE_F4,q},
        {NOTE_G4,h}, {REST,h},

        {NOTE_E5,q}, {NOTE_D5,q}, {NOTE_C5,e}, {NOTE_B4,e}, {NOTE_C5,q},
        {NOTE_D5,q}, {NOTE_E5,e}, {NOTE_F5,e}, {NOTE_G5,w},
        {NOTE_F5,q}, {NOTE_E5,q}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_D5,q}, {NOTE_E5,q}, {NOTE_F5,w},

        {REST,q}, {NOTE_E4,q}, {NOTE_F4,q}, {NOTE_G4,w}, {REST,q},
        {NOTE_E4,h}, {NOTE_D4,q}, {NOTE_C4,h}, {REST,q},
        {NOTE_G4,q}, {NOTE_F4,q}, {NOTE_E4,q}, {NOTE_F4,q},
        {NOTE_G4,h}, {NOTE_E4,q}, {REST,q},

        {NOTE_E4,q}, {NOTE_G4,q}, {NOTE_C5,q}, {NOTE_E5,e}, {NOTE_D5,e},
        {NOTE_C5,q}, {NOTE_G4,q}, {NOTE_E4,w},
        {NOTE_G4,q}, {NOTE_C5,e}, {NOTE_E5,e}, {NOTE_G5,q}, {NOTE_A5,e}, {NOTE_G5,e},
        {NOTE_C6,w}, {REST,w},
    };

    QVector<MusicNote> bass = {
        {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q},
        {NOTE_C3,q}, {NOTE_G2,q}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_C3,q},

        {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q},
        {NOTE_C3,q}, {NOTE_G2,q}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_C3,q}, {NOTE_G2,q},

        {NOTE_F3,q}, {NOTE_F3,q}, {NOTE_F3,e}, {NOTE_F3,q},
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {REST,q},

        {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_C3,q}, {REST,q},

        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_C3,q},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q},
        {NOTE_C3,q}, {NOTE_E3,q}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q}, {REST,w},
    };

    m_melodies[Theme::Kalinka] = {melody, bass};
}

// =================== Troika ===================

void SoundManager::buildTroika() {
    int q = 180, e = 90, h = 360, w = 720;

    QVector<MusicNote> melody = {
        {NOTE_E4,q}, {NOTE_G4,q}, {NOTE_C5,h}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_D5,q}, {NOTE_E5,q}, {NOTE_C5,w},
        {NOTE_D5,q}, {NOTE_C5,q}, {NOTE_G4,h}, {NOTE_A4,e}, {NOTE_B4,e},
        {NOTE_C5,q}, {NOTE_A4,q}, {NOTE_G4,w},

        {NOTE_E5,q}, {NOTE_G5,q}, {NOTE_C6,h}, {NOTE_D6,e}, {NOTE_C6,e},
        {NOTE_D6,q}, {NOTE_E6,q}, {NOTE_C6,w},
        {NOTE_D6,q}, {NOTE_E6,q}, {NOTE_C6,h}, {NOTE_D6,e}, {NOTE_C6,e},
        {NOTE_B4,q}, {NOTE_C5,q}, {NOTE_A4,w},

        {NOTE_E4,q}, {NOTE_G4,q}, {NOTE_C5,h}, {NOTE_D5,e}, {NOTE_E5,e},
        {NOTE_F5,q}, {NOTE_E5,q}, {NOTE_D5,h}, {REST,q},
        {NOTE_E5,q}, {NOTE_D5,q}, {NOTE_C5,h}, {NOTE_D5,e}, {NOTE_C5,e},
        {NOTE_D5,q}, {NOTE_E5,q}, {NOTE_C5,w},

        {NOTE_C5,q}, {NOTE_E5,q}, {NOTE_G5,h}, {NOTE_C6,e}, {NOTE_E6,e},
        {NOTE_D6,q}, {NOTE_C6,q}, {NOTE_G5,w},
        {NOTE_E6,q}, {NOTE_D6,q}, {NOTE_C6,h}, {NOTE_G5,e}, {NOTE_A5,e},
        {NOTE_B5,q}, {NOTE_C6,q}, {REST,w},
    };

    QVector<MusicNote> bass = {
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_E3,e},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,e}, {NOTE_G2,e},
        {NOTE_C3,q}, {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q},
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q},

        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_E3,e},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,e}, {NOTE_G2,e},
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q}, {REST,q},
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q},

        {NOTE_F3,q}, {NOTE_F3,q}, {NOTE_F3,e}, {NOTE_F3,e},
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q},
        {NOTE_C3,q}, {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q},
        {NOTE_C3,q}, {NOTE_G2,q}, {NOTE_C3,q}, {NOTE_C3,q},

        {NOTE_F3,q}, {NOTE_C3,q}, {NOTE_C3,e}, {NOTE_E3,e},
        {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q}, {NOTE_G2,q},
        {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_E3,q}, {NOTE_E3,q},
        {NOTE_F3,q}, {NOTE_C3,q}, {NOTE_C3,q}, {NOTE_C3,q},
    };

    m_melodies[Theme::Troika] = {melody, bass};
}

void SoundManager::buildMelodies() {
    buildKorobeinikiExtended();
    buildKalinka();
    buildTroika();
}

SoundManager::SoundManager(QObject *parent)
    : QObject(parent), m_muted(false) {

    m_audioFormat.setSampleRate(44100);
    m_audioFormat.setChannelCount(1);
    m_audioFormat.setSampleFormat(QAudioFormat::Int16);

    m_bufferSize = 44100 * 2 * 1 * 50 / 1000;

    auto setupSink = [this](QByteArray &data, AudioStreamer *&streamer,
                            QAudioSink *&sink, float defaultVolume) {
        streamer = new AudioStreamer(&data, false, this);
        sink = new QAudioSink(m_audioFormat, this);
        sink->setBufferSize(m_bufferSize);
        sink->setVolume(defaultVolume);
        sink->start(streamer);
    };

    // Rotate
    m_rotateData = genWave(880, 44100 * 40 / 1000, 44100, 0, 0.5, 0.4);
    setupSink(m_rotateData, m_rotateStreamer, m_rotateSink, 0.3f);

    // Drop
    m_dropData = genWave(150, 44100 * 30 / 1000, 44100, 0, 0.5, 0.3);
    m_dropData.append(genWave(100, 44100 * 70 / 1000, 44100, 0, 0.5, 0.3));
    setupSink(m_dropData, m_dropStreamer, m_dropSink, 0.3f);

    // Line clear
    int qn = 44100 * 60 / 1000;
    m_lineClearData = genWave(523, qn, 44100, 0, 0.5, 0.3);
    m_lineClearData.append(genWave(659, qn, 44100, 0, 0.5, 0.3));
    m_lineClearData.append(genWave(784, qn, 44100, 0, 0.5, 0.3));
    m_lineClearData.append(genWave(1047, qn * 2, 44100, 0, 0.5, 0.3));
    setupSink(m_lineClearData, m_lineClearStreamer, m_lineClearSink, 0.3f);

    // Game over
    int nm = 44100 * 200 / 1000;
    m_gameOverData = genWave(440, nm, 44100, 0, 0.5, 0.25);
    m_gameOverData.append(genWave(370, nm, 44100, 0, 0.5, 0.25));
    m_gameOverData.append(genWave(311, nm, 44100, 0, 0.5, 0.25));
    m_gameOverData.append(genWave(262, nm * 3, 44100, 0, 0.5, 0.25));
    setupSink(m_gameOverData, m_gameOverStreamer, m_gameOverSink, 0.3f);

    m_musicStreamer = new AudioStreamer(&m_musicData, true, this);
    buildMelodies();
    rebuildMusicData();
}

SoundManager::~SoundManager() {
    if (m_musicSink) { m_musicSink->stop(); delete m_musicSink; }
    if (m_musicStreamer) delete m_musicStreamer;
    if (m_rotateSink) { m_rotateSink->stop(); delete m_rotateSink; }
    if (m_rotateStreamer) delete m_rotateStreamer;
    if (m_dropSink) { m_dropSink->stop(); delete m_dropSink; }
    if (m_dropStreamer) delete m_dropStreamer;
    if (m_lineClearSink) { m_lineClearSink->stop(); delete m_lineClearSink; }
    if (m_lineClearStreamer) delete m_lineClearStreamer;
    if (m_gameOverSink) { m_gameOverSink->stop(); delete m_gameOverSink; }
    if (m_gameOverStreamer) delete m_gameOverStreamer;
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
    if (m_musicStreamer) m_musicStreamer->setMuted(muted);
    if (m_musicSink) m_musicSink->setVolume(muted ? 0.0f : 0.2f);
}
void SoundManager::setVolume(float vol) {
    float sfxVol = m_muted ? 0.0f : vol * 0.4f;
    float musicVol = m_muted ? 0.0f : vol * 0.28f;
    m_rotateSink->setVolume(sfxVol);
    m_dropSink->setVolume(sfxVol);
    m_lineClearSink->setVolume(sfxVol);
    m_gameOverSink->setVolume(sfxVol);
    if (m_musicSink) m_musicSink->setVolume(musicVol);
}
void SoundManager::startMusic() {
    m_musicStreamer->seekToStart();
    m_musicStreamer->setPlaying(true);
    if (m_musicSink) m_musicSink->setVolume(m_muted ? 0.0f : 0.2f);
}
void SoundManager::stopMusic() {
    if (m_musicStreamer) m_musicStreamer->setPlaying(false);
}
void SoundManager::pauseMusic() {
    if (m_musicStreamer) m_musicStreamer->setPlaying(false);
    if (m_musicSink) m_musicSink->setVolume(0.0f);
}
void SoundManager::resumeMusic() {
    if (m_musicStreamer) m_musicStreamer->setPlaying(true);
    if (m_musicSink) m_musicSink->setVolume(m_muted ? 0.0f : 0.2f);
}