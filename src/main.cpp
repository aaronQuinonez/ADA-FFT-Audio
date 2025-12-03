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
        
        // ========== FASE 4: Detección de Picos ==========
        std::cout << "\n=== FASE 4: DETECCIÓN DE PICOS ESPECTRALES ===" << std::endl;
        
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
        
        // Detectar picos en las bandas de frecuencia (Opcional)
        auto picosBandas = DetectorPicos::detectarPicosEnBandas(
            bandasEspectrograma, bandas, espectrograma.resolucionTemporal, configPicos
        );
        DetectorPicos::exportarCSV(picosBandas, "picos_bandas.csv");
        
        // Filtrar picos por rango de frecuencia para generar hashes limpios
        auto picosFiltrados = DetectorPicos::filtrarPicos(
            picosCompletos.picos, 0.15, 100.0, 5000.0
        );
        std::cout << "\nPicos filtrados para Hashing (100-5000 Hz, mag>0.15): " 
                  << picosFiltrados.size() << std::endl;

        // ========== FASE 5: Generación de Hashes (Fingerprints) ==========
        std::cout << "\n=== FASE 5: GENERACIÓN DE HASHES (FINGERPRINTS) ===" << std::endl;
        
        GeneradorHashes::Configuracion configHashes;
        configHashes.ventanaTemporalMs = 2000.0; // Buscar objetivos en los siguientes 2 segundos
        configHashes.maxPicosObjetivo = 5;       // Emparejar cada ancla con máximo 5 objetivos
        configHashes.frecuenciaMinima = 30.0;
        configHashes.frecuenciaMaxima = 5000.0;
        
        // Generar las huellas digitales usando los picos filtrados
        auto resultadoHashes = GeneradorHashes::generarHashes(picosFiltrados, configHashes);
        
        // Exportar a archivo de texto
        GeneradorHashes::exportarHashes(resultadoHashes, "fingerprints.csv", nombreArchivo);

        if (!resultadoHashes.hashes.empty()) {
            std::cout << "Total de Fingerprints generados: " << resultadoHashes.totalHashesGenerados << std::endl;
            std::cout << "Ejemplo de Hash: 0x" << std::hex << resultadoHashes.hashes[0].valor 
                      << std::dec << " (t=" << resultadoHashes.hashes[0].tiempoAncla << "s)" << std::endl;
        } else {
            std::cout << "[ADVERTENCIA] No se generaron fingerprints." << std::endl;
        }

        // ========== RESUMEN FINAL ==========
        std::cout << "\n=============================================" << std::endl;
        std::cout << "       RESUMEN FINAL DEL SISTEMA             " << std::endl;
        std::cout << "=============================================" << std::endl;
        
        std::cout << "[ OK ] Fase 1: Audio leído (" << audio.duracion << " seg, " << audio.frecuenciaMuestreo << " Hz)" << std::endl;
        std::cout << "[ OK ] Fase 2: Espectrograma generado exitosamente" << std::endl;
        std::cout << "[ OK ] Fase 3: División en 5 bandas de frecuencia" << std::endl;
        std::cout << "[ OK ] Fase 4: " << picosFiltrados.size() << " picos detectados (filtrados)" << std::endl;
        std::cout << "[ OK ] Fase 5: " << resultadoHashes.totalHashesGenerados << " fingerprints generados" << std::endl;
        
        std::cout << "\nArchivos de salida generados en carpeta build:" << std::endl;
        std::cout << "  1. espectrograma.csv" << std::endl;
        std::cout << "  2. bandas_frecuencia.csv" << std::endl;
        std::cout << "  3. picos_completos.csv" << std::endl;
        std::cout << "  4. constelacion.txt" << std::endl;
        std::cout << "  5. fingerprints.csv" << std::endl;
        
        std::cout << "\n>> El sistema está listo para la Fase 6: Base de Datos y Búsqueda" << std::endl;
        std::cout << "=============================================" << std::endl;
        
    } catch (const std::exception& excepcion) {
        std::cerr << "Error crítico en main: " << excepcion.what() << std::endl;
        return 1;
    }
    
    return 0;
}