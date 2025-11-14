#ifndef FFT_H
#define FFT_H

#include <vector>
#include "../utilidades/NumeroComplejo.h"

class FFT {
public:
    // FFT principal usando algoritmo Cooley-Tukey
    static void calcular(std::vector<NumeroComplejo>& datos);
    
    // Verificar si un número es potencia de 2
    static bool esPotenciaDeDos(int n);
    
    // Obtener la siguiente potencia de 2
    static int siguientePotenciaDeDos(int n);
    
private:
    // Implementación recursiva del algoritmo Cooley-Tukey
    static void fftRecursivo(std::vector<NumeroComplejo>& datos);
};

#endif