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
        
        // ========== FASE 3: Detección de Picos ==========
        std::cout << "\n=== FASE 3: DETECCIÓN DE PICOS ESPECTRALES ===" << std::endl;
        
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
        
        // Detectar picos en las bandas de frecuencia
        auto picosBandas = DetectorPicos::detectarPicosEnBandas(
            bandasEspectrograma, bandas, espectrograma.resolucionTemporal, configPicos
        );
        DetectorPicos::exportarCSV(picosBandas, "picos_bandas.csv");
        
        // Filtrar picos por rango de frecuencia (ejemplo: 100-5000 Hz)
        auto picosFiltrados = DetectorPicos::filtrarPicos(
            picosCompletos.picos, 0.15, 100.0, 5000.0
        );
        std::cout << "\nPicos filtrados (100-5000 Hz, mag>0.15): " 
                  << picosFiltrados.size() << std::endl;
        
        // Mostrar estadísticas finales
        std::cout << "\n=== RESUMEN FINAL ===" << std::endl;
        std::cout << "✓ Fase 1 completada: Audio leído y procesado" << std::endl;
        std::cout << "✓ Fase 2 completada: Espectrograma generado" << std::endl;
        std::cout << "✓ Fase 3 completada: Picos detectados" << std::endl;
        std::cout << "\nArchivos generados:" << std::endl;
        std::cout << "  1. espectrograma.csv - Espectrograma completo" << std::endl;
        std::cout << "  2. bandas_frecuencia.csv - Bandas de frecuencia" << std::endl;
        std::cout << "  3. picos_completos.csv - Picos detectados (espectrograma completo)" << std::endl;
        std::cout << "  4. picos_bandas.csv - Picos detectados (por bandas)" << std::endl;
        std::cout << "  5. constelacion.txt - Constelación de picos" << std::endl;
        std::cout << "\n¡Listo para continuar con Fase 4: Generación de Hashes!" << std::endl;
    } catch (const std::exception& excepcion) {
        std::cerr << "Error: " << excepcion.what() << std::endl;
        return 1;
    }
    
    return 0;
}