// src/busqueda/SistemaVotacion.cpp
#include "SistemaVotacion.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>

SistemaVotacion::SistemaVotacion(const Configuracion& config) : config_(config) {}

void SistemaVotacion::registrarVoto(int idCancion, double timestampQuery, double timestampDB) {
    // Calcular offset temporal
    double offset = timestampDB - timestampQuery;
    
    // Redondear offset para agrupar votos similares
    double offsetRedondeado = redondearOffset(offset);
    
    // Incrementar voto en el histograma
    histogramas_[idCancion][offsetRedondeado]++;
    contadores_[idCancion]++;
}

std::vector<SistemaVotacion::ResultadoCancion> SistemaVotacion::obtenerResultados(
    int totalHashesQuery
) const {
    std::vector<ResultadoCancion> resultados;
    
    for (const auto& par : histogramas_) {
        int idCancion = par.first;
        const auto& histograma = par.second;
        
        // Encontrar el offset con más votos
        double mejorOffset = 0.0;
        int maxVotos = 0;
        
        for (const auto& offsetVotos : histograma) {
            if (offsetVotos.second > maxVotos) {
                maxVotos = offsetVotos.second;
                mejorOffset = offsetVotos.first;
            }
        }
        
        // Verificar umbral mínimo
        if (maxVotos < config_.minimoCoincidencias) {
            continue;
        }
        
        // Crear resultado
        ResultadoCancion resultado;
        resultado.idCancion = idCancion;
        resultado.totalCoincidencias = contadores_.at(idCancion);
        resultado.offsetMejor = mejorOffset;
        resultado.votosMejor = maxVotos;
        resultado.confianza = calcularConfianza(maxVotos, totalHashesQuery);
        
        // Verificar umbral de confianza
        if (resultado.confianza >= config_.umbralConfianza) {
            resultados.push_back(resultado);
        }
    }
    
    // Ordenar por confianza descendente
    std::sort(resultados.begin(), resultados.end(),
              [](const ResultadoCancion& a, const ResultadoCancion& b) {
                  return a.confianza > b.confianza;
              });
    
    return resultados;
}

SistemaVotacion::ResultadoCancion SistemaVotacion::obtenerMejorResultado(
    int totalHashesQuery
) const {
    auto resultados = obtenerResultados(totalHashesQuery);
    if (!resultados.empty()) {
        return resultados[0];
    }
    return ResultadoCancion();
}

void SistemaVotacion::limpiar() {
    histogramas_.clear();
    contadores_.clear();
}

void SistemaVotacion::mostrarEstadisticas() const {
    std::cout << "\n=== Estadísticas de Votación ===" << std::endl;
    std::cout << "Canciones con coincidencias: " << histogramas_.size() << std::endl;
    
    for (const auto& par : contadores_) {
        int idCancion = par.first;
        int totalCoincidencias = par.second;
        
        std::cout << "\nCanción ID=" << idCancion << ":" << std::endl;
        std::cout << "  Total de coincidencias: " << totalCoincidencias << std::endl;
        
        const auto& histograma = histogramas_.at(idCancion);
        std::cout << "  Offsets diferentes: " << histograma.size() << std::endl;
        
        // Mostrar top 3 offsets
        std::vector<std::pair<double, int>> offsetsOrdenados;
        for (const auto& ov : histograma) {
            offsetsOrdenados.push_back({ov.first, ov.second});
        }
        std::sort(offsetsOrdenados.begin(), offsetsOrdenados.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::cout << "  Top offsets:" << std::endl;
        for (size_t i = 0; i < std::min((size_t)3, offsetsOrdenados.size()); i++) {
            std::cout << "    " << std::fixed << std::setprecision(3) 
                      << offsetsOrdenados[i].first << "s -> " 
                      << offsetsOrdenados[i].second << " votos" << std::endl;
        }
    }
}

double SistemaVotacion::redondearOffset(double offset) const {
    // Redondear al múltiplo más cercano de la tolerancia (en segundos)
    double toleranciaSeg = config_.toleranciaTemporalMs / 1000.0;
    return std::round(offset / toleranciaSeg) * toleranciaSeg;
}

double SistemaVotacion::calcularConfianza(int votosMejor, int totalHashesQuery) const {
    if (totalHashesQuery == 0) return 0.0;
    
    // Confianza basada en el porcentaje de hashes que coinciden
    double porcentaje = (100.0 * votosMejor) / totalHashesQuery;
    
    // Limitar entre 0 y 100
    return std::min(100.0, std::max(0.0, porcentaje));
}