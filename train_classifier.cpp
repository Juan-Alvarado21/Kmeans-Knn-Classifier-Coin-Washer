/*
    Clasificador K-NN mediante segmentación K-means por textura en Imagenes

    Análisis y diseño de algoritmos
    Juan Manuel Alvarado        3BV1

*/

#include "implementation.h"

int main()
{

  Features monedas, rondanas;
  entrenar_monedas(&monedas);
  // imprimir_features(monedas);
  entrenar_rondanas(&rondanas);
  // imprimir_features(rondanas);

  entrenar_prueba(monedas, rondanas);

  return 0;
}
