#include "NumeroComplejo.h"

const double PI = 3.14159265358979323846;

NumeroComplejo::NumeroComplejo(double r, double i) : real(r), imaginario(i) {}

NumeroComplejo NumeroComplejo::operator+(const NumeroComplejo& otro) const {
    return NumeroComplejo(real + otro.real, imaginario + otro.imaginario);
}

NumeroComplejo NumeroComplejo::operator-(const NumeroComplejo& otro) const {
    return NumeroComplejo(real - otro.real, imaginario - otro.imaginario);
}

NumeroComplejo NumeroComplejo::operator*(const NumeroComplejo& otro) const {
    return NumeroComplejo(
        real * otro.real - imaginario * otro.imaginario,
        real * otro.imaginario + imaginario * otro.real
    );
}

double NumeroComplejo::magnitud() const {
    return std::sqrt(real * real + imaginario * imaginario);
}

double NumeroComplejo::fase() const {
    return std::atan2(imaginario, real);
}

NumeroComplejo NumeroComplejo::desdePolares(double magnitud, double angulo) {
    return NumeroComplejo(magnitud * std::cos(angulo), magnitud * std::sin(angulo));
}