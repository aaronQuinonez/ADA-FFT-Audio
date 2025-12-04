// src/indexacion/IndiceInvertido.cpp
#include "IndiceInvertido.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>

IndiceInvertido::IndiceInvertido() {
    // Reservar espacio para mejorar rendimiento
    indice_.reserve(100000);
}

void IndiceInvertido::agregar(uint32_t hash, int idCancion, double timestamp) {
    indice_[hash].emplace_back(idCancion, timestamp);
}

const std::vector<IndiceInvertido::Entrada>* IndiceInvertido::buscar(uint32_t hash) const {
    auto it = indice_.find(hash);
    if (it != indice_.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool IndiceInvertido::existe(uint32_t hash) const {
    return indice_.find(hash) != indice_.end();
}

size_t IndiceInvertido::numeroHashesUnicos() const {
    return indice_.size();
}

IndiceInvertido::Estadisticas IndiceInvertido::obtenerEstadisticas() const {
    Estadisticas stats;
    stats.totalHashes = indice_.size();
    stats.totalEntradas = 0;
    stats.hashesUnicos = 0;
    stats.hashesDuplicados = 0;
    stats.maxEntradasEnHash = 0;
    
    for (const auto& par : indice_) {
        size_t numEntradas = par.second.size();
        stats.totalEntradas += numEntradas;
        
        if (numEntradas == 1) {
            stats.hashesUnicos++;
        } else {
            stats.hashesDuplicados++;
        }
        
        if (numEntradas > stats.maxEntradasEnHash) {
            stats.maxEntradasEnHash = numEntradas;
        }
    }
    
    stats.promedioEntradasPorHash = stats.totalHashes > 0 ? 
        (double)stats.totalEntradas / stats.totalHashes : 0.0;
    
    return stats;
}

void IndiceInvertido::limpiar() {
    indice_.clear();
}

void IndiceInvertido::mostrarInfo() const {
    Estadisticas stats = obtenerEstadisticas();
    
    std::cout << "\n=== Información del Índice Invertido ===" << std::endl;
    std::cout << "Total de hashes únicos: " << stats.totalHashes << std::endl;
    std::cout << "Total de entradas: " << stats.totalEntradas << std::endl;
    std::cout << "Hashes únicos (1 entrada): " << stats.hashesUnicos 
              << " (" << (100.0 * stats.hashesUnicos / stats.totalHashes) << "%)" << std::endl;
    std::cout << "Hashes duplicados (>1 entrada): " << stats.hashesDuplicados 
              << " (" << (100.0 * stats.hashesDuplicados / stats.totalHashes) << "%)" << std::endl;
    std::cout << "Promedio de entradas por hash: " 
              << std::fixed << std::setprecision(2) << stats.promedioEntradasPorHash << std::endl;
    std::cout << "Máximo de entradas en un hash: " << stats.maxEntradasEnHash << std::endl;
}

bool IndiceInvertido::guardarEnArchivo(const std::string& nombreArchivo) const {
    std::ofstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo de índice" << std::endl;
        return false;
    }
    
    // Escribir número de hashes
    size_t numHashes = indice_.size();
    archivo.write((char*)&numHashes, sizeof(size_t));
    
    // Escribir cada entrada del índice
    for (const auto& par : indice_) {
        uint32_t hash = par.first;
        const auto& entradas = par.second;
        size_t numEntradas = entradas.size();
        
        // Escribir hash
        archivo.write((char*)&hash, sizeof(uint32_t));
        // Escribir número de entradas
        archivo.write((char*)&numEntradas, sizeof(size_t));
        
        // Escribir cada entrada
        for (const auto& entrada : entradas) {
            archivo.write((char*)&entrada.idCancion, sizeof(int));
            archivo.write((char*)&entrada.timestamp, sizeof(double));
        }
    }
    
    archivo.close();
    return true;
}

bool IndiceInvertido::cargarDesdeArchivo(const std::string& nombreArchivo) {
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de índice" << std::endl;
        return false;
    }
    
    limpiar();
    
    // Leer número de hashes
    size_t numHashes;
    archivo.read((char*)&numHashes, sizeof(size_t));
    
    // Leer cada entrada del índice
    for (size_t i = 0; i < numHashes; i++) {
        uint32_t hash;
        size_t numEntradas;
        
        // Leer hash
        archivo.read((char*)&hash, sizeof(uint32_t));
        // Leer número de entradas
        archivo.read((char*)&numEntradas, sizeof(size_t));
        
        // Leer cada entrada
        for (size_t j = 0; j < numEntradas; j++) {
            int idCancion;
            double timestamp;
            
            archivo.read((char*)&idCancion, sizeof(int));
            archivo.read((char*)&timestamp, sizeof(double));
            
            agregar(hash, idCancion, timestamp);
        }
    }
    
    archivo.close();
    return true;
}