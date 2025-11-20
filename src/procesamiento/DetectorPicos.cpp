#include "DetectorPicos.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

DetectorPicos::Resultado DetectorPicos::detectarPicos(
    const Espectrograma::Resultado& espectrograma,
    const Configuracion& config
) {
    std::cout << "\n=== Detección de Picos Espectrales ===" << std::endl;
    std::cout << "Configuración:" << std::endl;
    std::cout << "  Umbral de magnitud: " << config.umbralMagnitud << std::endl;
    std::cout << "  Radio de vecinos: " << config.vecinosLocales << std::endl;
    std::cout << "  Picos por ventana: " << config.picosPorBanda << std::endl;
    std::cout << "  Umbral adaptativo: " << (config.usarAdaptativo ? "Sí" : "No") << std::endl;
    
    Resultado resultado;
    resultado.tiempoTotal = espectrograma.magnitudes.size() * espectrograma.resolucionTemporal;
    
    // Procesar cada ventana temporal
    int porcentajeAnterior = -1;
    for (size_t v = 0; v < espectrograma.magnitudes.size(); v++) {
        // Mostrar progreso
        int porcentaje = (100 * v) / espectrograma.magnitudes.size();
        if (porcentaje != porcentajeAnterior && porcentaje % 20 == 0) {
            std::cout << "  Progreso: " << porcentaje << "%" << std::endl;
            porcentajeAnterior = porcentaje;
        }
        
        const std::vector<double>& magnitudes = espectrograma.magnitudes[v];
        double tiempo = v * espectrograma.resolucionTemporal;
        
        // Calcular umbral para esta ventana
        double umbral = config.umbralMagnitud;
        if (config.usarAdaptativo) {
            umbral = calcularUmbralAdaptativo(magnitudes, config.percentilUmbral);
        }
        
        // Detectar máximos locales
        std::vector<int> indicesMaximos = detectarMaximosLocales(magnitudes, config.vecinosLocales);
        
        // Filtrar por umbral y crear picos
        std::vector<std::pair<double, int>> picosTemporales;
        for (int idx : indicesMaximos) {
            if (magnitudes[idx] >= umbral) {
                picosTemporales.push_back({magnitudes[idx], idx});
            }
        }
        
        // Ordenar por magnitud descendente y tomar los N más fuertes
        std::sort(picosTemporales.begin(), picosTemporales.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        
        int numPicos = std::min((int)picosTemporales.size(), config.picosPorBanda);
        for (int i = 0; i < numPicos; i++) {
            double magnitud = picosTemporales[i].first;
            int indiceFrecuencia = picosTemporales[i].second;
            double frecuencia = indiceFrecuencia * espectrograma.resolucionFrecuencia;
            
            resultado.picos.emplace_back(tiempo, frecuencia, magnitud, 0, v, indiceFrecuencia);
        }
    }
    
    resultado.totalPicosDetectados = resultado.picos.size();
    
    std::cout << "  Progreso: 100%" << std::endl;
    std::cout << "\n✓ Detección de picos completada" << std::endl;
    std::cout << "  Total de picos detectados: " << resultado.totalPicosDetectados << std::endl;
    std::cout << "  Promedio de picos por ventana: " 
              << (double)resultado.totalPicosDetectados / espectrograma.magnitudes.size() << std::endl;
    
    return resultado;
}

DetectorPicos::Resultado DetectorPicos::detectarPicosEnBandas(
    const std::vector<std::vector<double>>& bandas,
    const std::vector<std::pair<double, double>>& definicionesBandas,
    double resolucionTemporal,
    const Configuracion& config
) {
    std::cout << "\n=== Detección de Picos por Bandas de Frecuencia ===" << std::endl;
    std::cout << "Número de bandas: " << definicionesBandas.size() << std::endl;
    
    Resultado resultado;
    resultado.tiempoTotal = bandas.size() * resolucionTemporal;
    resultado.umbralesPorBanda.resize(definicionesBandas.size());
    
    // Calcular umbral adaptativo por banda si está configurado
    if (config.usarAdaptativo) {
        std::cout << "\nCalculando umbrales adaptativos por banda..." << std::endl;
        for (size_t b = 0; b < definicionesBandas.size(); b++) {
            std::vector<double> magnitudesBanda;
            for (const auto& ventana : bandas) {
                magnitudesBanda.push_back(ventana[b]);
            }
            resultado.umbralesPorBanda[b] = calcularUmbralAdaptativo(
                magnitudesBanda, config.percentilUmbral
            );
            std::cout << "  Banda " << (b+1) << " (" 
                      << definicionesBandas[b].first << "-" 
                      << definicionesBandas[b].second << " Hz): umbral = " 
                      << resultado.umbralesPorBanda[b] << std::endl;
        }
    }
    
    // Procesar cada ventana temporal
    std::cout << "\nDetectando picos..." << std::endl;
    int porcentajeAnterior = -1;
    
    for (size_t v = 0; v < bandas.size(); v++) {
        // Mostrar progreso
        int porcentaje = (100 * v) / bandas.size();
        if (porcentaje != porcentajeAnterior && porcentaje % 20 == 0) {
            std::cout << "  Progreso: " << porcentaje << "%" << std::endl;
            porcentajeAnterior = porcentaje;
        }
        
        double tiempo = v * resolucionTemporal;
        
        // Detectar pico máximo en cada banda
        for (size_t b = 0; b < definicionesBandas.size(); b++) {
            double magnitud = bandas[v][b];
            
            // Aplicar umbral
            double umbral = config.usarAdaptativo ? 
                resultado.umbralesPorBanda[b] : config.umbralMagnitud;
            
            if (magnitud >= umbral) {
                // Frecuencia central de la banda
                double frecuenciaCentral = (definicionesBandas[b].first + 
                                           definicionesBandas[b].second) / 2.0;
                
                resultado.picos.emplace_back(tiempo, frecuenciaCentral, magnitud, b, v, 0);
            }
        }
    }
    
    resultado.totalPicosDetectados = resultado.picos.size();
    
    std::cout << "  Progreso: 100%" << std::endl;
    std::cout << "\n✓ Detección de picos por bandas completada" << std::endl;
    std::cout << "  Total de picos detectados: " << resultado.totalPicosDetectados << std::endl;
    std::cout << "  Promedio de picos por ventana: " 
              << (double)resultado.totalPicosDetectados / bandas.size() << std::endl;
    
    return resultado;
}

std::vector<int> DetectorPicos::detectarMaximosLocales(
    const std::vector<double>& magnitudes,
    int radio
) {
    std::vector<int> maximos;
    
    for (size_t i = radio; i < magnitudes.size() - radio; i++) {
        bool esMaximo = true;
        
        // Verificar si es máximo en su vecindad
        for (int j = -radio; j <= radio; j++) {
            if (j == 0) continue;
            if (magnitudes[i] < magnitudes[i + j]) {
                esMaximo = false;
                break;
            }
        }
        
        if (esMaximo) {
            maximos.push_back(i);
        }
    }
    
    return maximos;
}

double DetectorPicos::calcularUmbralAdaptativo(
    const std::vector<double>& magnitudes,
    double percentil
) {
    if (magnitudes.empty()) return 0.0;
    
    // Copiar y ordenar magnitudes
    std::vector<double> magOrdenadas = magnitudes;
    std::sort(magOrdenadas.begin(), magOrdenadas.end());
    
    // Calcular índice del percentil
    size_t indice = (size_t)(percentil / 100.0 * magOrdenadas.size());
    if (indice >= magOrdenadas.size()) {
        indice = magOrdenadas.size() - 1;
    }
    
    return magOrdenadas[indice];
}

void DetectorPicos::exportarCSV(
    const Resultado& resultado,
    const std::string& nombreArchivo
) {
    std::cout << "\nExportando picos a CSV..." << std::endl;
    
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo: " + nombreArchivo);
    }
    
    // Escribir encabezado
    archivo << "Tiempo(s),Frecuencia(Hz),Magnitud,Banda,VentanaIdx,FrecuenciaIdx" << std::endl;
    
    // Escribir picos
    for (const auto& pico : resultado.picos) {
        archivo << std::fixed << std::setprecision(6)
                << pico.tiempo << ","
                << pico.frecuencia << ","
                << std::scientific << std::setprecision(6)
                << pico.magnitud << ","
                << pico.indiceBanda << ","
                << pico.indiceVentana << ","
                << pico.indiceFrecuencia << std::endl;
    }
    
    archivo.close();
    std::cout << "✓ Picos exportados a '" << nombreArchivo << "'" << std::endl;
}

void DetectorPicos::exportarConstelacion(
    const Resultado& resultado,
    const std::string& nombreArchivo
) {
    std::cout << "\nExportando constelación de picos..." << std::endl;
    
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo: " + nombreArchivo);
    }
    
    // Formato de constelación: (tiempo, frecuencia, magnitud)
    archivo << "# Constelación de Picos Espectrales" << std::endl;
    archivo << "# Total de picos: " << resultado.totalPicosDetectados << std::endl;
    archivo << "# Duración: " << resultado.tiempoTotal << " segundos" << std::endl;
    archivo << "# Formato: tiempo(s) frecuencia(Hz) magnitud" << std::endl;
    
    for (const auto& pico : resultado.picos) {
        archivo << std::fixed << std::setprecision(4)
                << pico.tiempo << " "
                << pico.frecuencia << " "
                << std::scientific << std::setprecision(6)
                << pico.magnitud << std::endl;
    }
    
    archivo.close();
    std::cout << "✓ Constelación exportada a '" << nombreArchivo << "'" << std::endl;
}

std::vector<DetectorPicos::Pico> DetectorPicos::filtrarPicos(
    const std::vector<Pico>& picos,
    double magnitudMinima,
    double frecuenciaMin,
    double frecuenciaMax
) {
    std::vector<Pico> picosFiltrados;
    
    for (const auto& pico : picos) {
        if (pico.magnitud >= magnitudMinima &&
            pico.frecuencia >= frecuenciaMin &&
            pico.frecuencia <= frecuenciaMax) {
            picosFiltrados.push_back(pico);
        }
    }
    
    return picosFiltrados;
}