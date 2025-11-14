// src/audio/AudioReader.h
#ifndef AUDIO_READER_H
#define AUDIO_READER_H

#include <string>
#include <vector>
#include <cstdint>

struct AudioData {
    std::vector<double> samples;  // Muestras normalizadas [-1.0, 1.0]
    int sampleRate;               // Frecuencia de muestreo (Hz)
    int numChannels;              // Número de canales (1=mono, 2=estéreo)
    int bitsPerSample;            // Bits por muestra
    double duration;              // Duración en segundos
};

class AudioReader {
public:
    // Leer archivo WAV y convertir a mono si es necesario
    static AudioData readWAV(const std::string& filename);
    
    // Convertir estéreo a mono (promedio de canales)
    static std::vector<double> stereoToMono(const std::vector<double>& stereo);
    
    // Mostrar información del audio
    static void printAudioInfo(const AudioData& audio);
};

#endif
