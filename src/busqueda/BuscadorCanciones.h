// src/busqueda/BuscadorCanciones.h
#ifndef BUSCADOR_CANCIONES_H
#define BUSCADOR_CANCIONES_H

#include "../indexacion/BaseDatosHashes.h"
#include "SistemaVotacion.h"
#include <vector>
#include <string>

class BuscadorCanciones {
public:
    // Resultado de búsqueda
    struct Resultado {
        bool encontrado;
        int idCancion;
        std::string nombreCancion;
        double offsetSegundos;
        double confianza;
        int coincidencias;
        int totalHashesQuery;
        double tiempoBusqueda;  // Tiempo en milisegundos
        
        Resultado() : encontrado(false), idCancion(-1), offsetSegundos(0.0),
                      confianza(0.0), coincidencias(0), totalHashesQuery(0),
                      tiempoBusqueda(0.0) {}
    };
    
    // Configuración del buscador
    struct Configuracion {
        SistemaVotacion::Configuracion configVotacion;
        bool mostrarProgreso;
        bool mostrarEstadisticas;

        Configuracion()
            : mostrarProgreso(true),
              mostrarEstadisticas(false) {}
    };
    
    // Constructor
    BuscadorCanciones(const BaseDatosHashes& baseDatos,
                      const Configuracion& config = Configuracion());
    
    Resultado buscar(const std::vector<GeneradorHashes::Hash>& hashesQuery);
    
    std::vector<Resultado> buscarTopN(
        const std::vector<GeneradorHashes::Hash>& hashesQuery,
        int topN = 5
    );
    
    static void mostrarResultado(const Resultado& resultado);
    
private:
    const BaseDatosHashes& baseDatos_;
    Configuracion config_;
    
    Resultado convertirResultado(
        const SistemaVotacion::ResultadoCancion& resultadoVotacion,
        int totalHashesQuery,
        double tiempoBusqueda
    ) const;
};

#endif
