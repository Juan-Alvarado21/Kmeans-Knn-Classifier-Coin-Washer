/*
    Clasificador K-NN mediante segmentación K-means por textura en Imagenes

    Análisis y diseño de algoritmos
    Juan Alvarado       3BV1

*/

#include <cstring>
#include <iostream>
#include <limits.h>
#include <math.h>
#include <sstream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <vector>

// Estructuras a definir

typedef unsigned char pixel[3];
typedef std::vector<std::vector<double>> Features;

typedef struct
{
  pixel pxl;
  int count;

} centroide;

typedef struct
{
  unsigned char r, g, b;
} Color;

Color colores_caracteristicos[] = {
    {0, 255, 0},     // Verde para el centroide 0
    {255, 0, 0},     // Rojo para el centroide 1
    {0, 0, 255},     // Azul para el centroide 2
    {255, 255, 0},   // Amarillo para el centroide 3
    {255, 0, 255},   // Magenta para el centroide 4
    {0, 255, 255},   // Cian para el centroide 5
    {128, 0, 0},     // Marrón para el centroide 6
    {128, 128, 128}, // Gris para el centroide 7
    {255, 165, 0},   // Naranja para el centroide 8
    {128, 0, 128}    // Púrpura para el centroide 9
};

/*     Lectura de Archivo PPM        */

// Comprueba si el archivo es PPM
int esPpm(char *);

// Revisa si la profundidad es de 8 Bits
int profundidadCorrecta(char *);

// Si la imagen es Apta
int validarPpm(char *);

// Devuelve el tamaño de la imagen
void obtenerTamanio(char *, int *, int *);

pixel **leerMatriz(char *);

void imprimirPixel(pixel);
// Escribe la imagen en formato P6
void escribirImagen(char *, pixel **, int, int);

/*     Manipulacion de Algoritmo Segmentación por K-means  */

void imprimir_centroides(centroide *, int);
centroide *generar_centroides(int);
int distancia(pixel, centroide);
int asignar_a_cluster(pixel, centroide *, int);
void K_means(pixel **, centroide *, int, int, int);
std::vector<std::vector<int>> convertirAMatrizDeEnteros(
    pixel **, int,
    int); // Recupera la matriz, dimension de grises para procesar una sola
void asignarEscalaDeGrisesAMatriz(
    pixel **, std::vector<std::vector<int>> &, int,
    int); // Asigna la matriz transformada a los pixeles correspondientes

/* Clasificador por detección de bordes       */

std::vector<std::vector<int>>
gradiente(int[3][3], std::vector<std::vector<int>> &, int, int);
void imprimir_ent(std::vector<std::vector<int>> &, int, int);
std::vector<std::vector<int>>
gradiente_total(const std::vector<std::vector<int>> &,
                const std::vector<std::vector<int>> &, int, int);
std::vector<double> calculo_hu_moments(std::vector<std::vector<int>>, int, int);
void imprimir_momentos(std::vector<double>);
void imprimir_features(Features);
double distanciaEuclidiana(const std::vector<double> &,
                           const std::vector<double> &);
double distanciaMinima(const std::vector<double> &, Features);
double distanciaMaxima(const std::vector<double> &, Features);
void procesarArchivo(const char *nombreArchivo, Features *);
void detector(std::vector<double>, Features, Features);

void entrenar_monedas(Features *);
void entrenar_rondanas(Features *);
void procesarPrueba(const char *, Features, Features);
void entrenar_prueba(Features, Features);
