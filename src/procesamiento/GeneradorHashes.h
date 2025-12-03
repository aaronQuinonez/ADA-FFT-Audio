#ifndef GENERADOR_HASHES_H
#define GENERADOR_HASHES_H

#include <vector>
#include <string>
#include <cstdint>
#include "DetectorPicos.h"

class GeneradorHashes {
public:
    // Estructura para representar un hash generado
    struct Hash {
        uint32_t valor;              // Hash de 32 bits
        double tiempoAncla;          // Timestamp del pico ancla en la canción
        int indiceAncla;             // Índice del pico ancla
        int indiceObjetivo;          // Índice del pico objetivo
        
        Hash(uint32_t v, double t, int a, int o)
            : valor(v), tiempoAncla(t), indiceAncla(a), indiceObjetivo(o) {}
    };
    
    // Configuración del generador
    struct Configuracion {
        double ventanaTemporalMs = 200.0;      // Ventana de tiempo para emparejar (ms)
        int maxPicosObjetivo = 5;              // Máximo de picos objetivo por ancla
        double frecuenciaMinima = 30.0;        // Frecuencia mínima en Hz
        double frecuenciaMaxima = 5000.0;      // Frecuencia máxima en Hz
        int bitsFrequenciaAncla = 9;           // Bits para frecuencia ancla (0-511)
        int bitsFrequenciaObjetivo = 9;        // Bits para frecuencia objetivo (0-511)
        int bitsDiferenciaTemporal = 14;       // Bits para diferencia temporal (0-16383)
    };
    
    // Resultado de la generación
    struct Resultado {
        std::vector<Hash> hashes;
        int totalHashesGenerados;
        int totalPicosUsados;
        double densidadHashes;         // Hashes por segundo
    };
    
    // Generar hashes a partir de picos detectados
    static Resultado generarHashes(
        const std::vector<DetectorPicos::Pico>& picos,
        const Configuracion& config
    );
    
    // Codificar par de picos como hash de 32 bits
    static uint32_t codificarHash(
        double frecuenciaAncla,
        double frecuenciaObjetivo,
        double diferenciaTemporal,
        const Configuracion& config
    );
    
    // Decodificar hash de 32 bits
    struct HashDecodificado {
        int frecuenciaAncla;
        int frecuenciaObjetivo;
        int diferenciaTemporal;
    };
    static HashDecodificado decodificarHash(uint32_t hash, const Configuracion& config);
    
    // Exportar hashes a archivo
    static void exportarHashes(
        const Resultado& resultado,
        const std::string& nombreArchivo,
        const std::string& nombreCancion = "desconocido"
    );
    
    // Exportar hashes en formato binario (más eficiente)
    static void exportarHashesBinario(
        const Resultado& resultado,
        const std::string& nombreArchivo
    );
    
    // Filtrar picos por rango de frecuencia antes de generar hashes
    static std::vector<DetectorPicos::Pico> filtrarPicosPorFrecuencia(
        const std::vector<DetectorPicos::Pico>& picos,
        double frecuenciaMin,
        double frecuenciaMax
    );
    
private:
    // Cuantizar frecuencia al número de bits especificado
    static int cuantizarFrecuencia(
        double frecuencia,
        double frecuenciaMin,
        double frecuenciaMax,
        int numBits
    );
    
    // Cuantizar diferencia temporal
    static int cuantizarTiempo(
        double diferenciaMs,
        double ventanaMaxMs,
        int numBits
    );
};

#endif
