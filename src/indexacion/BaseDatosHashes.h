// src/indexacion/BaseDatosHashes.h
#ifndef BASE_DATOS_HASHES_H
#define BASE_DATOS_HASHES_H

#include "IndiceInvertido.h"
#include "../procesamiento/GeneradorHashes.h"
#include <string>
#include <vector>
#include <map>

class BaseDatosHashes {
public:
    // Metadatos de una canción
    struct MetadatosCancion {
        int id;
        std::string nombre;
        std::string rutaArchivo;
        double duracion;
        int numeroHashes;
        
        MetadatosCancion() : id(-1), duracion(0.0), numeroHashes(0) {}
        MetadatosCancion(int i, const std::string& n, const std::string& r, double d, int nh)
            : id(i), nombre(n), rutaArchivo(r), duracion(d), numeroHashes(nh) {}
    };
    
    // Estadísticas de la base de datos
    struct Estadisticas {
        int totalCanciones;
        int totalHashes;
        double duracionTotal;
        double promedioHashesPorCancion;
        double promedioHashesPorSegundo;
    };
    
    // Constructor
    BaseDatosHashes();
    
    // Agregar una canción a la base de datos
    int agregarCancion(
        const std::string& nombre,
        const std::string& rutaArchivo,
        double duracion,
        const std::vector<GeneradorHashes::Hash>& hashes
    );
    
    // Obtener metadatos de una canción por ID
    const MetadatosCancion* obtenerMetadatos(int idCancion) const;
    
    // Obtener metadatos de una canción por nombre
    const MetadatosCancion* obtenerMetadatosPorNombre(const std::string& nombre) const;
    
    // Obtener el índice invertido
    const IndiceInvertido& obtenerIndice() const;
    
    // Obtener número de canciones
    int numeroCanciones() const;
    
    // Obtener estadísticas
    Estadisticas obtenerEstadisticas() const;
    
    // Mostrar información de la base de datos
    void mostrarInfo() const;
    
    // Listar todas las canciones
    void listarCanciones() const;
    
    // Guardar base de datos en archivos
    bool guardar(const std::string& rutaBase) const;
    
    // Cargar base de datos desde archivos
    bool cargar(const std::string& rutaBase);
    
    // Limpiar base de datos
    void limpiar();
    
private:
    IndiceInvertido indice_;
    std::map<int, MetadatosCancion> canciones_;
    int siguienteId_;
};

#endif