// src/main.cpp
#include <iostream>
#include "audio/LectorAudio.h"
#include "fft/FFT.h"
#include <fstream>
#include <algorithm>

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
        DatosAudio audio = LectorAudio::leerWAV(nombreArchivo);
        LectorAudio::mostrarInfoAudio(audio);
        
        // Paso 2: Preparar datos para FFT (tomar una ventana de 1024 muestras)
        int tamanoFFT = 1024;
        if (audio.muestras.size() < static_cast<size_t>(tamanoFFT)) {
            std::cerr << "El audio es muy corto para aplicar FFT de tamaño " 
                      << tamanoFFT << std::endl;
            return 1;
        }
        
        std::vector<NumeroComplejo> datosFFT(tamanoFFT);
        for (int i = 0; i < tamanoFFT; i++) {
            datosFFT[i] = NumeroComplejo(audio.muestras[i], 0.0);
        }
        
        // Paso 3: Aplicar FFT
        std::cout << "\nAplicando FFT..." << std::endl;
        FFT::calcular(datosFFT);
        
        // Paso 4: Guardar resultados
        std::ofstream archivoSalida("resultados_fft.txt");
        archivoSalida << "Frecuencia(Hz),Magnitud,Fase(rad)" << std::endl;
        
        double resolucionFrecuencia = (double)audio.frecuenciaMuestreo / tamanoFFT;
        
        std::cout << "\nFrecuencias dominantes detectadas:" << std::endl;
        for (int i = 0; i < tamanoFFT / 2; i++) {  // Solo la mitad positiva del espectro
            double frecuencia = i * resolucionFrecuencia;
            double magnitud = datosFFT[i].magnitud();
            double fase = datosFFT[i].fase();
            
            archivoSalida << frecuencia << "," << magnitud << "," << fase << std::endl;
            
            // Mostrar las 20 frecuencias con mayor magnitud
            if (i < 20) {
                std::cout << "  Frecuencia " << frecuencia << " Hz: magnitud = " 
                          << magnitud << std::endl;
            }
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
