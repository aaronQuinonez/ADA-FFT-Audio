// src/main.cpp
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <algorithm>
#include "audio/LectorAudio.h"
#include "fft/FFT.h"
#include "procesamiento/Espectrograma.h"
#include "procesamiento/DetectorPicos.h"
#include "procesamiento/GeneradorHashes.h" 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(int argc, char* argv[]) {
    try {
        // Verificar argumentos
        if (argc < 2) {
            std::cout << "Uso: " << argv[0] << " <archivo.wav>" << std::endl;
            return 1;
        }
        
        std::string nombreArchivo = argv[1];
        std::cout << "=== SISTEMA DE RECONOCIMIENTO DE AUDIO CON FFT ===" << std::endl;
        std::cout << "Archivo: " << nombreArchivo << std::endl << std::endl;
        
        // ========== FASE 1: Lectura de Audio ==========
        std::cout << "=== FASE 1: LECTURA DE AUDIO ===" << std::endl;
        DatosAudio audio = LectorAudio::leerWAV(nombreArchivo);
        LectorAudio::mostrarInfoAudio(audio);
        
        // Encontrar inicio del audio (saltar silencio)
        std::cout << "\nBuscando inicio del audio (saltando silencio)..." << std::endl;
        int inicioAudio = 0;
        float umbralSilencio = 0.01f;
        
        for (size_t i = 0; i < audio.muestras.size(); i++) {
            if (std::abs(audio.muestras[i]) > umbralSilencio) {
                inicioAudio = i;
                break;
            }
        }
        
        std::cout << "Inicio del audio detectado en muestra: " << inicioAudio << std::endl;
        std::cout << "Tiempo de silencio inicial: " 
                  << (double)inicioAudio / audio.frecuenciaMuestreo << " segundos" << std::endl;
        
        // ========== FASE 2: Generación del Espectrograma ==========
        std::cout << "\n=== FASE 2: GENERACIÓN DEL ESPECTROGRAMA ===" << std::endl;
        
        Espectrograma::Configuracion config;
        config.tamanoVentana = 1024;
        config.solapamiento = 512;  // 50% overlap
        config.inicioAudio = inicioAudio;
        config.aplicarHamming = true;
        
        Espectrograma::Resultado espectrograma = Espectrograma::calcular(audio, config);
        
        // Exportar espectrograma completo
        Espectrograma::exportarCSV(espectrograma, "espectrograma.csv");
        
        // ========== PREPARACIÓN PARA FASE 3: Dividir en Bandas ==========
        std::cout << "\n=== PREPARACIÓN PARA FASE 3: BANDAS DE FRECUENCIA ===" << std::endl;
        
        // Definir bandas de frecuencia (según plan original)
        std::vector<std::pair<double, double>> bandas = {
            {30, 40},      // Banda 1: Graves muy bajos
            {40, 80},      // Banda 2: Graves bajos
            {80, 120},     // Banda 3: Graves
            {120, 180},    // Banda 4: Graves-medios
            {180, 300}     // Banda 5: Medios
        };
        
        auto bandasEspectrograma = Espectrograma::dividirEnBandas(espectrograma, bandas);
        Espectrograma::exportarBandasCSV(bandasEspectrograma, bandas, "bandas_frecuencia.csv");
        
        // ========== FASE 3 y 4: Detección de Picos ==========
        std::cout << "\n=== FASE 3 y 4: DETECCIÓN DE PICOS ESPECTRALES ===" << std::endl;
        
        DetectorPicos::Configuracion configPicos;
        configPicos.umbralMagnitud = 0.1;
        configPicos.vecinosLocales = 3;
        configPicos.picosPorBanda = 5;
        configPicos.usarAdaptativo = true;
        configPicos.percentilUmbral = 75.0;
        
        // Detectar picos en el espectrograma completo
        auto picosCompletos = DetectorPicos::detectarPicos(espectrograma, configPicos);
        DetectorPicos::exportarCSV(picosCompletos, "picos_completos.csv");
        DetectorPicos::exportarConstelacion(picosCompletos, "constelacion.txt");
        
        // Detectar picos en las bandas de frecuencia (Opcional, pero útil para debug)
        auto picosBandas = DetectorPicos::detectarPicosEnBandas(
            bandasEspectrograma, bandas, espectrograma.resolucionTemporal, configPicos
        );
        DetectorPicos::exportarCSV(picosBandas, "picos_bandas.csv");
        
        // Filtrar picos por rango de frecuencia (ejemplo: 100-5000 Hz) para generar hashes limpios
        auto picosFiltrados = DetectorPicos::filtrarPicos(
            picosCompletos.picos, 0.15, 100.0, 5000.0
        );
        std::cout << "\nPicos filtrados para Hashing (100-5000 Hz, mag>0.15): " 
                  << picosFiltrados.size() << std::endl;

        // ========== FASE 5: Generación de Hashes (Fingerprints) ==========
        std::cout << "\n=== FASE 5: GENERACIÓN DE HASHES (FINGERPRINTS) ===" << std::endl;
        
        GeneradorHashes::Configuracion configHashes;
        configHashes.frecuenciaMuestreo = audio.frecuenciaMuestreo;
        configHashes.tamanoFFT = config.tamanoVentana; // Usamos 1024, igual que en espectrograma
        
        // Generar las huellas digitales usando los picos filtrados
        auto fingerprints = GeneradorHashes::generar(picosFiltrados, configHashes);
        
        // Exportar a archivo de texto para inspección
        GeneradorHashes::exportarTXT(fingerprints, "fingerprints.txt");

        if (!fingerprints.empty()) {
            std::cout << "Total de Fingerprints generados: " << fingerprints.size() << std::endl;
            std::cout << "Ejemplo de Hash: " << fingerprints[0].hash 
                      << " (t=" << fingerprints[0].tiempoAncla << "s)" << std::endl;
        } else {
            std::cout << "[ADVERTENCIA] No se generaron fingerprints. Revisa los umbrales de picos." << std::endl;
        }

        // ========== RESUMEN FINAL ==========
        std::cout << "\n=== RESUMEN FINAL DEL SISTEMA ===" << std::endl;
        std::cout << "Fase 1: Audio leído y normalizado" << std::endl;
        std::cout << "Fase 2: Espectrograma generado (" << espectrograma.datos.size() << " ventanas)" << std::endl;
        std::cout << "Fase 3/4: Picos detectados (" << picosFiltrados.size() << " picos útiles)" << std::endl;
        std::cout << "Fase 5: Hashes generados (" << fingerprints.size() << " huellas)" << std::endl;
        
        std::cout << "\nArchivos de salida generados:" << std::endl;
        std::cout << "  1. espectrograma.csv" << std::endl;
        std::cout << "  2. bandas_frecuencia.csv" << std::endl;
        std::cout << "  3. picos_completos.csv" << std::endl;
        std::cout << "  4. constelacion.txt" << std::endl;
        std::cout << "  5. fingerprints.txt  <-- NUEVO (Tu huella digital)" << std::endl;
        
        std::cout << "\n¡El sistema está listo para la Fase 6: Base de Datos y Búsqueda!" << std::endl;
        
    } catch (const std::exception& excepcion) {
        std::cerr << "Error crítico en main: " << excepcion.what() << std::endl;
        return 1;
    }
    
    return 0;
}
