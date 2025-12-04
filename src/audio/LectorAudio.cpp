// src/audio/LectorAudio.cpp
#include "LectorAudio.h"
#include <cstdint>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <iomanip> 

DatosAudio LectorAudio::leerWAV(const std::string& nombreArchivo) {
    DatosAudio datosAudio;
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + nombreArchivo);
    }
    
    std::cout << "[DEBUG] Archivo abierto correctamente" << std::endl;
    
    // Leer encabezado RIFF (12 bytes)
    char encabezadoRIFF[12];
    archivo.read(encabezadoRIFF, 12);
    
    if (!archivo) {
        throw std::runtime_error("Error al leer encabezado RIFF");
    }
    
    // Verificar que sea un archivo RIFF WAV
    if (std::strncmp(encabezadoRIFF, "RIFF", 4) != 0 ||
        std::strncmp(encabezadoRIFF + 8, "WAVE", 4) != 0) {
        throw std::runtime_error("No es un archivo WAV válido");
    }
    
    std::cout << "[DEBUG] Encabezado RIFF válido" << std::endl;
    
    // Variables para almacenar información del formato
    bool formatoEncontrado = false;
    bool datosEncontrados = false;
    int32_t tamanoDatos = 0;
    std::streampos posicionDatos = 0;
    
    // Buscar chunks "fmt " y "data"
    char chunkID[4];
    int32_t chunkSize;
    
    while (archivo.read(chunkID, 4)) {
        if (!archivo.read((char*)&chunkSize, 4)) {
            break;
        }
        
        std::cout << "[DEBUG] Chunk encontrado: " 
                  << chunkID[0] << chunkID[1] << chunkID[2] << chunkID[3] 
                  << " (tamaño: " << chunkSize << " bytes)" << std::endl;
        
        if (std::strncmp(chunkID, "fmt ", 4) == 0) {
            // Leer información del formato
            int16_t formatoAudio;
            int16_t numeroCanales;
            int32_t frecuenciaMuestreo;
            int32_t byteRate;
            int16_t blockAlign;
            int16_t bitsPorMuestra;
            
            archivo.read((char*)&formatoAudio, 2);
            archivo.read((char*)&numeroCanales, 2);
            archivo.read((char*)&frecuenciaMuestreo, 4);
            archivo.read((char*)&byteRate, 4);
            archivo.read((char*)&blockAlign, 2);
            archivo.read((char*)&bitsPorMuestra, 2);
            
            if (!archivo) {
                throw std::runtime_error("Error al leer chunk fmt");
            }
            
            datosAudio.numeroCanales = numeroCanales;
            datosAudio.frecuenciaMuestreo = frecuenciaMuestreo;
            datosAudio.bitsPorMuestra = bitsPorMuestra;
            
            std::cout << "[DEBUG] Formato de audio: " << formatoAudio << std::endl;
            std::cout << "[DEBUG] Block align: " << blockAlign << std::endl;
            std::cout << "[DEBUG] Byte rate: " << byteRate << std::endl;
            
            // Verificar formato PCM
            if (formatoAudio != 1) {
                throw std::runtime_error("Solo se soporta formato PCM (formato=" + std::to_string(formatoAudio) + ")");
            }
            
            // Saltar bytes extra del chunk fmt si los hay
            int bytesLeidos = 16;
            int bytesRestantes = chunkSize - bytesLeidos;
            if (bytesRestantes > 0) {
                archivo.seekg(bytesRestantes, std::ios::cur);
            }
            
            formatoEncontrado = true;
            std::cout << "[DEBUG] Chunk fmt procesado correctamente" << std::endl;
            
        } else if (std::strncmp(chunkID, "data", 4) == 0) {
            tamanoDatos = chunkSize;
            posicionDatos = archivo.tellg();
            datosEncontrados = true;
            
            std::cout << "[DEBUG] Chunk data encontrado en posición " 
                      << posicionDatos << std::endl;
            
            // No leer los datos aún, solo marcar la posición
            archivo.seekg(chunkSize, std::ios::cur);
            
            // Si ya tenemos formato y datos, podemos salir
            if (formatoEncontrado) {
                break;
            }
            
        } else {
            // Saltar este chunk desconocido
            int padding = chunkSize % 2;
            std::cout << "[DEBUG] Saltando chunk desconocido (+" 
                      << chunkSize << " bytes, padding: " << padding << ")" << std::endl;
            
            archivo.seekg(chunkSize + padding, std::ios::cur);
            
            if (!archivo) {
                std::cout << "[DEBUG] Fin de archivo alcanzado" << std::endl;
                break;
            }
        }
    }
    
    if (!formatoEncontrado) {
        throw std::runtime_error("No se encontró el chunk de formato (fmt) en el archivo WAV");
    }
    
    if (!datosEncontrados || tamanoDatos <= 0) {
        throw std::runtime_error("No se encontró el chunk de datos (data) en el archivo WAV");
    }
    
    std::cout << "[DEBUG] Ambos chunks encontrados, leyendo datos de audio..." << std::endl;
    
    // Verificar formato soportado
    if (datosAudio.bitsPorMuestra != 16) {
        throw std::runtime_error("Solo se soportan archivos WAV de 16 bits (se recibió: " + 
                                 std::to_string(datosAudio.bitsPorMuestra) + " bits)");
    }
    
    // Regresar a la posición del chunk data
    archivo.seekg(posicionDatos);
    
    // Calcular número de muestras
    int bytesPorMuestra = datosAudio.bitsPorMuestra / 8;
    int totalMuestras = tamanoDatos / bytesPorMuestra;
    int numeroMuestras = totalMuestras / datosAudio.numeroCanales;
    datosAudio.duracion = (double)numeroMuestras / datosAudio.frecuenciaMuestreo;
    
    std::cout << "[DEBUG] Total de muestras a leer: " << totalMuestras << std::endl;
    std::cout << "[DEBUG] Bytes por muestra: " << bytesPorMuestra << std::endl;
    std::cout << "[DEBUG] Tamaño del chunk data: " << tamanoDatos << " bytes" << std::endl;
    
    // Leer datos de audio (usar int16_t con signo)
    std::vector<int16_t> muestrasCrudas(totalMuestras);
    archivo.read((char*)muestrasCrudas.data(), tamanoDatos);
    
    // Verificar cuántos bytes se leyeron realmente
    std::streamsize bytesLeidos = archivo.gcount();
    std::cout << "[DEBUG] Bytes leídos: " << bytesLeidos << " de " << tamanoDatos << std::endl;
    
    if (bytesLeidos < tamanoDatos) {
        std::cerr << "[ADVERTENCIA] Se leyeron " << bytesLeidos 
                  << " bytes de " << tamanoDatos << " esperados" << std::endl;
        muestrasCrudas.resize(bytesLeidos / bytesPorMuestra);
    }
    
    archivo.close();
    
    std::cout << "[DEBUG] Datos leídos, verificando contenido..." << std::endl;
    
    // *** DIAGNÓSTICO CRÍTICO: Mostrar muestras crudas ***
    std::cout << "[DEBUG] Primeras 20 muestras crudas (int16_t):" << std::endl;
    for (int i = 0; i < std::min(20, (int)muestrasCrudas.size()); i++) {
        std::cout << "  RAW[" << i << "] = " << std::setw(6) << muestrasCrudas[i];
        
        // Mostrar también en hexadecimal para debug
        uint16_t valorHex = *((uint16_t*)&muestrasCrudas[i]);
        std::cout << " (hex: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                  << valorHex << std::dec << std::setfill(' ') << ")" << std::endl;
    }
    
    // Verificar si todas son ceros
    bool todosCeros = true;
    int primeroNoZero = -1;
    for (size_t i = 0; i < std::min((size_t)10000, muestrasCrudas.size()); i++) {
        if (muestrasCrudas[i] != 0) {
            todosCeros = false;
            primeroNoZero = i;
            break;
        }
    }
    
    if (todosCeros) {
        std::cout << "[ERROR CRÍTICO] ¡Las primeras 10000 muestras son todas CERO!" << std::endl;
        std::cout << "[ERROR CRÍTICO] El archivo puede estar corrupto o la lectura falló." << std::endl;
    } else {
        std::cout << "[DEBUG] Primera muestra no-cero encontrada en posición: " << primeroNoZero << std::endl;
    }
    
    std::cout << "[DEBUG] Normalizando muestras..." << std::endl;
    
    // Convertir a formato normalizado [-1.0, 1.0]
    datosAudio.muestras.resize(muestrasCrudas.size());
    for (size_t i = 0; i < muestrasCrudas.size(); i++) {
        datosAudio.muestras[i] = static_cast<float>(muestrasCrudas[i]) / 32768.0f;
    }
    
    // Convertir a mono si es estéreo
    if (datosAudio.numeroCanales == 2) {
        std::cout << "[DEBUG] Convirtiendo estéreo a mono..." << std::endl;
        datosAudio.muestras = estereoAMono(datosAudio.muestras);
        datosAudio.numeroCanales = 1;
    }
    
    std::cout << "[DEBUG] Lectura completada exitosamente" << std::endl;
    
    return datosAudio;
}

std::vector<float> LectorAudio::estereoAMono(const std::vector<float>& estereo) {
    // Validar que el tamaño sea par
    if (estereo.size() % 2 != 0) {
        throw std::runtime_error("El arreglo estéreo debe tener un número par de muestras");
    }
    
    std::vector<float> mono(estereo.size() / 2);
    for (size_t i = 0; i < mono.size(); i++) {
        mono[i] = (estereo[i * 2] + estereo[i * 2 + 1]) / 2.0f;
    }
    
    return mono;
}

void LectorAudio::mostrarInfoAudio(const DatosAudio& audio) {
    std::cout << "\n=== Información del Audio ===" << std::endl;
    std::cout << "Frecuencia de muestreo: " << audio.frecuenciaMuestreo << " Hz" << std::endl;
    std::cout << "Canales: " << audio.numeroCanales << std::endl;
    std::cout << "Bits por muestra: " << audio.bitsPorMuestra << std::endl;
    std::cout << "Duración: " << audio.duracion << " segundos" << std::endl;
    std::cout << "Total de muestras: " << audio.muestras.size() << std::endl;
    std::cout << "\nPrimeras 100 muestras:" << std::endl;
    
    int muestrasAMostrar = std::min(100, (int)audio.muestras.size());
    for (int i = 0; i < muestrasAMostrar; i++) {
        std::cout << "  [" << i << "] = " << audio.muestras[i] << std::endl;
    }
}