#include "GeneradorHashes.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cmath>

GeneradorHashes::Resultado GeneradorHashes::generarHashes(
    const std::vector<DetectorPicos::Pico>& picos,
    const Configuracion& config
) {
    std::cout << "\n=== FASE 4: GENERACIÓN DE HASHES (FINGERPRINTS) ===" << std::endl;
    std::cout << "Configuración:" << std::endl;
    std::cout << "  Ventana temporal: " << config.ventanaTemporalMs << " ms" << std::endl;
    std::cout << "  Máximo picos objetivo por ancla: " << config.maxPicosObjetivo << std::endl;
    std::cout << "  Rango de frecuencia: " << config.frecuenciaMinima << "-" 
              << config.frecuenciaMaxima << " Hz" << std::endl;
    std::cout << "  Bits - Frecuencia ancla: " << config.bitsFrequenciaAncla << std::endl;
    std::cout << "  Bits - Frecuencia objetivo: " << config.bitsFrequenciaObjetivo << std::endl;
    std::cout << "  Bits - Diferencia temporal: " << config.bitsDiferenciaTemporal << std::endl;
    
    Resultado resultado;
    
    // Filtrar picos por rango de frecuencia
    auto picosFiltrados = filtrarPicosPorFrecuencia(
        picos, config.frecuenciaMinima, config.frecuenciaMaxima
    );
    
    std::cout << "\nPicos filtrados: " << picosFiltrados.size() 
              << " de " << picos.size() << " originales" << std::endl;
    
    if (picosFiltrados.empty()) {
        std::cout << "No hay picos para procesar." << std::endl;
        return resultado;
    }
    
    // Ordenar picos por tiempo
    std::vector<DetectorPicos::Pico> picosOrdenados = picosFiltrados;
    std::sort(picosOrdenados.begin(), picosOrdenados.end(),
              [](const auto& a, const auto& b) { return a.tiempo < b.tiempo; });
    
    std::cout << "Generando hashes..." << std::endl;
    
    double ventanaTemporalSeg = config.ventanaTemporalMs / 1000.0;
    int hashesGenerados = 0;
    int porcentajeAnterior = -1;
    
    // Para cada pico ancla
    for (size_t i = 0; i < picosOrdenados.size(); i++) {
        // Mostrar progreso
        int porcentaje = (100 * i) / picosOrdenados.size();
        if (porcentaje != porcentajeAnterior && porcentaje % 10 == 0) {
            std::cout << "  Progreso: " << porcentaje << "% (" 
                      << hashesGenerados << " hashes)" << std::endl;
            porcentajeAnterior = porcentaje;
        }
        
        const auto& picoAncla = picosOrdenados[i];
        double tiempoLimite = picoAncla.tiempo + ventanaTemporalSeg;
        int picosEmparejados = 0;
        
        // Buscar picos objetivo dentro de la ventana temporal
        for (size_t j = i + 1; j < picosOrdenados.size() && 
             picosEmparejados < config.maxPicosObjetivo; j++) {
            
            const auto& picoObjetivo = picosOrdenados[j];
            
            // Si excedemos la ventana temporal, detener búsqueda
            if (picoObjetivo.tiempo > tiempoLimite) {
                break;
            }
            
            // Calcular diferencia temporal en ms
            double diferenciaTemporal = (picoObjetivo.tiempo - picoAncla.tiempo) * 1000.0;
            
            // Generar hash para este par
            uint32_t hash = codificarHash(
                picoAncla.frecuencia,
                picoObjetivo.frecuencia,
                diferenciaTemporal,
                config
            );
            
            // Almacenar hash con timestamp del ancla
            resultado.hashes.emplace_back(hash, picoAncla.tiempo, i, j);
            
            hashesGenerados++;
            picosEmparejados++;
        }
    }
    
    resultado.totalHashesGenerados = hashesGenerados;
    resultado.totalPicosUsados = picosOrdenados.size();
    
    // Calcular densidad de hashes
    if (!picosOrdenados.empty()) {
        double duracionTotal = picosOrdenados.back().tiempo - picosOrdenados.front().tiempo;
        resultado.densidadHashes = (duracionTotal > 0) ? 
            (double)hashesGenerados / duracionTotal : 0.0;
    }
    
    std::cout << "  Progreso: 100%" << std::endl;
    std::cout << "\n✓ Generación de hashes completada" << std::endl;
    std::cout << "  Total de hashes generados: " << resultado.totalHashesGenerados << std::endl;
    std::cout << "  Picos utilizados: " << resultado.totalPicosUsados << std::endl;
    std::cout << "  Densidad de hashes: " << std::fixed << std::setprecision(2) 
              << resultado.densidadHashes << " hashes/segundo" << std::endl;
    std::cout << "  Promedio de hashes por pico: " << std::fixed << std::setprecision(2)
              << (double)resultado.totalHashesGenerados / resultado.totalPicosUsados << std::endl;
    
    return resultado;
}

uint32_t GeneradorHashes::codificarHash(
    double frecuenciaAncla,
    double frecuenciaObjetivo,
    double diferenciaTemporal,
    const Configuracion& config
) {
    // Cuantizar valores
    int f1 = cuantizarFrecuencia(
        frecuenciaAncla, 
        config.frecuenciaMinima, 
        config.frecuenciaMaxima,
        config.bitsFrequenciaAncla
    );
    
    int f2 = cuantizarFrecuencia(
        frecuenciaObjetivo, 
        config.frecuenciaMinima, 
        config.frecuenciaMaxima,
        config.bitsFrequenciaObjetivo
    );
    
    int dt = cuantizarTiempo(
        diferenciaTemporal,
        config.ventanaTemporalMs,
        config.bitsDiferenciaTemporal
    );
    
    // Ensamblar hash de 32 bits: [f1(9bits)][f2(9bits)][dt(14bits)]
    uint32_t hash = 0;
    hash |= ((uint32_t)f1 & 0x1FF) << 23;  // 9 bits para f1 en posiciones 23-31
    hash |= ((uint32_t)f2 & 0x1FF) << 14;  // 9 bits para f2 en posiciones 14-22
    hash |= ((uint32_t)dt & 0x3FFF);        // 14 bits para dt en posiciones 0-13
    
    return hash;
}

GeneradorHashes::HashDecodificado GeneradorHashes::decodificarHash(
    uint32_t hash, 
    const Configuracion& config
) {
    HashDecodificado resultado;
    
    // Extraer componentes
    resultado.frecuenciaAncla = (hash >> 23) & 0x1FF;
    resultado.frecuenciaObjetivo = (hash >> 14) & 0x1FF;
    resultado.diferenciaTemporal = hash & 0x3FFF;
    
    return resultado;
}

void GeneradorHashes::exportarHashes(
    const Resultado& resultado,
    const std::string& nombreArchivo,
    const std::string& nombreCancion
) {
    std::cout << "\nExportando hashes a archivo de texto..." << std::endl;
    
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo: " + nombreArchivo);
    }
    
    // Escribir encabezado
    archivo << "# Fingerprints de Audio - Fase 4" << std::endl;
    archivo << "# Canción: " << nombreCancion << std::endl;
    archivo << "# Total de hashes: " << resultado.totalHashesGenerados << std::endl;
    archivo << "# Formato: hash(hex), timestamp(s), indice_ancla, indice_objetivo" << std::endl;
    archivo << std::endl;
    
    // Escribir encabezado CSV
    archivo << "Hash,Timestamp,IndiceAncla,IndiceObjetivo" << std::endl;
    
    // Escribir hashes
    for (const auto& hash : resultado.hashes) {
        archivo << "0x" << std::hex << std::setw(8) << std::setfill('0') 
                << hash.valor << std::dec << ","
                << std::fixed << std::setprecision(6) << hash.tiempoAncla << ","
                << hash.indiceAncla << ","
                << hash.indiceObjetivo << std::endl;
    }
    
    archivo.close();
    std::cout << "✓ Hashes exportados a '" << nombreArchivo << "'" << std::endl;
}

void GeneradorHashes::exportarHashesBinario(
    const Resultado& resultado,
    const std::string& nombreArchivo
) {
    std::cout << "\nExportando hashes en formato binario..." << std::endl;
    
    std::ofstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo: " + nombreArchivo);
    }
    
    // Escribir número de hashes
    uint32_t numHashes = resultado.hashes.size();
    archivo.write((char*)&numHashes, sizeof(uint32_t));
    
    // Escribir cada hash
    for (const auto& hash : resultado.hashes) {
        archivo.write((char*)&hash.valor, sizeof(uint32_t));
        archivo.write((char*)&hash.tiempoAncla, sizeof(double));
    }
    
    archivo.close();
    std::cout << "✓ Hashes exportados en formato binario a '" << nombreArchivo << "'" << std::endl;
}

std::vector<DetectorPicos::Pico> GeneradorHashes::filtrarPicosPorFrecuencia(
    const std::vector<DetectorPicos::Pico>& picos,
    double frecuenciaMin,
    double frecuenciaMax
) {
    std::vector<DetectorPicos::Pico> picosFiltrados;
    
    for (const auto& pico : picos) {
        if (pico.frecuencia >= frecuenciaMin && pico.frecuencia <= frecuenciaMax) {
            picosFiltrados.push_back(pico);
        }
    }
    
    return picosFiltrados;
}

int GeneradorHashes::cuantizarFrecuencia(
    double frecuencia,
    double frecuenciaMin,
    double frecuenciaMax,
    int numBits
) {
    // Normalizar frecuencia al rango [0, 1]
    double normalizada = (frecuencia - frecuenciaMin) / (frecuenciaMax - frecuenciaMin);
    normalizada = std::max(0.0, std::min(1.0, normalizada));
    
    // Cuantizar a numBits
    int maxValor = (1 << numBits) - 1;  // 2^numBits - 1
    int valorCuantizado = (int)(normalizada * maxValor);
    
    return std::max(0, std::min(maxValor, valorCuantizado));
}

int GeneradorHashes::cuantizarTiempo(
    double diferenciaMs,
    double ventanaMaxMs,
    int numBits
) {
    // Normalizar tiempo al rango [0, 1]
    double normalizado = diferenciaMs / ventanaMaxMs;
    normalizado = std::max(0.0, std::min(1.0, normalizado));
    
    // Cuantizar a numBits
    int maxValor = (1 << numBits) - 1;  // 2^numBits - 1
    int valorCuantizado = (int)(normalizado * maxValor);
    
    return std::max(0, std::min(maxValor, valorCuantizado));
}
