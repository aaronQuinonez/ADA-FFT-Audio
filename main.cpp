// src/main.cpp
#include <iostream>
#include "audio/AudioReader.h"
#include "fft/FFT.h"
#include <fstream>

int main(int argc, char* argv[]) {
    try {
        // Verificar argumentos
        if (argc < 2) {
            std::cout << "Uso: " << argv[0] << " <archivo.wav>" << std::endl;
            return 1;
        }
        
        std::string filename = argv[1];
        std::cout << "Leyendo archivo: " << filename << std::endl;
        
        // Paso 1: Leer archivo de audio
        AudioData audio = AudioReader::readWAV(filename);
        AudioReader::printAudioInfo(audio);
        
        // Paso 2: Preparar datos para FFT (tomar una ventana de 1024 muestras)
        int fftSize = 1024;
        if (audio.samples.size() < fftSize) {
            std::cerr << "El audio es muy corto para FFT" << std::endl;
            return 1;
        }
        
        std::vector<NumeroComplejo> fftData(fftSize);
        for (int i = 0; i < fftSize; i++) {
            fftData[i] = NumeroComplejo(audio.samples[i], 0.0);
        }
        
        // Paso 3: Aplicar FFT
        std::cout << "\nAplicando FFT..." << std::endl;
        FFT::calcular(fftData);
        
        // Paso 4: Guardar resultados
        std::ofstream outFile("fft_results.txt");
        outFile << "Frecuencia(Hz),Magnitud,Fase(rad)" << std::endl;
        
        double frequencyResolution = (double)audio.sampleRate / fftSize;
        
        for (int i = 0; i < fftSize / 2; i++) {  // Solo la mitad positiva
            double frequency = i * frequencyResolution;
            double magnitude = fftData[i].magnitud();
            double phase = fftData[i].fase();
            
            outFile << frequency << "," << magnitude << "," << phase << std::endl;
            
            // Mostrar las 20 frecuencias con mayor magnitud
            if (i < 20) {
                std::cout << "Frecuencia " << frequency << " Hz: magnitud = " 
                          << magnitude << std::endl;
            }
        }
        
        outFile.close();
        std::cout << "\nResultados guardados en fft_results.txt" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
