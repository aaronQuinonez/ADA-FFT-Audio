// src/busqueda/SistemaVotacion.h
#ifndef SISTEMA_VOTACION_H
#define SISTEMA_VOTACION_H

#include <map>
#include <vector>
#include <cstdint>

class SistemaVotacion {
public:
    // Resultado de votación para una canción
    struct ResultadoCancion {
        int idCancion;
        int totalCoincidencias;
        double offsetMejor;          // Offset con más votos
        int votosMejor;              // Número de votos en el mejor offset
        double confianza;            // Confianza (0-100%)
        
        ResultadoCancion() : idCancion(-1), totalCoincidencias(0), 
                            offsetMejor(0.0), votosMejor(0), confianza(0.0) {}
    };
    
    // Configuración del sistema
    struct Configuracion {
        double toleranciaTemporalMs;
        int minimoCoincidencias;
        double umbralConfianza;
        
        Configuracion() 
            : toleranciaTemporalMs(50.0),
              minimoCoincidencias(5),
              umbralConfianza(15.0) {}
    };
    
    // Constructor
    SistemaVotacion(const Configuracion& config);
    
    // Registrar un voto (coincidencia de hash)
    void registrarVoto(int idCancion, double timestampQuery, double timestampDB);
    
    // Obtener resultados ordenados por confianza
    std::vector<ResultadoCancion> obtenerResultados(int totalHashesQuery) const;
    
    // Obtener mejor resultado
    ResultadoCancion obtenerMejorResultado(int totalHashesQuery) const;
    
    // Limpiar votos
    void limpiar();
    
    // Mostrar estadísticas de votación
    void mostrarEstadisticas() const;
    
private:
    // Histograma de offsets por canción
    // idCancion -> offset -> número de votos
    std::map<int, std::map<double, int>> histogramas_;
    
    // Contador total de coincidencias por canción
    std::map<int, int> contadores_;
    
    Configuracion config_;
    
    // Redondear offset a la tolerancia configurada
    double redondearOffset(double offset) const;
    
    // Calcular confianza basada en votos
    double calcularConfianza(int votosMejor, int totalHashesQuery) const;
};

#endif