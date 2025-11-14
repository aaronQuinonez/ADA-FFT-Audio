// src/audio/LectorAudio.cpp
#include "LectorAudio.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>

DatosAudio LectorAudio::leerWAV(const std::string& nombreArchivo) {
    DatosAudio datosAudio;
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + nombreArchivo);
    }
    
    // Leer encabezado WAV (44 bytes estándar)
    char encabezado[44];
    archivo.read(encabezado, 44);
    
    // Verificar que sea un archivo RIFF WAV
    if (std::strncmp(encabezado, "RIFF", 4) != 0 || 
        std::strncmp(encabezado + 8, "WAVE", 4) != 0) {
        throw std::runtime_error("No es un archivo WAV válido");
    }
    
    // Extraer información del encabezado
    datosAudio.numeroCanales = *(int16_t*)(encabezado + 22);
    datosAudio.frecuenciaMuestreo = *(int32_t*)(encabezado + 24);
    datosAudio.bitsPorMuestra = *(int16_t*)(encabezado + 34);
    int tamanoDatos = *(int32_t*)(encabezado + 40);
    
    // Calcular número de muestras
    int bytesPorMuestra = datosAudio.bitsPorMuestra / 8;
    int numeroMuestras = tamanoDatos / (bytesPorMuestra * datosAudio.numeroCanales);
    datosAudio.duracion = (double)numeroMuestras / datosAudio.frecuenciaMuestreo;
    
    // Leer datos de audio
    std::vector<int16_t> muestrasCrudas(numeroMuestras * datosAudio.numeroCanales);
    archivo.read((char*)muestrasCrudas.data(), tamanoDatos);
    archivo.close();
    
    // Convertir a formato normalizado [-1.0, 1.0]
    datosAudio.muestras.resize(numeroMuestras * datosAudio.numeroCanales);
    for (size_t i = 0; i < muestrasCrudas.size(); i++) {
        datosAudio.muestras[i] = muestrasCrudas[i] / 32768.0;
    }
    
    // Convertir a mono si es estéreo
    if (datosAudio.numeroCanales == 2) {
        datosAudio.muestras = estereoAMono(datosAudio.muestras);
        datosAudio.numeroCanales = 1;
    }
    
    return datosAudio;
}

std::vector<double> LectorAudio::estereoAMono(const std::vector<double>& estereo) {
    std::vector<double> mono(estereo.size() / 2);
    for (size_t i = 0; i < mono.size(); i++) {
        mono[i] = (estereo[i * 2] + estereo[i * 2 + 1]) / 2.0;
    }
    return mono;
}

void LectorAudio::mostrarInfoAudio(const DatosAudio& audio) {
    std::cout << "=== Información del Audio ===" << std::endl;
    std::cout << "Frecuencia de muestreo: " << audio.frecuenciaMuestreo << " Hz" << std::endl;
    std::cout << "Canales: " << audio.numeroCanales << std::endl;
    std::cout << "Bits por muestra: " << audio.bitsPorMuestra << std::endl;
    std::cout << "Duración: " << audio.duracion << " segundos" << std::endl;
    std::cout << "Total de muestras: " << audio.muestras.size() << std::endl;
    std::cout << "Primeras 10 muestras:" << std::endl;
    for (int i = 0; i < std::min(10, (int)audio.muestras.size()); i++) {
        std::cout << "  [" << i << "] = " << audio.muestras[i] << std::endl;
    }
}
