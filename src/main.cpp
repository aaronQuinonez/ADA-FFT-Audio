// src/main.cpp - Sistema Completo de Reconocimiento de Audio
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <filesystem>
#include <chrono>
#include "audio/LectorAudio.h"
#include "fft/FFT.h"
#include "procesamiento/Espectrograma.h"
#include "procesamiento/DetectorPicos.h"
#include "procesamiento/GeneradorHashes.h"
#include "indexacion/BaseDatosHashes.h"
#include "busqueda/BuscadorCanciones.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace fs = std::filesystem;

// MODO 1: DEMO COMPLETO (Fases 1-5)

int modoDemo(const std::string& nombreArchivo) {
    try {
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
        config.solapamiento = 512;
        config.inicioAudio = inicioAudio;
        config.aplicarHamming = true;
        
        Espectrograma::Resultado espectrograma = Espectrograma::calcular(audio, config);
        
        // Exportar espectrograma completo
        Espectrograma::exportarCSV(espectrograma, "espectrograma.csv");
        
        // ========== PREPARACIÓN PARA FASE 3: Dividir en Bandas ==========
        std::cout << "\n=== PREPARACIÓN PARA FASE 3: BANDAS DE FRECUENCIA ===" << std::endl;
        
        std::vector<std::pair<double, double>> bandas = {
            {30, 40}, {40, 80}, {80, 120}, {120, 180}, {180, 300}
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
        
        auto picosCompletos = DetectorPicos::detectarPicos(espectrograma, configPicos);
        DetectorPicos::exportarCSV(picosCompletos, "picos_completos.csv");
        DetectorPicos::exportarConstelacion(picosCompletos, "constelacion.txt");
        
        auto picosBandas = DetectorPicos::detectarPicosEnBandas(
            bandasEspectrograma, bandas, espectrograma.resolucionTemporal, configPicos
        );
        DetectorPicos::exportarCSV(picosBandas, "picos_bandas.csv");
        
        auto picosFiltrados = DetectorPicos::filtrarPicos(
            picosCompletos.picos, 0.15, 100.0, 5000.0
        );
        std::cout << "\nPicos filtrados para Hashing (100-5000 Hz, mag>0.15): " 
                  << picosFiltrados.size() << std::endl;

        // ========== FASE 5: Generación de Hashes (Fingerprints) ==========
        std::cout << "\n=== FASE 5: GENERACIÓN DE HASHES (FINGERPRINTS) ===" << std::endl;
        
        GeneradorHashes::Configuracion configHashes;
        configHashes.ventanaTemporalMs = 2000.0;
        configHashes.maxPicosObjetivo = 5;
        configHashes.frecuenciaMinima = 30.0;
        configHashes.frecuenciaMaxima = 5000.0;
        
        auto resultadoHashes = GeneradorHashes::generarHashes(picosFiltrados, configHashes);
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
        
        std::cout << "\nArchivos de salida generados:" << std::endl;
        std::cout << "  1. espectrograma.csv" << std::endl;
        std::cout << "  2. bandas_frecuencia.csv" << std::endl;
        std::cout << "  3. picos_completos.csv" << std::endl;
        std::cout << "  4. constelacion.txt" << std::endl;
        std::cout << "  5. fingerprints.csv" << std::endl;
        std::cout << "=============================================" << std::endl;
        
    } catch (const std::exception& excepcion) {
        std::cerr << "Error crítico: " << excepcion.what() << std::endl;
        return 1;
    }
    
    return 0;
}

// MODO 2: INDEXAR CANCIONES

bool procesarArchivoParaIndexar(const std::string& rutaArchivo, BaseDatosHashes& baseDatos) {
    try {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Procesando: " << fs::path(rutaArchivo).filename().string() << std::endl;
        std::cout << "========================================" << std::endl;
        
        auto inicio = std::chrono::high_resolution_clock::now();
        
        // 1. Leer audio
        std::cout << "[1/4] Leyendo audio..." << std::endl;
        DatosAudio audio = LectorAudio::leerWAV(rutaArchivo);
        std::cout << "  ✓ Duración: " << audio.duracion << "s, " 
                  << audio.frecuenciaMuestreo << " Hz" << std::endl;
        
        // 2. Generar espectrograma
        std::cout << "[2/4] Generando espectrograma..." << std::endl;
        Espectrograma::Configuracion configEspectro;
        configEspectro.tamanoVentana = 1024;
        configEspectro.solapamiento = 512;
        configEspectro.aplicarHamming = true;
        
        auto espectrograma = Espectrograma::calcular(audio, configEspectro);
        std::cout << "  ✓ Ventanas generadas: " << espectrograma.magnitudes.size() << std::endl;
        
        // 3. Detectar picos
        std::cout << "[3/4] Detectando picos..." << std::endl;
        DetectorPicos::Configuracion configPicos;
        configPicos.umbralMagnitud = 0.1;
        configPicos.vecinosLocales = 3;
        configPicos.picosPorBanda = 5;
        configPicos.usarAdaptativo = true;
        configPicos.percentilUmbral = 75.0;
        
        auto resultadoPicos = DetectorPicos::detectarPicos(espectrograma, configPicos);
        auto picosFiltrados = DetectorPicos::filtrarPicos(
            resultadoPicos.picos, 0.15, 100.0, 5000.0
        );
        std::cout << "  ✓ Picos detectados: " << picosFiltrados.size() << std::endl;
        
        // 4. Generar hashes
        std::cout << "[4/4] Generando fingerprints..." << std::endl;
        GeneradorHashes::Configuracion configHashes;
        configHashes.ventanaTemporalMs = 2000.0;
        configHashes.maxPicosObjetivo = 5;
        configHashes.frecuenciaMinima = 30.0;
        configHashes.frecuenciaMaxima = 5000.0;
        
        auto resultadoHashes = GeneradorHashes::generarHashes(picosFiltrados, configHashes);
        std::cout << "  ✓ Fingerprints generados: " << resultadoHashes.totalHashesGenerados << std::endl;
        
        // 5. Agregar a la base de datos
        std::string nombreCancion = fs::path(rutaArchivo).filename().string();
        baseDatos.agregarCancion(nombreCancion, rutaArchivo, audio.duracion, resultadoHashes.hashes);
        
        auto fin = std::chrono::high_resolution_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::seconds>(fin - inicio);
        
        std::cout << "Completado en " << duracion.count() << " segundos" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error al procesar archivo: " << e.what() << std::endl;
        return false;
    }
}

int modoIndexar(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "\nUso: " << argv[0] << " --indexar <base_de_datos> <archivo1.wav> [archivo2.wav] ..." << std::endl;
        std::cout << "\nEjemplos:" << std::endl;
        std::cout << "  " << argv[0] << " --indexar mi_database cancion1.wav cancion2.wav" << std::endl;
        return 1;
    }
    
    std::string nombreDB = argv[2];
    std::vector<std::string> archivos;
    
    for (int i = 3; i < argc; i++) {
        archivos.push_back(argv[i]);
    }
    
    std::cout << "============================================" << std::endl;
    std::cout << "   SISTEMA DE INDEXACIÓN DE CANCIONES      " << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "\nBase de datos: " << nombreDB << std::endl;
    std::cout << "Archivos a procesar: " << archivos.size() << std::endl;
    
    BaseDatosHashes baseDatos;
    
    // Cargar DB existente si existe
    if (fs::exists(nombreDB + "_metadata.txt")) {
        std::cout << "\n📂 Base de datos existente detectada, cargando..." << std::endl;
        if (baseDatos.cargar(nombreDB)) {
            baseDatos.mostrarInfo();
        }
    }
    
    // Procesar archivos
    auto inicioTotal = std::chrono::high_resolution_clock::now();
    int exitosos = 0, fallidos = 0;
    
    for (const auto& archivo : archivos) {
        if (procesarArchivoParaIndexar(archivo, baseDatos)) {
            exitosos++;
        } else {
            fallidos++;
        }
    }
    
    auto finTotal = std::chrono::high_resolution_clock::now();
    auto duracionTotal = std::chrono::duration_cast<std::chrono::seconds>(finTotal - inicioTotal);
    
    // Guardar base de datos
    std::cout << "\n========================================" << std::endl;
    std::cout << "Guardando base de datos..." << std::endl;
    
    if (baseDatos.guardar(nombreDB)) {
        std::cout << "Base de datos guardada exitosamente" << std::endl;
    } else {
        std::cerr << "Error al guardar la base de datos" << std::endl;
        return 1;
    }
    
    // Resumen
    std::cout << "\n========================================" << std::endl;
    std::cout << "         RESUMEN DE INDEXACIÓN          " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "✓ Archivos procesados: " << exitosos << std::endl;
    if (fallidos > 0) std::cout << "Errores: " << fallidos << std::endl;
    std::cout << "⏱  Tiempo total: " << duracionTotal.count() << " segundos" << std::endl;
    
    baseDatos.mostrarInfo();
    baseDatos.listarCanciones();
    
    std::cout << "\n>> Usa '" << argv[0] << " --buscar " << nombreDB << " query.wav' para buscar canciones" << std::endl;
    return 0;
}

// MODO 3: BUSCAR CANCIÓN

std::vector<GeneradorHashes::Hash> procesarQuery(const std::string& rutaArchivo) {
    std::cout << "\n=== Procesando Audio Query ===" << std::endl;
    
    DatosAudio audio = LectorAudio::leerWAV(rutaArchivo);
    std::cout << "[1/4] Audio leído: " << audio.duracion << "s" << std::endl;
    
    Espectrograma::Configuracion configEspectro;
    configEspectro.tamanoVentana = 1024;
    configEspectro.solapamiento = 512;
    configEspectro.aplicarHamming = true;
    
    auto espectrograma = Espectrograma::calcular(audio, configEspectro);
    std::cout << "[2/4] Espectrograma: " << espectrograma.magnitudes.size() << " ventanas" << std::endl;
    
    DetectorPicos::Configuracion configPicos;
    configPicos.umbralMagnitud = 0.1;
    configPicos.vecinosLocales = 3;
    configPicos.picosPorBanda = 5;
    configPicos.usarAdaptativo = true;
    
    auto resultadoPicos = DetectorPicos::detectarPicos(espectrograma, configPicos);
    auto picosFiltrados = DetectorPicos::filtrarPicos(resultadoPicos.picos, 0.15, 100.0, 5000.0);
    std::cout << "[3/4] Picos detectados: " << picosFiltrados.size() << std::endl;
    
    GeneradorHashes::Configuracion configHashes;
    configHashes.ventanaTemporalMs = 2000.0;
    configHashes.maxPicosObjetivo = 5;
    
    auto resultadoHashes = GeneradorHashes::generarHashes(picosFiltrados, configHashes);
    std::cout << "[4/4] Fingerprints: " << resultadoHashes.totalHashesGenerados << std::endl;
    
    return resultadoHashes.hashes;
}

int modoBuscar(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "\nUso: " << argv[0] << " --buscar <base_de_datos> <query.wav> [--top N]" << std::endl;
        std::cout << "\nEjemplos:" << std::endl;
        std::cout << "  " << argv[0] << " --buscar mi_database query.wav" << std::endl;
        std::cout << "  " << argv[0] << " --buscar mi_database query.wav --top 5" << std::endl;
        return 1;
    }
    
    std::string nombreDB = argv[2];
    std::string archivoQuery = argv[3];
    int topN = 1;
    
    // Parsear --top
    for (int i = 4; i < argc; i++) {
        if (std::string(argv[i]) == "--top" && i + 1 < argc) {
            topN = std::stoi(argv[++i]);
        }
    }
    
    std::cout << "============================================" << std::endl;
    std::cout << "     SISTEMA DE BÚSQUEDA DE CANCIONES      " << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "\nBase de datos: " << nombreDB << std::endl;
    std::cout << "Query: " << archivoQuery << std::endl;
    
    try {
        // Cargar base de datos
        std::cout << "\n=== Cargando Base de Datos ===" << std::endl;
        BaseDatosHashes baseDatos;
        
        if (!baseDatos.cargar(nombreDB)) {
            std::cerr << "Error: No se pudo cargar la base de datos" << std::endl;
            return 1;
        }
        
        baseDatos.mostrarInfo();
        
        // Procesar query
        auto hashesQuery = procesarQuery(archivoQuery);
        
        if (hashesQuery.empty()) {
            std::cerr << "Error: No se generaron fingerprints del query" << std::endl;
            return 1;
        }
        
        // Buscar
        BuscadorCanciones::Configuracion configBuscador;
        configBuscador.mostrarProgreso = true;
        BuscadorCanciones buscador(baseDatos, configBuscador);
        
        if (topN == 1) {
            auto resultado = buscador.buscar(hashesQuery);
            BuscadorCanciones::mostrarResultado(resultado);
        } else {
            auto resultados = buscador.buscarTopN(hashesQuery, topN);
            
            std::cout << "\n========================================" << std::endl;
            std::cout << "       TOP " << topN << " RESULTADOS" << std::endl;
            std::cout << "========================================" << std::endl;
            
            if (resultados.empty()) {
                std::cout << "No se encontraron coincidencias" << std::endl;
            } else {
                for (size_t i = 0; i < resultados.size(); i++) {
                    std::cout << "\n#" << (i + 1) << " - " << resultados[i].nombreCancion << std::endl;
                    std::cout << "    Confianza: " << std::fixed << std::setprecision(1) 
                              << resultados[i].confianza << "%" << std::endl;
                    std::cout << "    Offset: " << resultados[i].offsetSegundos << "s" << std::endl;
                }
            }
            std::cout << "========================================" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

// ============================================
// MAIN PRINCIPAL CON MENÚ
// ============================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "============================================" << std::endl;
        std::cout << "   SISTEMA DE RECONOCIMIENTO DE AUDIO      " << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << "\nModos de uso:" << std::endl;
        std::cout << "\n1. DEMO COMPLETO (Fases 1-5):" << std::endl;
        std::cout << "   " << argv[0] << " <archivo.wav>" << std::endl;
        std::cout << "   Ejecuta análisis completo y genera archivos CSV" << std::endl;
        
        std::cout << "\n2. INDEXAR CANCIONES:" << std::endl;
        std::cout << "   " << argv[0] << " --indexar <database> <cancion1.wav> [cancion2.wav] ..." << std::endl;
        std::cout << "   Crea base de datos con fingerprints de canciones" << std::endl;
        
        std::cout << "\n3. BUSCAR/IDENTIFICAR:" << std::endl;
        std::cout << "   " << argv[0] << " --buscar <database> <query.wav> [--top N]" << std::endl;
        std::cout << "   Identifica una canción desde un fragmento de audio" << std::endl;
        
        std::cout << "\n============================================" << std::endl;
        return 1;
    }
    
    std::string primerArg = argv[1];
    
    // Determinar modo de operación
    if (primerArg == "--indexar") {
        return modoIndexar(argc, argv);
    } 
    else if (primerArg == "--buscar") {
        return modoBuscar(argc, argv);
    } 
    else {
        // Modo demo por defecto
        return modoDemo(primerArg);
    }
}