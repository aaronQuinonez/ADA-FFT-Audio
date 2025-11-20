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

## Fase 3: Detección de Picos Espectrales

### Características implementadas:
- **Detección de máximos locales**: Algoritmo que identifica picos espectrales en ventanas con radio de 3 vecinos (±70 Hz)
- **Umbral adaptativo**: Cálculo automático por percentil 75 que se ajusta al contenido de cada ventana o banda de frecuencia
- **Filtrado inteligente**: Selección de top 5 picos por ventana ordenados por magnitud
- **Análisis dual**: 
  - Detección en espectrograma completo (512 frecuencias) → 7,490 picos
  - Detección por bandas de frecuencia (5 bandas) → 1,875 picos
- **Umbrales adaptativos por banda**:
  - Banda 1 (30-40 Hz): 0.059
  - Banda 2 (40-80 Hz): 0.121
  - Banda 3 (80-120 Hz): 0.434
  - Banda 4 (120-180 Hz): 0.633
  - Banda 5 (180-300 Hz): 1.535
- **Filtrado por rango**: Picos en 100-5000 Hz con magnitud >0.15 → 6,382 picos relevantes
- **Exportación múltiple**: Tres formatos de salida
  - `picos_completos.csv`: Todos los picos con precisión de frecuencia exacta
  - `picos_bandas.csv`: Picos detectados por bandas de frecuencia
  - `constelacion.txt`: Formato de texto legible para visualización (tiempo, frecuencia, magnitud)

### Estadísticas de detección:
- Total de picos detectados: 7,490 (espectrograma completo)
- Promedio de picos por ventana: 5.0 picos/ventana
- Densidad temporal: ~234 picos/segundo
- Reducción dimensional: 99% (de 767,936 puntos a 7,490 picos)
