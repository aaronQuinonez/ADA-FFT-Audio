cmake_minimum_required(VERSION 3.10)
project(ReconocimientoAudioFFT)

set(CMAKE_CXX_STANDARD 17)

# Agregar archivos fuente
add_executable(reconocimiento_audio
    src/main.cpp
    src/audio/LectorAudio.cpp
    src/utilidades/NumeroComplejo.cpp
    src/fft/FFT.cpp
    src/procesamiento/Espectrograma.cpp
    src/procesamiento/DetectorPicos.cpp
)

# Incluir directorios
target_include_directories(reconocimiento_audio PRIVATE src)