# Sistema de Reconocimiento de Audio con FFT

Proyecto de reconocimiento musical, implementado en C++ utilizando FFT.

## Fase 1: Lectura de Audio y FFT Básica

### Características implementadas:
- Implementación propia del algoritmo FFT (Cooley-Tukey)
- Lectura de archivos WAV
- Conversión estéreo a mono
- Análisis de frecuencias básico

## Fase 2: Generación del Espectrograma

### Características implementadas:
- **Short-Time Fourier Transform (STFT)**: Procesamiento de audio en ventanas superpuestas de 1024 muestras con 50% de solapamiento (512 muestras)
- **Ventana de Hamming**: Aplicación automática de ventana de Hamming a cada segmento para reducir efectos de borde (spectral leakage)
- **Matriz tiempo-frecuencia**: Generación de espectrograma completo con dimensiones 1498 ventanas × 512 frecuencias
- **División en bandas de frecuencia**: Agrupación del espectro en 5 bandas personalizadas:
  - 30-40 Hz (graves muy bajos)
  - 40-80 Hz (graves bajos)
  - 80-120 Hz (graves)
  - 120-180 Hz (graves-medios)
  - 180-300 Hz (medios)
- **Exportación a CSV**: Dos archivos de salida para análisis y visualización
  - `espectrograma.csv`: Matriz completa con todas las frecuencias
  - `bandas_frecuencia.csv`: Espectrograma simplificado por bandas
