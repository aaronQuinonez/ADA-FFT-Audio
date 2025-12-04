// src/indexacion/IndiceInvertido.h
#ifndef INDICE_INVERTIDO_H
#define INDICE_INVERTIDO_H

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <string>

class IndiceInvertido {
public:
    // Estructura para almacenar una entrada del índice
    struct Entrada {
        int idCancion;          // ID de la canción
        double timestamp;       // Timestamp en segundos
        
        Entrada(int id, double t) : idCancion(id), timestamp(t) {}
    };
    
    // Estadísticas del índice
    struct Estadisticas {
        size_t totalHashes;             // Total de hashes únicos
        size_t totalEntradas;           // Total de entradas (sum de listas)
        size_t hashesUnicos;            // Hashes con 1 sola entrada
        size_t hashesDuplicados;        // Hashes con múltiples entradas
        double promedioEntradasPorHash; // Promedio de entradas por hash
        size_t maxEntradasEnHash;       // Máximo de entradas en un hash
    };
    
    // Constructor
    IndiceInvertido();
    
    // Agregar una entrada al índice
    void agregar(uint32_t hash, int idCancion, double timestamp);
    
    // Buscar todas las entradas para un hash
    const std::vector<Entrada>* buscar(uint32_t hash) const;
    
    // Verificar si un hash existe
    bool existe(uint32_t hash) const;
    
    // Obtener número de hashes únicos
    size_t numeroHashesUnicos() const;
    
    // Obtener estadísticas del índice
    Estadisticas obtenerEstadisticas() const;
    
    // Limpiar el índice
    void limpiar();
    
    // Mostrar información del índice
    void mostrarInfo() const;
    
    // Serialización
    bool guardarEnArchivo(const std::string& nombreArchivo) const;
    bool cargarDesdeArchivo(const std::string& nombreArchivo);
    
private:
    // Hash table: hash -> lista de (idCancion, timestamp)
    std::unordered_map<uint32_t, std::vector<Entrada>> indice_;
};

#endif