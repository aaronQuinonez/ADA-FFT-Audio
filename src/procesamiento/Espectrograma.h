#ifndef ESPECTROGRAMA_H
#define ESPECTROGRAMA_H

#include <vector>
#include <string>
#include "../audio/LectorAudio.h"
#include "../utilidades/NumeroComplejo.h"

class Espectrograma {
public:
    // Configuración del espectrograma
    struct Configuracion {
        int tamanoVentana = 1024;       // Tamaño de la ventana FFT
        int solapamiento = 512;          // 50% de solapamiento
        int inicioAudio = 0;             // Desde dónde empezar (para saltar silencio)
        bool aplicarHamming = true;      // Aplicar ventana de Hamming
    };
    
    // Estructura de resultado
    struct Resultado {
        std::vector<std::vector<double>> magnitudes;  // [ventana][frecuencia]
        int numVentanas;
        int numFrecuencias;
        double resolucionFrecuencia;    // Hz por bin
        double resolucionTemporal;      // Segundos por ventana
        int frecuenciaMuestreo;
    };
    
    // Calcular espectrograma completo
    static Resultado calcular(const DatosAudio& audio, const Configuracion& config);
    
    // Aplicar ventana de Hamming
    static std::vector<double> ventanaHamming(int tamano);
    
    // Dividir en bandas de frecuencia (para Fase 3)
    static std::vector<std::vector<double>> dividirEnBandas(
        const Resultado& resultado,
        const std::vector<std::pair<double, double>>& bandas
    );
    
    // Exportar a CSV
    static void exportarCSV(const Resultado& resultado, const std::string& nombreArchivo);
    
    // Exportar bandas a CSV
    static void exportarBandasCSV(
        const std::vector<std::vector<double>>& bandas,
        const std::vector<std::pair<double, double>>& definicionesBandas,
        const std::string& nombreArchivo
    );
};

#endif
