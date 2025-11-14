// src/audio/AudioReader.cpp
#include "AudioReader.h"
#include <fstream>
#include <iostream>
#include <cstring>

AudioData AudioReader::readWAV(const std::string& filename) {
    AudioData audioData;
    std::ifstream file(filename, std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    }
    
    // Leer encabezado WAV (44 bytes estándar)
    char header[44];
    file.read(header, 44);
    
    // Verificar que sea un archivo RIFF WAV
    if (std::strncmp(header, "RIFF", 4) != 0 || 
        std::strncmp(header + 8, "WAVE", 4) != 0) {
        throw std::runtime_error("No es un archivo WAV válido");
    }
    
    // Extraer información del encabezado
    audioData.numChannels = *(int16_t*)(header + 22);
    audioData.sampleRate = *(int32_t*)(header + 24);
    audioData.bitsPerSample = *(int16_t*)(header + 34);
    int dataSize = *(int32_t*)(header + 40);
    
    // Calcular número de muestras
    int bytesPerSample = audioData.bitsPerSample / 8;
    int numSamples = dataSize / (bytesPerSample * audioData.numChannels);
    audioData.duration = (double)numSamples / audioData.sampleRate;
    
    // Leer datos de audio
    std::vector<int16_t> rawSamples(numSamples * audioData.numChannels);
    file.read((char*)rawSamples.data(), dataSize);
    file.close();
    
    // Convertir a formato normalizado [-1.0, 1.0]
    audioData.samples.resize(numSamples * audioData.numChannels);
    for (int i = 0; i < rawSamples.size(); i++) {
        audioData.samples[i] = rawSamples[i] / 32768.0;
    }
    
    // Convertir a mono si es estéreo
    if (audioData.numChannels == 2) {
        audioData.samples = stereoToMono(audioData.samples);
        audioData.numChannels = 1;
    }
    
    return audioData;
}

std::vector<double> AudioReader::stereoToMono(const std::vector<double>& stereo) {
    std::vector<double> mono(stereo.size() / 2);
    for (size_t i = 0; i < mono.size(); i++) {
        mono[i] = (stereo[i * 2] + stereo[i * 2 + 1]) / 2.0;
    }
    return mono;
}

void AudioReader::printAudioInfo(const AudioData& audio) {
    std::cout << "=== Información del Audio ===" << std::endl;
    std::cout << "Frecuencia de muestreo: " << audio.sampleRate << " Hz" << std::endl;
    std::cout << "Canales: " << audio.numChannels << std::endl;
    std::cout << "Bits por muestra: " << audio.bitsPerSample << std::endl;
    std::cout << "Duración: " << audio.duration << " segundos" << std::endl;
    std::cout << "Total de muestras: " << audio.samples.size() << std::endl;
    std::cout << "Primeras 10 muestras:" << std::endl;
    for (int i = 0; i < std::min(10, (int)audio.samples.size()); i++) {
        std::cout << "  [" << i << "] = " << audio.samples[i] << std::endl;
    }
}
