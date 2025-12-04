// src/indexacion/BaseDatosHashes.cpp
#include "BaseDatosHashes.h"
#include <iostream>
#include <fstream>
#include <iomanip>

BaseDatosHashes::BaseDatosHashes() : siguienteId_(0) {}

int BaseDatosHashes::agregarCancion(
    const std::string& nombre,
    const std::string& rutaArchivo,
    double duracion,
    const std::vector<GeneradorHashes::Hash>& hashes
) {
    int idCancion = siguienteId_++;
    
    // Agregar metadatos
    canciones_[idCancion] = MetadatosCancion(
        idCancion, nombre, rutaArchivo, duracion, hashes.size()
    );
    
    // Agregar hashes al índice
    for (const auto& hash : hashes) {
        indice_.agregar(hash.valor, idCancion, hash.tiempoAncla);
    }
    
    std::cout << "[DB] Canción agregada: '" << nombre << "' (ID=" << idCancion 
              << ", " << hashes.size() << " hashes)" << std::endl;
    
    return idCancion;
}

const BaseDatosHashes::MetadatosCancion* BaseDatosHashes::obtenerMetadatos(int idCancion) const {
    auto it = canciones_.find(idCancion);
    if (it != canciones_.end()) {
        return &(it->second);
    }
    return nullptr;
}

const BaseDatosHashes::MetadatosCancion* BaseDatosHashes::obtenerMetadatosPorNombre(
    const std::string& nombre
) const {
    for (const auto& par : canciones_) {
        if (par.second.nombre == nombre) {
            return &(par.second);
        }
    }
    return nullptr;
}

const IndiceInvertido& BaseDatosHashes::obtenerIndice() const {
    return indice_;
}

int BaseDatosHashes::numeroCanciones() const {
    return canciones_.size();
}

BaseDatosHashes::Estadisticas BaseDatosHashes::obtenerEstadisticas() const {
    Estadisticas stats;
    stats.totalCanciones = canciones_.size();
    stats.totalHashes = 0;
    stats.duracionTotal = 0.0;
    
    for (const auto& par : canciones_) {
        stats.totalHashes += par.second.numeroHashes;
        stats.duracionTotal += par.second.duracion;
    }
    
    stats.promedioHashesPorCancion = stats.totalCanciones > 0 ?
        (double)stats.totalHashes / stats.totalCanciones : 0.0;
    
    stats.promedioHashesPorSegundo = stats.duracionTotal > 0 ?
        (double)stats.totalHashes / stats.duracionTotal : 0.0;
    
    return stats;
}

void BaseDatosHashes::mostrarInfo() const {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   INFORMACIÓN DE LA BASE DE DATOS     " << std::endl;
    std::cout << "========================================" << std::endl;
    
    Estadisticas stats = obtenerEstadisticas();
    
    std::cout << "Canciones indexadas: " << stats.totalCanciones << std::endl;
    std::cout << "Total de fingerprints: " << stats.totalHashes << std::endl;
    std::cout << "Duración total: " << std::fixed << std::setprecision(1) 
              << stats.duracionTotal << " segundos" << std::endl;
    std::cout << "Promedio de hashes por canción: " 
              << std::fixed << std::setprecision(0) << stats.promedioHashesPorCancion << std::endl;
    std::cout << "Densidad de hashes: " 
              << std::fixed << std::setprecision(1) << stats.promedioHashesPorSegundo 
              << " hashes/segundo" << std::endl;
    
    std::cout << "\n";
    indice_.mostrarInfo();
}

void BaseDatosHashes::listarCanciones() const {
    std::cout << "\n=== Canciones en la Base de Datos ===" << std::endl;
    std::cout << std::left << std::setw(5) << "ID" 
              << std::setw(40) << "Nombre" 
              << std::setw(12) << "Duración" 
              << std::setw(10) << "Hashes" << std::endl;
    std::cout << std::string(67, '-') << std::endl;
    
    for (const auto& par : canciones_) {
        const auto& meta = par.second;
        std::cout << std::left << std::setw(5) << meta.id
                  << std::setw(40) << meta.nombre
                  << std::setw(12) << (std::to_string((int)meta.duracion) + "s")
                  << std::setw(10) << meta.numeroHashes << std::endl;
    }
}

bool BaseDatosHashes::guardar(const std::string& rutaBase) const {
    std::cout << "\nGuardando base de datos..." << std::endl;
    
    // 1. Guardar metadatos
    std::string archivoMetadatos = rutaBase + "_metadata.txt";
    std::ofstream meta(archivoMetadatos);
    if (!meta.is_open()) {
        std::cerr << "Error: No se pudo crear archivo de metadatos" << std::endl;
        return false;
    }
    
    meta << "[METADATA]\n";
    meta << "num_songs=" << canciones_.size() << "\n";
    meta << "version=1.0\n\n";
    
    for (const auto& par : canciones_) {
        const auto& c = par.second;
        meta << "[SONG_" << c.id << "]\n";
        meta << "id=" << c.id << "\n";
        meta << "name=" << c.nombre << "\n";
        meta << "path=" << c.rutaArchivo << "\n";
        meta << "duration=" << std::fixed << std::setprecision(3) << c.duracion << "\n";
        meta << "num_hashes=" << c.numeroHashes << "\n\n";
    }
    
    meta.close();
    std::cout << "✓ Metadatos guardados en '" << archivoMetadatos << "'" << std::endl;
    
    // 2. Guardar índice
    std::string archivoIndice = rutaBase + "_index.bin";
    if (!indice_.guardarEnArchivo(archivoIndice)) {
        return false;
    }
    std::cout << "✓ Índice guardado en '" << archivoIndice << "'" << std::endl;
    
    std::cout << "✓ Base de datos guardada exitosamente" << std::endl;
    return true;
}

bool BaseDatosHashes::cargar(const std::string& rutaBase) {
    std::cout << "\nCargando base de datos..." << std::endl;
    
    limpiar();
    
    // 1. Cargar metadatos
    std::string archivoMetadatos = rutaBase + "_metadata.txt";
    std::ifstream meta(archivoMetadatos);
    if (!meta.is_open()) {
        std::cerr << "Error: No se pudo abrir archivo de metadatos" << std::endl;
        return false;
    }
    
    std::string linea;
    int idActual = -1;
    MetadatosCancion cancionActual;
    
    while (std::getline(meta, linea)) {
        if (linea.empty() || linea[0] == '[') {
            // Nueva sección
            if (idActual >= 0 && !cancionActual.nombre.empty()) {
                canciones_[idActual] = cancionActual;
                if (idActual >= siguienteId_) {
                    siguienteId_ = idActual + 1;
                }
            }
            
            if (linea.find("[SONG_") == 0) {
                cancionActual = MetadatosCancion();
            }
            continue;
        }
        
        size_t pos = linea.find('=');
        if (pos == std::string::npos) continue;
        
        std::string clave = linea.substr(0, pos);
        std::string valor = linea.substr(pos + 1);
        
        if (clave == "id") {
            idActual = std::stoi(valor);
            cancionActual.id = idActual;
        } else if (clave == "name") {
            cancionActual.nombre = valor;
        } else if (clave == "path") {
            cancionActual.rutaArchivo = valor;
        } else if (clave == "duration") {
            cancionActual.duracion = std::stod(valor);
        } else if (clave == "num_hashes") {
            cancionActual.numeroHashes = std::stoi(valor);
        }
    }
    
    // Agregar última canción
    if (idActual >= 0 && !cancionActual.nombre.empty()) {
        canciones_[idActual] = cancionActual;
    }
    
    meta.close();
    std::cout << "✓ Metadatos cargados: " << canciones_.size() << " canciones" << std::endl;
    
    // 2. Cargar índice
    std::string archivoIndice = rutaBase + "_index.bin";
    if (!indice_.cargarDesdeArchivo(archivoIndice)) {
        return false;
    }
    std::cout << "✓ Índice cargado: " << indice_.numeroHashesUnicos() << " hashes únicos" << std::endl;
    
    std::cout << "✓ Base de datos cargada exitosamente" << std::endl;
    return true;
}

void BaseDatosHashes::limpiar() {
    indice_.limpiar();
    canciones_.clear();
    siguienteId_ = 0;
}