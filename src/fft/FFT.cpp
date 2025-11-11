#include "FFT.h"
#include <cmath>
#include <stdexcept>

const double PI = 3.14159265358979323846;

void FFT::calcular(std::vector<NumeroComplejo>& datos) {
    int n = datos.size();
    
    if (!esPotenciaDeDos(n)) {
        throw std::invalid_argument("El tamaño de la FFT debe ser una potencia de 2");
    }
    
    fftRecursivo(datos);
}

void FFT::fftRecursivo(std::vector<NumeroComplejo>& datos) {
    int N = datos.size();
    
    // Caso base: si el tamaño es 1, ya está resuelto
    if (N <= 1) {
        return;
    }
    
    // Dividir en pares e impares
    std::vector<NumeroComplejo> pares(N / 2);
    std::vector<NumeroComplejo> impares(N / 2);
    
    for (int i = 0; i < N / 2; i++) {
        pares[i] = datos[i * 2];
        impares[i] = datos[i * 2 + 1];
    }
    
    // Recursión en ambas mitades
    fftRecursivo(pares);
    fftRecursivo(impares);
    
    // Combinar resultados
    for (int k = 0; k < N / 2; k++) {
        // Factor de giro: e^(-2πik/N)
        double angulo = -2.0 * PI * k / N;
        NumeroComplejo t = NumeroComplejo::desdePolares(1.0, angulo) * impares[k];
        
        datos[k] = pares[k] + t;
        datos[k + N / 2] = pares[k] - t;
    }
}

bool FFT::esPotenciaDeDos(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

int FFT::siguientePotenciaDeDos(int n) {
    int potencia = 1;
    while (potencia < n) {
        potencia *= 2;
    }
    return potencia;
}