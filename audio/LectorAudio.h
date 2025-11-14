// src/audio/LectorAudio.h
#ifndef LECTOR_AUDIO_H
#define LECTOR_AUDIO_H

#include <string>
#include <vector>
#include <cstdint>

struct DatosAudio {
    std::vector<double> muestras;     // Muestras normalizadas [-1.0, 1.0]
    int frecuenciaMuestreo;           // Frecuencia de muestreo (Hz)
    int numeroCanales;                // Número de canales (1=mono, 2=estéreo)
    int bitsPorMuestra;               // Bits por muestra
    double duracion;                  // Duración en segundos
};

class LectorAudio {
public:
    // Leer archivo WAV y convertir a mono si es necesario
    static DatosAudio leerWAV(const std::string& nombreArchivo);
    
    // Convertir estéreo a mono (promedio de canales)
    static std::vector<double> estereoAMono(const std::vector<double>& estereo);
    
    // Mostrar información del audio
    static void mostrarInfoAudio(const DatosAudio& audio);
};

#endif
