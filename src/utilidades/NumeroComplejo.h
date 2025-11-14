#ifndef NUMERO_COMPLEJO_H
#define NUMERO_COMPLEJO_H

#include <cmath>

class NumeroComplejo {
public:
    double real;
    double imaginario;
    
    NumeroComplejo(double r = 0.0, double i = 0.0);
    
    // Operaciones básicas
    NumeroComplejo operator+(const NumeroComplejo& otro) const;
    NumeroComplejo operator-(const NumeroComplejo& otro) const;
    NumeroComplejo operator*(const NumeroComplejo& otro) const;
    
    // Magnitud
    double magnitud() const;
    
    // Fase
    double fase() const;
    
    // Crear desde coordenadas polares
    static NumeroComplejo desdePolares(double magnitud, double angulo);
};

#endif