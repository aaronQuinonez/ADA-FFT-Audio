// src/main.cpp
#define _USE_MATH_DEFINES
#include <iostream>
#include "audio/LectorAudio.h"
#include "fft/FFT.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

int main(int argc, char* argv[]) {
    try {
        // Verificar argumentos
        if (argc < 2) {
            std::cout << "Uso: " << argv[0] << " <archivo.wav>" << std::endl;
            return 1;
        }
        
        std::string nombreArchivo = argv[1];
        std::cout << "Leyendo archivo: " << nombreArchivo << std::endl << std::endl;
        
        // Paso 1: Leer archivo de audio
        std::cout << "[DEBUG] Iniciando lectura del archivo WAV..." << std::endl;
        DatosAudio audio = LectorAudio::leerWAV(nombreArchivo);
        std::cout << "[DEBUG] Archivo leído exitosamente" << std::endl << std::endl;
        
        LectorAudio::mostrarInfoAudio(audio);
        
        // Paso 2: Encontrar el inicio del audio (saltar silencio)
        std::cout << "\n[DEBUG] Buscando inicio del audio (saltando silencio)..." << std::endl;
        
        int inicioAudio = 0;
        float umbralSilencio = 0.01f; // Considerar silencio si magnitud < 0.01
        
        for (size_t i = 0; i < audio.muestras.size(); i++) {
            if (std::abs(audio.muestras[i]) > umbralSilencio) {
                inicioAudio = i;
                break;
            }
        }
        
        std::cout << "[DEBUG] Inicio del audio detectado en muestra: " << inicioAudio << std::endl;
        std::cout << "[DEBUG] Tiempo de silencio inicial: " 
                  << (double)inicioAudio / audio.frecuenciaMuestreo << " segundos" << std::endl;
        
        // Paso 3: Preparar datos para FFT (tomar una ventana de 1024 muestras DESPUÉS del silencio)
        int tamanoFFT = 1024;
        
        if (audio.muestras.size() < static_cast<size_t>(inicioAudio + tamanoFFT)) {
            std::cerr << "El audio es muy corto para aplicar FFT después del silencio inicial" << std::endl;
            return 1;
        }
        
        std::cout << "\n[DEBUG] Preparando datos para FFT..." << std::endl;
        std::cout << "[DEBUG] Tomando muestras desde la posición " << inicioAudio 
                  << " hasta " << (inicioAudio + tamanoFFT) << std::endl;
        
        std::vector<NumeroComplejo> datosFFT(tamanoFFT);
        
        // Aplicar ventana de Hamming para reducir efectos de borde
        std::cout << "[DEBUG] Aplicando ventana de Hamming..." << std::endl;
        for (int i = 0; i < tamanoFFT; i++) {
            // Ventana de Hamming: w(n) = 0.54 - 0.46 * cos(2πn/(N-1))
            double ventanaHamming = 0.54 - 0.46 * std::cos(2.0 * M_PI * i / (tamanoFFT - 1));
            
            // Tomar muestra desde inicioAudio y aplicar ventana
            double muestra = audio.muestras[inicioAudio + i] * ventanaHamming;
            datosFFT[i] = NumeroComplejo(muestra, 0.0);
        }
        
        // Mostrar primeras muestras que se van a analizar
        std::cout << "\n[DEBUG] Primeras 20 muestras que se analizarán:" << std::endl;
        for (int i = 0; i < 20; i++) {
            std::cout << "  [" << (inicioAudio + i) << "] = " 
                      << audio.muestras[inicioAudio + i] << std::endl;
        }
        
        // Paso 4: Aplicar FFT
        std::cout << "\n[DEBUG] Aplicando FFT..." << std::endl;
        FFT::calcular(datosFFT);
        std::cout << "[DEBUG] FFT calculada exitosamente" << std::endl;
        
        // Paso 5: Guardar resultados y encontrar frecuencias dominantes
        std::cout << "\n[DEBUG] Guardando resultados..." << std::endl;
        std::ofstream archivoSalida("resultados_fft.txt");
        archivoSalida << "Frecuencia(Hz),Magnitud,Fase(rad)" << std::endl;
        
        double resolucionFrecuencia = (double)audio.frecuenciaMuestreo / tamanoFFT;
        
        // Crear vector para ordenar frecuencias por magnitud
        std::vector<std::pair<double, double>> frecuenciasMagnitudes;
        
        for (int i = 0; i < tamanoFFT / 2; i++) { // Solo la mitad positiva del espectro
            double frecuencia = i * resolucionFrecuencia;
            double magnitud = datosFFT[i].magnitud();
            double fase = datosFFT[i].fase();
            
            archivoSalida << frecuencia << "," << magnitud << "," << fase << std::endl;
            
            // Guardar para ordenar (ignorar DC component en i=0)
            if (i > 0) {
                frecuenciasMagnitudes.push_back({magnitud, frecuencia});
            }
        }
        
        // Ordenar por magnitud (descendente)
        std::sort(frecuenciasMagnitudes.begin(), frecuenciasMagnitudes.end(), 
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        
        // Mostrar las 20 frecuencias dominantes
        std::cout << "\nFrecuencias dominantes detectadas (ordenadas por magnitud):" << std::endl;
        for (int i = 0; i < std::min(20, (int)frecuenciasMagnitudes.size()); i++) {
            std::cout << "  #" << (i+1) << ": Frecuencia " << frecuenciasMagnitudes[i].second 
                      << " Hz → magnitud = " << frecuenciasMagnitudes[i].first << std::endl;
        }
        
        archivoSalida.close();
        std::cout << "\n✓ Resultados guardados en 'resultados_fft.txt'" << std::endl;
        std::cout << "✓ Fase 1 completada exitosamente" << std::endl;
        
    } catch (const std::exception& excepcion) {
        std::cerr << "Error: " << excepcion.what() << std::endl;
        return 1;
    }
    
    return 0;
}
