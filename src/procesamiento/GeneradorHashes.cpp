#include "GeneradorHashes.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <stdexcept>

// Método principal
GeneradorHashes::Resultado GeneradorHashes::generarHashes(
    const std::vector<DetectorPicos::Pico>& picos,
    const Configuracion& config
) {
    std::cout << "Configuración:" << std::endl;
    std::cout << "  Ventana temporal: " << config.ventanaTemporalMs << " ms" << std::endl;
    std::cout << "  Rango: " << config.frecuenciaMinima << "-" << config.frecuenciaMaxima << " Hz" << std::endl;
    
    Resultado resultado;
    
    // 1. Filtrar picos por rango de frecuencia
    auto picosFiltrados = filtrarPicosPorFrecuencia(picos, config.frecuenciaMinima, config.frecuenciaMaxima);
    
    std::cout << "  Picos en rango util: " << picosFiltrados.size() << " de " << picos.size() << std::endl;
    
    if (picosFiltrados.empty()) return resultado;
    
    // 2. Ordenar picos por tiempo (usando DetectorPicos::Pico)
    std::vector<DetectorPicos::Pico> picosOrdenados = picosFiltrados;
    std::sort(picosOrdenados.begin(), picosOrdenados.end(),
              [](const DetectorPicos::Pico& a, const DetectorPicos::Pico& b) { return a.tiempo < b.tiempo; });
    
    std::cout << "  Generando hashes..." << std::endl;
    
    double ventanaTemporalSeg = config.ventanaTemporalMs / 1000.0;
    int hashesGenerados = 0;
    
    // 3. Algoritmo de Emparejamiento (Combinatorial Hashing)
    for (size_t i = 0; i < picosOrdenados.size(); i++) {
        const auto& picoAncla = picosOrdenados[i];
        double tiempoLimite = picoAncla.tiempo + ventanaTemporalSeg;
        int picosEmparejados = 0;
        
        // Buscar objetivos hacia adelante
        for (size_t j = i + 1; j < picosOrdenados.size() && picosEmparejados < config.maxPicosObjetivo; j++) {
            const auto& picoObjetivo = picosOrdenados[j];
            
            // Si el objetivo está muy lejos, paramos
            if (picoObjetivo.tiempo > tiempoLimite) break;
            
            // Si el objetivo está muy cerca (mismo instante), saltamos
            if (picoObjetivo.tiempo <= picoAncla.tiempo) continue;

            double diferenciaTemporalMs = (picoObjetivo.tiempo - picoAncla.tiempo) * 1000.0;
            
            // Generar Hash de 32 bits
            uint32_t hash = codificarHash(
                picoAncla.frecuencia,
                picoObjetivo.frecuencia,
                diferenciaTemporalMs,
                config
            );
            
            resultado.hashes.emplace_back(hash, picoAncla.tiempo, i, j);
            hashesGenerados++;
            picosEmparejados++;
        }
    }
    
    // 4. Estadísticas
    resultado.totalHashesGenerados = hashesGenerados;
    resultado.totalPicosUsados = picosOrdenados.size();
    
    if (!picosOrdenados.empty()) {
        double duracionTotal = picosOrdenados.back().tiempo - picosOrdenados.front().tiempo;
        resultado.densidadHashes = (duracionTotal > 0) ? (double)hashesGenerados / duracionTotal : 0.0;
    }
    
    return resultado;
}

uint32_t GeneradorHashes::codificarHash(
    double frecuenciaAncla,
    double frecuenciaObjetivo,
    double diferenciaTemporal,
    const Configuracion& config
) {
    int f1 = cuantizarFrecuencia(frecuenciaAncla, config.frecuenciaMinima, config.frecuenciaMaxima, config.bitsFrequenciaAncla);
    int f2 = cuantizarFrecuencia(frecuenciaObjetivo, config.frecuenciaMinima, config.frecuenciaMaxima, config.bitsFrequenciaObjetivo);
    int dt = cuantizarTiempo(diferenciaTemporal, config.ventanaTemporalMs, config.bitsDiferenciaTemporal);
    
    // Empaquetado de bits: [F1: 23-31] [F2: 14-22] [DT: 0-13]
    uint32_t hash = 0;
    hash |= ((uint32_t)f1 & 0x1FF) << 23;
    hash |= ((uint32_t)f2 & 0x1FF) << 14;
    hash |= ((uint32_t)dt & 0x3FFF);
    
    return hash;
}

void GeneradorHashes::exportarHashes(const Resultado& resultado, const std::string& nombreArchivo, const std::string& nombreCancion) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) return;
    
    archivo << "Hash_Hex,Timestamp,IndiceAncla,IndiceObjetivo\n";
    for (const auto& h : resultado.hashes) {
        archivo << "0x" << std::hex << std::setw(8) << std::setfill('0') << h.valor << std::dec << ","
                << std::fixed << std::setprecision(4) << h.tiempoAncla << ","
                << h.indiceAncla << "," << h.indiceObjetivo << "\n";
    }
    std::cout << "[INFO] Exportado a " << nombreArchivo << std::endl;
}

// Método auxiliar corregido para usar DetectorPicos::Pico
std::vector<DetectorPicos::Pico> GeneradorHashes::filtrarPicosPorFrecuencia(
    const std::vector<DetectorPicos::Pico>& picos,
    double fMin,
    double fMax
) {
    std::vector<DetectorPicos::Pico> filtrados;
    for (const auto& p : picos) {
        if (p.frecuencia >= fMin && p.frecuencia <= fMax) filtrados.push_back(p);
    }
    return filtrados;
}

int GeneradorHashes::cuantizarFrecuencia(double freq, double min, double max, int bits) {
    double norm = (freq - min) / (max - min);
    norm = std::max(0.0, std::min(1.0, norm));
    int maxVal = (1 << bits) - 1;
    return (int)(norm * maxVal);
}

int GeneradorHashes::cuantizarTiempo(double diff, double max, int bits) {
    double norm = diff / max;
    norm = std::max(0.0, std::min(1.0, norm));
    int maxVal = (1 << bits) - 1;
    return (int)(norm * maxVal);
}

void GeneradorHashes::exportarHashesBinario(const Resultado& res, const std::string& nombre) {}
GeneradorHashes::HashDecodificado GeneradorHashes::decodificarHash(uint32_t h, const Configuracion& c) { return {}; }
