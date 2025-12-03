#ifndef GENERADOR_HASHES_H
#define GENERADOR_HASHES_H

#include <vector>
#include <string>
#include <cstdint>
#include "DetectorPicos.h" // Incluye tu archivo DetectorPicos.h

class GeneradorHashes {
public:
    // Configuración avanzada para la generación de fingerprints
    struct Configuracion {
        double ventanaTemporalMs = 2000.0;    // Ventana de búsqueda (2 segundos)
        int maxPicosObjetivo = 5;             // Picos objetivo por ancla
        
        // Rango de frecuencias a considerar
        double frecuenciaMinima = 30.0;
        double frecuenciaMaxima = 5000.0;
        
        // Distribución de bits para el Hash (Total 32 bits)
        // 9 + 9 + 14 = 32 bits
        int bitsFrequenciaAncla = 9;
        int bitsFrequenciaObjetivo = 9;
        int bitsDiferenciaTemporal = 14;
    };

    // Estructura de un Hash individual
    struct Hash {
        uint32_t valor;
        double tiempoAncla;
        size_t indiceAncla;
        size_t indiceObjetivo;

        Hash(uint32_t v, double t, size_t ia, size_t io) 
            : valor(v), tiempoAncla(t), indiceAncla(ia), indiceObjetivo(io) {}
    };

    // Estructura para devolver los resultados y estadísticas
    struct Resultado {
        std::vector<Hash> hashes;
        int totalHashesGenerados = 0;
        size_t totalPicosUsados = 0;
        double densidadHashes = 0.0;
    };

    struct HashDecodificado {
        int frecuenciaAncla;
        int frecuenciaObjetivo;
        int diferenciaTemporal;
    };

    // --- MÉTODOS PRINCIPALES ---

    // Método principal para generar los hashes
    static Resultado generarHashes(
        const std::vector<DetectorPicos::Pico>& picos, // <--- CAMBIO AQUÍ
        const Configuracion& config
    );

    // Exportar a TXT
    static void exportarHashes(
        const Resultado& resultado,
        const std::string& nombreArchivo,
        const std::string& nombreCancion = "Desconocida"
    );

    // Exportar a Binario
    static void exportarHashesBinario(
        const Resultado& resultado,
        const std::string& nombreArchivo
    );

private:
    // --- MÉTODOS INTERNOS (AUXILIARES) ---
    
    static uint32_t codificarHash(
        double frecuenciaAncla,
        double frecuenciaObjetivo,
        double diferenciaTemporal,
        const Configuracion& config
    );

    static HashDecodificado decodificarHash(
        uint32_t hash, 
        const Configuracion& config
    );

    static std::vector<DetectorPicos::Pico> filtrarPicosPorFrecuencia( // <--- CAMBIO AQUÍ
        const std::vector<DetectorPicos::Pico>& picos,
        double frecuenciaMin,
        double frecuenciaMax
    );

    static int cuantizarFrecuencia(
        double frecuencia,
        double frecuenciaMin,
        double frecuenciaMax,
        int numBits
    );

    static int cuantizarTiempo(
        double diferenciaMs,
        double ventanaMaxMs,
        int numBits
    );
};

#endif
