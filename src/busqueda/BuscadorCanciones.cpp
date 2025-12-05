// src/busqueda/BuscadorCanciones.cpp
#include "BuscadorCanciones.h"
#include <iostream>
#include <iomanip>
#include <chrono>

BuscadorCanciones::BuscadorCanciones(
    const BaseDatosHashes& baseDatos,
    const Configuracion& config
) : baseDatos_(baseDatos), config_(config) {}

BuscadorCanciones::Resultado BuscadorCanciones::buscar(
    const std::vector<GeneradorHashes::Hash>& hashesQuery
) {
    auto inicio = std::chrono::high_resolution_clock::now();
    
    if (config_.mostrarProgreso) {
        std::cout << "\n=== Iniciando Búsqueda ===" << std::endl;
        std::cout << "Hashes del query: " << hashesQuery.size() << std::endl;
        std::cout << "Canciones en la base de datos: " << baseDatos_.numeroCanciones() << std::endl;
    }
    
    // Crear sistema de votación
    SistemaVotacion votacion(config_.configVotacion);
    
    // Buscar cada hash del query en el índice
    const IndiceInvertido& indice = baseDatos_.obtenerIndice();
    int hashesEncontrados = 0;
    
    if (config_.mostrarProgreso) {
        std::cout << "\nBuscando coincidencias..." << std::endl;
    }
    
    int progreso = 0;
    int totalHashes = hashesQuery.size();
    
    for (const auto& hashQuery : hashesQuery) {
        // Mostrar progreso
        if (config_.mostrarProgreso) {
            int porcentaje = (100 * progreso) / totalHashes;
            if (progreso % (totalHashes / 10 + 1) == 0) {
                std::cout << "  Progreso: " << porcentaje << "%" << std::endl;
            }
            progreso++;
        }
        
        // Buscar este hash en el índice
        const auto* entradas = indice.buscar(hashQuery.valor);
        
        if (entradas) {
            hashesEncontrados++;
            
            // Registrar voto para cada coincidencia
            for (const auto& entrada : *entradas) {
                votacion.registrarVoto(
                    entrada.idCancion,
                    hashQuery.tiempoAncla,
                    entrada.timestamp
                );
            }
        }
    }
    
    if (config_.mostrarProgreso) {
        std::cout << "  Progreso: 100%" << std::endl;
        std::cout << "\nHashes con coincidencias: " << hashesEncontrados 
                  << " de " << hashesQuery.size() 
                  << " (" << (100.0 * hashesEncontrados / hashesQuery.size()) << "%)" 
                  << std::endl;
    }
    
    if (config_.mostrarEstadisticas) {
        votacion.mostrarEstadisticas();
    }
    
    // Obtener mejor resultado
    auto resultadoVotacion = votacion.obtenerMejorResultado(hashesQuery.size());
    
    // Calcular tiempo de búsqueda
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
    double tiempoMs = duracion.count();
    
    // Convertir a resultado de búsqueda
    return convertirResultado(resultadoVotacion, hashesQuery.size(), tiempoMs);
}

std::vector<BuscadorCanciones::Resultado> BuscadorCanciones::buscarTopN(
    const std::vector<GeneradorHashes::Hash>& hashesQuery,
    int topN
) {
    auto inicio = std::chrono::high_resolution_clock::now();
    
    // Crear sistema de votación
    SistemaVotacion votacion(config_.configVotacion);
    
    // Buscar cada hash
    const IndiceInvertido& indice = baseDatos_.obtenerIndice();
    
    for (const auto& hashQuery : hashesQuery) {
        const auto* entradas = indice.buscar(hashQuery.valor);
        
        if (entradas) {
            for (const auto& entrada : *entradas) {
                votacion.registrarVoto(
                    entrada.idCancion,
                    hashQuery.tiempoAncla,
                    entrada.timestamp
                );
            }
        }
    }
    
    // Obtener todos los resultados
    auto resultadosVotacion = votacion.obtenerResultados(hashesQuery.size());
    
    // Calcular tiempo
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
    double tiempoMs = duracion.count();
    
    // Convertir resultados (limitar a topN)
    std::vector<Resultado> resultados;
    int limite = std::min(topN, (int)resultadosVotacion.size());
    
    for (int i = 0; i < limite; i++) {
        resultados.push_back(
            convertirResultado(resultadosVotacion[i], hashesQuery.size(), tiempoMs)
        );
    }
    
    return resultados;
}

void BuscadorCanciones::mostrarResultado(const Resultado& resultado) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "       RESULTADO DE LA BÚSQUEDA         " << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (!resultado.encontrado) {
        std::cout << "No se encontró ninguna coincidencia" << std::endl;
        std::cout << "Hashes analizados: " << resultado.totalHashesQuery << std::endl;
        std::cout << "Tiempo de búsqueda: " << std::fixed << std::setprecision(1) 
                  << resultado.tiempoBusqueda << " ms" << std::endl;
        return;
    }
    
    std::cout << " Canción detectada: " << resultado.nombreCancion << std::endl;
    std::cout << " ID: " << resultado.idCancion << std::endl;
    std::cout << " Confianza: " << std::fixed << std::setprecision(1) 
              << resultado.confianza << "%" << std::endl;
    std::cout << " Offset temporal: " << std::fixed << std::setprecision(2) 
              << resultado.offsetSegundos << " segundos" << std::endl;
    std::cout << " Coincidencias: " << resultado.coincidencias 
              << " de " << resultado.totalHashesQuery 
              << " hashes (" << std::fixed << std::setprecision(1)
              << (100.0 * resultado.coincidencias / resultado.totalHashesQuery) 
              << "%)" << std::endl;
    std::cout << " Tiempo de búsqueda: " << std::fixed << std::setprecision(1) 
              << resultado.tiempoBusqueda << " ms" << std::endl;
    
    // Nivel de confianza visual
    std::cout << "\n  Nivel de confianza: ";
    if (resultado.confianza >= 70) {
        std::cout << "MUY ALTO" << std::endl;
    } else if (resultado.confianza >= 50) {
        std::cout << "ALTO" << std::endl;
    } else if (resultado.confianza >= 30) {
        std::cout << "MEDIO" << std::endl;
    } else {
        std::cout << "BAJO" << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
}

BuscadorCanciones::Resultado BuscadorCanciones::convertirResultado(
    const SistemaVotacion::ResultadoCancion& resultadoVotacion,
    int totalHashesQuery,
    double tiempoBusqueda
) const {
    Resultado resultado;
    
    if (resultadoVotacion.idCancion < 0) {
        resultado.encontrado = false;
        resultado.totalHashesQuery = totalHashesQuery;
        resultado.tiempoBusqueda = tiempoBusqueda;
        return resultado;
    }
    
    // Obtener metadatos de la canción
    const auto* metadatos = baseDatos_.obtenerMetadatos(resultadoVotacion.idCancion);
    
    resultado.encontrado = true;
    resultado.idCancion = resultadoVotacion.idCancion;
    resultado.nombreCancion = metadatos ? metadatos->nombre : "Desconocida";
    resultado.offsetSegundos = resultadoVotacion.offsetMejor;
    resultado.confianza = resultadoVotacion.confianza;
    resultado.coincidencias = resultadoVotacion.votosMejor;
    resultado.totalHashesQuery = totalHashesQuery;
    resultado.tiempoBusqueda = tiempoBusqueda;
    
    return resultado;
}