#include "Espectrograma.h"
#include "../fft/FFT.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Espectrograma::Resultado Espectrograma::calcular(
    const DatosAudio& audio, 
    const Configuracion& config
) {
    Resultado resultado;
    
    std::cout << "\n=== Generando Espectrograma ===" << std::endl;
    std::cout << "Tamaño de ventana: " << config.tamanoVentana << " muestras" << std::endl;
    std::cout << "Solapamiento: " << config.solapamiento << " muestras ("
              << (100.0 * config.solapamiento / config.tamanoVentana) << "%)" << std::endl;
    
    // Calcular número de ventanas posibles
    int muestrasDisponibles = audio.muestras.size() - config.inicioAudio - config.tamanoVentana;
    resultado.numVentanas = (muestrasDisponibles / config.solapamiento) + 1;
    resultado.numFrecuencias = config.tamanoVentana / 2; // Solo frecuencias positivas
    resultado.frecuenciaMuestreo = audio.frecuenciaMuestreo;
    resultado.resolucionFrecuencia = (double)audio.frecuenciaMuestreo / config.tamanoVentana;
    resultado.resolucionTemporal = (double)config.solapamiento / audio.frecuenciaMuestreo;
    
    std::cout << "Número de ventanas a procesar: " << resultado.numVentanas << std::endl;
    std::cout << "Resolución de frecuencia: " << resultado.resolucionFrecuencia << " Hz/bin" << std::endl;
    std::cout << "Resolución temporal: " << resultado.resolucionTemporal << " segundos/ventana" << std::endl;
    
    // Generar ventana de Hamming si es necesario
    std::vector<double> ventana;
    if (config.aplicarHamming) {
        ventana = ventanaHamming(config.tamanoVentana);
        std::cout << "Ventana de Hamming aplicada" << std::endl;
    }
    
    // Procesar cada ventana
    std::cout << "\nProcesando ventanas..." << std::endl;
    int porcentajeAnterior = -1;
    
    for (int v = 0; v < resultado.numVentanas; v++) {
        // Mostrar progreso
        int porcentaje = (100 * v) / resultado.numVentanas;
        if (porcentaje != porcentajeAnterior && porcentaje % 10 == 0) {
            std::cout << "  Progreso: " << porcentaje << "%" << std::endl;
            porcentajeAnterior = porcentaje;
        }
        
        // Calcular posición de inicio de esta ventana
        int inicio = config.inicioAudio + (v * config.solapamiento);
        
        // Verificar que no nos salgamos del rango
        if (inicio + config.tamanoVentana > audio.muestras.size()) {
            resultado.numVentanas = v; // Ajustar número real de ventanas
            break;
        }
        
        // Preparar datos para FFT
        std::vector<NumeroComplejo> datosFFT(config.tamanoVentana);
        
        for (int i = 0; i < config.tamanoVentana; i++) {
            double muestra = audio.muestras[inicio + i];
            
            // Aplicar ventana si está configurado
            if (config.aplicarHamming) {
                muestra *= ventana[i];
            }
            
            datosFFT[i] = NumeroComplejo(muestra, 0.0);
        }
        
        // Aplicar FFT
        FFT::calcular(datosFFT);
        
        // Extraer magnitudes (solo la mitad positiva del espectro)
        std::vector<double> magnitudes(resultado.numFrecuencias);
        for (int i = 0; i < resultado.numFrecuencias; i++) {
            magnitudes[i] = datosFFT[i].magnitud();
        }
        
        resultado.magnitudes.push_back(magnitudes);
    }
    
    std::cout << "  Progreso: 100%" << std::endl;
    std::cout << "\n✓ Espectrograma generado exitosamente" << std::endl;
    std::cout << "  Dimensiones: " << resultado.magnitudes.size() << " ventanas × "
              << resultado.numFrecuencias << " frecuencias" << std::endl;
    
    return resultado;
}

std::vector<double> Espectrograma::ventanaHamming(int tamano) {
    std::vector<double> ventana(tamano);
    for (int i = 0; i < tamano; i++) {
        ventana[i] = 0.54 - 0.46 * std::cos(2.0 * M_PI * i / (tamano - 1));
    }
    return ventana;
}

void Espectrograma::exportarCSV(const Resultado& resultado, const std::string& nombreArchivo) {
    std::cout << "\nExportando espectrograma a CSV..." << std::endl;
    
    std::ofstream archivo(nombreArchivo);
    
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo: " + nombreArchivo);
    }
    
    // Escribir encabezado con frecuencias
    archivo << "Ventana,Tiempo(s)";
    for (int f = 0; f < resultado.numFrecuencias; f++) {
        double frecuencia = f * resultado.resolucionFrecuencia;
        archivo << "," << std::fixed << std::setprecision(2) << frecuencia << "Hz";
    }
    archivo << std::endl;
    
    // Escribir datos
    for (size_t v = 0; v < resultado.magnitudes.size(); v++) {
        double tiempo = v * resultado.resolucionTemporal;
        archivo << v << "," << std::fixed << std::setprecision(4) << tiempo;
        
        for (int f = 0; f < resultado.numFrecuencias; f++) {
            archivo << "," << std::scientific << std::setprecision(6) 
                    << resultado.magnitudes[v][f];
        }
        archivo << std::endl;
    }
    
    archivo.close();
    std::cout << "✓ Espectrograma exportado a '" << nombreArchivo << "'" << std::endl;
}

std::vector<std::vector<double>> Espectrograma::dividirEnBandas(
    const Resultado& resultado,
    const std::vector<std::pair<double, double>>& bandas
) {
    std::cout << "\nDividiendo espectrograma en bandas de frecuencia..." << std::endl;
    
    std::vector<std::vector<double>> bandasResultado;
    
    for (size_t v = 0; v < resultado.magnitudes.size(); v++) {
        std::vector<double> bandasVentana;
        
        for (const auto& banda : bandas) {
            double minFrec = banda.first;
            double maxFrec = banda.second;
            
            // Encontrar índices de frecuencia correspondientes
            int indiceMin = (int)(minFrec / resultado.resolucionFrecuencia);
            int indiceMax = (int)(maxFrec / resultado.resolucionFrecuencia);
            
            // Calcular magnitud promedio en esta banda
            double suma = 0.0;
            int contador = 0;
            
            for (int f = indiceMin; f <= indiceMax && f < resultado.numFrecuencias; f++) {
                suma += resultado.magnitudes[v][f];
                contador++;
            }
            
            double promedio = (contador > 0) ? (suma / contador) : 0.0;
            bandasVentana.push_back(promedio);
        }
        
        bandasResultado.push_back(bandasVentana);
    }
    
    std::cout << "✓ Espectrograma dividido en " << bandas.size() << " bandas" << std::endl;
    return bandasResultado;
}

void Espectrograma::exportarBandasCSV(
    const std::vector<std::vector<double>>& bandas,
    const std::vector<std::pair<double, double>>& definicionesBandas,
    const std::string& nombreArchivo
) {
    std::cout << "\nExportando bandas de frecuencia a CSV..." << std::endl;
    
    std::ofstream archivo(nombreArchivo);
    
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo: " + nombreArchivo);
    }
    
    // Escribir encabezado
    archivo << "Ventana";
    for (const auto& banda : definicionesBandas) {
        archivo << "," << banda.first << "-" << banda.second << "Hz";
    }
    archivo << std::endl;
    
    // Escribir datos
    for (size_t v = 0; v < bandas.size(); v++) {
        archivo << v;
        for (size_t b = 0; b < bandas[v].size(); b++) {
            archivo << "," << std::scientific << std::setprecision(6) << bandas[v][b];
        }
        archivo << std::endl;
    }
    
    archivo.close();
    std::cout << "✓ Bandas exportadas a '" << nombreArchivo << "'" << std::endl;
}