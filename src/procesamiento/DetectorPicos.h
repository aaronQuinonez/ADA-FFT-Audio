#ifndef DETECTOR_PICOS_H
#define DETECTOR_PICOS_H

#include <vector>
#include <string>
#include "Espectrograma.h"

class DetectorPicos {
public:
    // Estructura para representar un pico detectado
    struct Pico {
        double tiempo;          // Timestamp en segundos
        double frecuencia;      // Frecuencia en Hz
        double magnitud;        // Magnitud del pico
        int indiceBanda;        // Índice de la banda de frecuencia
        int indiceVentana;      // Índice de la ventana temporal
        int indiceFrecuencia;   // Índice dentro de la banda
        
        // Constructor
        Pico(double t, double f, double m, int banda, int ventana, int freq)
            : tiempo(t), frecuencia(f), magnitud(m), 
              indiceBanda(banda), indiceVentana(ventana), indiceFrecuencia(freq) {}
    };
    
    // Configuración de detección
    struct Configuracion {
        double umbralMagnitud = 0.1;        // Magnitud mínima para considerar un pico
        int vecinosLocales = 3;             // Radio para máximo local
        int picosPorBanda = 5;              // Número de picos a detectar por banda
        bool usarAdaptativo = true;         // Usar umbral adaptativo
        double percentilUmbral = 75.0;      // Percentil para umbral adaptativo (75%)
    };
    
    // Resultado de la detección
    struct Resultado {
        std::vector<Pico> picos;
        int totalPicosDetectados;
        double tiempoTotal;
        std::vector<double> umbralesPorBanda;  // Umbrales usados por banda
    };
    
    // Detectar picos en espectrograma completo
    static Resultado detectarPicos(
        const Espectrograma::Resultado& espectrograma,
        const Configuracion& config
    );
    
    // Detectar picos en bandas de frecuencia
    static Resultado detectarPicosEnBandas(
        const std::vector<std::vector<double>>& bandas,
        const std::vector<std::pair<double, double>>& definicionesBandas,
        double resolucionTemporal,
        const Configuracion& config
    );
    
    // Detectar máximos locales en una ventana
    static std::vector<int> detectarMaximosLocales(
        const std::vector<double>& magnitudes,
        int radio
    );
    
    // Calcular umbral adaptativo
    static double calcularUmbralAdaptativo(
        const std::vector<double>& magnitudes,
        double percentil
    );
    
    // Exportar picos a archivo CSV
    static void exportarCSV(
        const Resultado& resultado,
        const std::string& nombreArchivo
    );
    
    // Exportar picos a formato de constelación (para Fase 4)
    static void exportarConstelacion(
        const Resultado& resultado,
        const std::string& nombreArchivo
    );
    
    // Filtrar picos por criterios adicionales
    static std::vector<Pico> filtrarPicos(
        const std::vector<Pico>& picos,
        double magnitudMinima,
        double frecuenciaMin = 0.0,
        double frecuenciaMax = 20000.0
    );
};

#endif