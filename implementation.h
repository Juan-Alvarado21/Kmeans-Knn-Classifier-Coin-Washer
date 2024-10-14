/*
    Clasificador K-NN mediante segmentación K-means por textura en Imagenes

    Análisis y diseño de algoritmos
    Juan Manuel Alvarado        3BV1

*/

#include "classifier.h"

/**********************     Lectura de Archivo PPM        */

// Comprueba si el archivo es PPM
int esPpm(char *nombre)
{
  FILE *f = fopen(nombre, "r");
  if (!f)
  {
    printf("No se puede abrir el archivo.\n");
    return 0;
  }

  unsigned char c[2];
  fread(c, 1, 2, f);
  fclose(f);

  if (c[0] != 'P' || c[1] != '6')
  {
    printf("Archivo no es ppm P6.\n");
    return 0;
  }
  else
    return 1;
}

// Revisa si la profundidad es de 8 Bits
int profundidadCorrecta(char *nombre)
{
  FILE *f = fopen(nombre, "r");
  if (!f)
  {
    printf("No se puede abrir el archivo.\n");
    return 0;
  }

  while (getc(f) != '\n')
    ;
  while (getc(f) != '\n')
    ;

  int prof;
  fscanf(f, "%d", &prof);
  fclose(f);

  if (prof == 255)
    return 1;
  else
  {
    printf("Profundidad no es de 8 bits.\n");
    return 0;
  }
}

// Si la imagen es Apta
int validarPpm(char *nombre)
{
  return profundidadCorrecta(nombre) && esPpm(nombre);
}

// Devuelve el tamaño de la imagen
void obtenerTamanio(char *nombre, int *X, int *Y)
{
  FILE *f = fopen(nombre, "r");
  if (!f)
  {
    printf("No se puede abrir el archivo.\n");
    return;
  }

  while (getc(f) != '\n')
    ;
  fscanf(f, "%d %d", X, Y);
  fclose(f);

  if (*X != *Y)
    printf("ADVERTENCIA: tu matriz no es cuadrada.\n");
}

pixel **leerMatriz(char *nombre)
{
  FILE *f = fopen(nombre, "r");
  if (!f)
  {
    printf("No se puede abrir el archivo.\n");
    return NULL;
  }

  pixel *fila;
  int x, y;
  obtenerTamanio(nombre, &x, &y);

  for (int i = 0; i < 3; i++)
    while (fgetc(f) != '\n')
      ; // Posicionarse en la cuarta línea

  pixel **mapa = (pixel **)malloc(sizeof(pixel *) * y);

  for (int i = 0; i < y; i++)
  {
    fila = (pixel *)malloc(sizeof(pixel) * x);
    fread(fila, 3, x, f);
    mapa[i] = fila;
  }

  fclose(f);
  return mapa;
}

void imprimirPixel(pixel p) { printf("(%d,%d,%d)", p[0], p[1], p[2]); }

// Escribe la imagen en formato P6
void escribirImagen(char *nombre, pixel **mapa, int x, int y)
{
  FILE *f = fopen(nombre, "w+");

  fprintf(f, "P6\n%d %d\n255\n", x, y);
  for (int i = 0; i < y; i++)
    fwrite(mapa[i], 3, x, f);
  fclose(f);
}

/*********************    Manipulacion de Algoritmo Segmentación por K-means  */

void imprimir_centroides(centroide *nuevo, int k)
{
  for (int i = 0; i < k; i++)
  {
    printf("(%d,%d,%d)\n", nuevo[i].pxl[1], nuevo[i].pxl[2], nuevo[i].pxl[3]);
  }
  printf("\n");
}

centroide *generar_centroides(int k)
{
  centroide *nuevo = (centroide *)malloc(sizeof(centroide) * k);
  srand(time(NULL));
  for (int i = 0; i < k; i++)
  {
    nuevo[i].count = 0;
    nuevo[i].pxl[0] = rand() % 255; // R
    nuevo[i].pxl[1] = rand() % 255; // G
    nuevo[i].pxl[2] = rand() % 255; // B
  }
  // imprimir_centroides(nuevo,k);
  return nuevo;
}

int distancia(pixel pxl, centroide centroide)
{
  return sqrt(pow(pxl[0] - centroide.pxl[0], 2) +
              pow(pxl[1] - centroide.pxl[1], 2) +
              pow(pxl[2] - centroide.pxl[2], 2));
}

int asignar_a_cluster(pixel p, centroide *centroides, int k)
{
  int indice_min = 0;
  int dist_min = INT_MAX;

  for (int j = 0; j < k; j++)
  {
    int dist = distancia(p, centroides[j]);
    if (dist < dist_min)
    {
      dist_min = dist;
      indice_min = j;
    }
  }

  // printf("Pixel: (%d,%d,%d) asociado a %d\n",p[0],p[1],p[2],indice_min);

  return indice_min;
}

void K_means(pixel **mapa, centroide *centroides, int k, int n, int rango)
{
  bool cambio;
  int iteraciones = 0;
  int max_iteraciones = 10; // Número máximo de iteraciones

  for (int i = 0; i < k; i++)
  {
    centroides[i].count = 0;
    centroides[i].pxl[0] = 0;
    centroides[i].pxl[1] = 0;
    centroides[i].pxl[2] = 0;
  }

  // Inicializar centroides

  do
  {
    cambio = false;

    // Asignar cada pixel al centroide
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < n; j++)
      {
        int indice = asignar_a_cluster(mapa[i][j], centroides, k);
        // Acumular valores para el recalculo de centroides
        centroides[indice].pxl[0] = mapa[i][j][0];
        centroides[indice].pxl[1] = mapa[i][j][1];
        centroides[indice].pxl[2] = mapa[i][j][2];
        centroides[indice].count++;

        // Actualizar el color del píxel
        mapa[i][j][0] = colores_caracteristicos[indice].r;
        mapa[i][j][1] = colores_caracteristicos[indice].g;
        mapa[i][j][2] = colores_caracteristicos[indice].b;
      }
    }

    // Volver a calcular el centroide
    for (int i = 0; i < k; i++)
    {
      if (centroides[i].count > 0)
      {
        // Calcular nuevo centroide
        int nuevo_r = centroides[i].pxl[0] / centroides[i].count;
        int nuevo_g = centroides[i].pxl[1] / centroides[i].count;
        int nuevo_b = centroides[i].pxl[2] / centroides[i].count;

        // Si cambia la posicion del centroide
        if (nuevo_r != centroides[i].pxl[0] ||
            nuevo_g != centroides[i].pxl[1] ||
            nuevo_b != centroides[i].pxl[2])
        {
          cambio = true;
        }

        centroides[i].pxl[0] = nuevo_r;
        centroides[i].pxl[1] = nuevo_g;
        centroides[i].pxl[2] = nuevo_b;
      }
    }

    iteraciones++;
  } while (cambio && iteraciones < max_iteraciones);

  // Imprimir los centroides finales
  // imprimir_centroides(centroides, k);
}

// Recupera la matriz, dimension de grises para procesar una sola
std::vector<std::vector<int>> convertirAMatrizDeEnteros(pixel **mapa, int x,
                                                        int y)
{
  std::vector<std::vector<int>> matrizDeEnteros(y, std::vector<int>(x));
  for (int i = 0; i < y; i++)
  {
    for (int j = 0; j < x; j++)
    {
      int promedio = (mapa[i][j][0] + mapa[i][j][1] + mapa[i][j][2]) / 3;
      matrizDeEnteros[i][j] = promedio;
    }
  }
  return matrizDeEnteros;
}

// Asigna la matriz transformada a los pixeles correspondientes
void asignarEscalaDeGrisesAMatriz(
    pixel **mapa, std::vector<std::vector<int>> &matrizDeEnteros, int x,
    int y)
{
  for (int i = 0; i < y; i++)
  {
    for (int j = 0; j < x; j++)
    {
      int valorGris = matrizDeEnteros[i][j];
      mapa[i][j][0] = valorGris; // canal R
      mapa[i][j][1] = valorGris; // canal G
      mapa[i][j][2] = valorGris; // canal B
    }
  }
}

/************ Clasificador por detección de bordes       */

std::vector<std::vector<int>>
gradiente(int kernel[3][3], std::vector<std::vector<int>> &matrizDeEnteros,
          int x, int y)
{

  // Expandir la matrizDeEnteros

  matrizDeEnteros.insert(matrizDeEnteros.begin(), std::vector<int>(x, 0));
  matrizDeEnteros.push_back(std::vector<int>(x, 0));

  for (auto &fila : matrizDeEnteros)
  {
    fila.insert(fila.begin(), 0);
    fila.push_back(0);
  }

  std::vector<std::vector<int>> gradiente(y, std::vector<int>(x, 0));

  // Aplicar el kernel
  for (int i = 1; i <= y; ++i)
  {
    for (int j = 1; j <= x; ++j)
    {
      int sum = 0;
      for (int a = 0; a < 3; ++a)
      {
        for (int b = 0; b < 3; ++b)
        {
          sum += matrizDeEnteros[i - 1 + a][j - 1 + b] * kernel[a][b];
        }
      }
      gradiente[i - 1][j - 1] = sum;
    }
  }
  return gradiente;
}

void imprimir_ent(std::vector<std::vector<int>> &matrizDeEnteros, int x,
                  int y)
{
  for (int i = 0; i < y; i++)
  {
    for (int j = 0; j < x; j++)
    {
      printf("%d ", matrizDeEnteros[i][j]);
    }
    printf("\n");
  }
}

std::vector<std::vector<int>>
gradiente_total(const std::vector<std::vector<int>> &grad_x,
                const std::vector<std::vector<int>> &grad_y, int x, int y)
{

  std::vector<std::vector<int>> total(x, std::vector<int>(y, 0));

  for (int i = 0; i < x; i++)
  {
    for (int j = 0; j < y; j++)
    {
      // Calcular la magnitud del gradiente
      total[i][j] =
          static_cast<int>(sqrt(pow(grad_x[i][j], 2) + pow(grad_y[i][j], 2)));
    }
  }

  return total;
}

/************ K-Nearest Neighbours     */

std::vector<double> calculo_hu_moments(std::vector<std::vector<int>> imagen,
                                       int x, int y)
{

  int j, i;
  double M00 = 0, M10 = 0, M01 = 0;
  double mu20 = 0, mu02 = 0, mu11 = 0, mu30 = 0, mu03 = 0, mu21 = 0, mu12 = 0;
  double x_bar, y_bar;
  std::vector<double> huMoments(7);

  int height = y;
  int width = x;

  // Momentos de la imagen
  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      M00 += imagen[i][j];
      M10 += j * imagen[i][j];
      M01 += j * imagen[i][j];
    }
  }

  // Calcular centroides
  x_bar = M10 / M00;
  y_bar = M01 / M00;

  // Momentos centrales
  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      double x_diff = j - x_bar;
      double y_diff = x - y_bar;
      mu20 += x_diff * x_diff * imagen[i][j];
      mu02 += y_diff * y_diff * imagen[i][j];
      mu11 += x_diff * y_diff * imagen[i][j];
      mu30 += x_diff * x_diff * x_diff * imagen[i][j];
      mu03 += y_diff * y_diff * y_diff * imagen[i][j];
      mu21 += x_diff * x_diff * y_diff * imagen[i][j];
      mu12 += x_diff * y_diff * y_diff * imagen[i][j];
    }
  }

  // Momentos de HU
  huMoments[0] = mu20 + mu02;
  huMoments[1] = pow(mu20 - mu02, 2) + 4 * pow(mu11, 2);
  huMoments[2] = pow(mu30 - 3 * mu12, 2) + pow(3 * mu21 - mu03, 2);
  huMoments[3] = pow(mu30 + mu12, 2) + pow(mu21 + mu03, 2);
  huMoments[4] = (mu30 - 3 * mu12) * (mu30 + mu12) *
                     (pow(mu30 + mu12, 2) - 3 * pow(mu21 + mu03, 2)) +
                 (3 * mu21 - mu03) * (mu21 + mu03) *
                     (3 * pow(mu30 + mu12, 2) - pow(mu21 + mu03, 2));
  huMoments[5] = (mu20 - mu02) * (pow(mu30 + mu12, 2) - pow(mu21 + mu03, 2)) +
                 4 * mu11 * (mu30 + mu12) * (mu21 + mu03);
  huMoments[6] = (3 * mu21 - mu03) * (mu30 + mu12) *
                     (pow(mu30 + mu12, 2) - 3 * pow(mu21 + mu03, 2)) -
                 (mu30 - 3 * mu12) * (mu21 + mu03) *
                     (3 * pow(mu30 + mu12, 2) - pow(mu21 + mu03, 2));

  return huMoments;
}

void imprimir_momentos(std::vector<double> momentos)
{
  for (int i = 0; i < 7; i++)
  {
    printf("%f\n", momentos[i]);
  }
  printf("\n");
}

void imprimir_features(Features features)
{
  for (const auto momentos : features)
  {
    imprimir_momentos(momentos);
    printf("------\n"); // Separador para cada conjunto de momentos
  }
}

double distanciaEuclidiana(const std::vector<double> &v1,
                           const std::vector<double> &v2)
{
  double suma = 0.0;
  for (size_t i = 0; i < v1.size() && i < v2.size(); ++i)
  {
    suma += (v1[i] - v2[i]) * (v1[i] - v2[i]);
  }
  return std::sqrt(suma);
}

double distanciaMinima(const std::vector<double> &vector, Features features)
{
  double minDist = std::numeric_limits<double>::max();

  for (const auto &feature : features)
  {
    double dist = distanciaEuclidiana(vector, feature);
    if (dist < minDist)
    {
      minDist = dist;
    }
  }

  return minDist;
}

void procesarArchivo(const char *nombreArchivo, Features *features)
{

  // Variables de manipulación de imagen
  int x, y;
  pixel **mapa;

  // Kernel de transformación de Sobel
  int kernel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
  int kernel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

  // Contenedores para el procesamiento de imagen
  std::vector<std::vector<int>> matriz_ent, grad_x, grad_y, grad_total;
  std::vector<double> momentos;

  char archivoNombreC[100];

  std::strcpy(archivoNombreC, nombreArchivo);

  if (validarPpm(archivoNombreC))
  {

    obtenerTamanio(archivoNombreC, &x, &y);
    mapa = leerMatriz(archivoNombreC);
    matriz_ent = convertirAMatrizDeEnteros(mapa, x, y);
    grad_x = gradiente(kernel_x, matriz_ent, x, y);
    grad_y = gradiente(kernel_y, matriz_ent, x, y);
    grad_total = gradiente_total(grad_x, grad_y, x, y);

    momentos = calculo_hu_moments(grad_total, x, y);

    features->push_back(momentos);
    // imprimir_momentos(momentos);

    asignarEscalaDeGrisesAMatriz(mapa, grad_total, x, y);
    // std::vector<std::vector<int>> map = convertirAMatrizDeEnteros(mapa, x, y);
    // imprimir_ent(map, x, y);
    // printf("\n\n\n");

    centroide *cent = generar_centroides(3);
    K_means(mapa, cent, 3, y, 255);

    char salidaNombre[20];
    snprintf(salidaNombre, sizeof(salidaNombre), "salida_%s",
             nombreArchivo);
    // Habilitar escribir imagen en caso de que se quiera visualizar las imagenes procesadas.
    // escribirImagen(salidaNombre, mapa, x, y);

    delete[] cent;
  }
}

void procesarPrueba(const char *nombreArchivo, Features clase1,
                    Features clase2)
{

  // Variables de manipulación de imagen
  int x, y;
  pixel **mapa;

  // Kernel de transformación de Sobel
  int kernel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
  int kernel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

  // Contenedores para el procesamiento de imagen
  std::vector<std::vector<int>> matriz_ent, grad_x, grad_y, grad_total;
  std::vector<double> momentos;

  char archivoNombreC[100];

  std::strcpy(archivoNombreC, nombreArchivo);

  if (validarPpm(archivoNombreC))
  {
    obtenerTamanio(archivoNombreC, &x, &y);
    mapa = leerMatriz(archivoNombreC);

    matriz_ent = convertirAMatrizDeEnteros(mapa, x, y);
    grad_x = gradiente(kernel_x, matriz_ent, x, y);
    grad_y = gradiente(kernel_y, matriz_ent, x, y);
    grad_total = gradiente_total(grad_x, grad_y, x, y);

    momentos = calculo_hu_moments(grad_total, x, y);
    // imprimir_momentos(momentos);

    asignarEscalaDeGrisesAMatriz(mapa, grad_total, x, y);
    centroide *cent = generar_centroides(3);
    K_means(mapa, cent, 3, y, 255);
    detector(momentos, clase1, clase2);

    char salidaNombre[20];
    snprintf(salidaNombre, sizeof(salidaNombre), "salida_%s",
             nombreArchivo);
    escribirImagen(salidaNombre, mapa, x, y);

    delete[] cent;
  }
}

double distanciaMaxima(const std::vector<double> &vector, Features features)
{
  double maxDist = std::numeric_limits<double>::max();

  for (const auto &feature : features)
  {
    double dist = distanciaEuclidiana(vector, feature);
    if (dist > maxDist)
    {
      maxDist = dist;
    }
  }

  return maxDist;
}

void entrenar_monedas(Features *monedas)
{

  for (int i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << i << ".ppm";
    std::string archivoNombre = ss.str();

    procesarArchivo(archivoNombre.c_str(), monedas);
  }
}

void entrenar_prueba(Features clase1, Features clase2)
{

  printf("\n-------------------------------------------------------------------"
         "------------------\n");
  printf("Validacion del conjunto de prueba\n");
  printf("---------------------------------------------------------------------"
         "------------------\n");

  for (int i = 1; i <= 12; i++)
  {
    std::stringstream ss;
    ss << "p" << i << ".ppm";
    std::string archivoNombre = ss.str();
    procesarPrueba(archivoNombre.c_str(), clase1, clase2);
  }
}

void entrenar_rondanas(Features *rondanas)
{
  // Procesar rondanas
  for (char c1 = 'a'; c1 <= 'z'; ++c1)
  {
    procesarArchivo((std::string(1, c1) + ".ppm").c_str(), rondanas);
  }

  // Dos letras iguales
  for (char c1 = 'a'; c1 <= 'z'; ++c1)
  {
    procesarArchivo((std::string(2, c1) + ".ppm").c_str(), rondanas);
  }

  // Tres letras iguales
  for (char c1 = 'a'; c1 <= 'z'; ++c1)
  {
    procesarArchivo((std::string(3, c1) + ".ppm").c_str(), rondanas);
  }

  // Cuatro letras iguales
  for (char c1 = 'a'; c1 <= 'y'; ++c1)
  {
    procesarArchivo((std::string(4, c1) + ".ppm").c_str(), rondanas);
  }

  // Cinco letras iguales
  for (char c1 = 'a'; c1 <= 'o'; ++c1)
  {
    procesarArchivo((std::string(5, c1) + ".ppm").c_str(), rondanas);
  }
}

void detector(std::vector<double> actual, Features clase1, Features clase2)
{
  double distMin1 = distanciaMinima(actual, clase1);
  double distMin2 = distanciaMinima(actual, clase2);

  double distMax1 = distanciaMaxima(actual, clase1);
  double distMax2 = distanciaMaxima(actual, clase2);

  // Consideramos tanto la distancia mínima como la máxima para determinar la
  // pertenencia
  if (distMin1 < distMin2 && distMin1 < distMax1)
  {
    printf("PERTENECE A LA CLASE MONEDAS\n");
  }
  else if (distMin2 < distMin1 && distMin2 < distMax2)
  {
    printf("PERTENECE A LA CLASE RONDANAS\n");
  }
  else
  {
    // En caso de que no se cumpla ninguna de las condiciones anteriores
    printf("NO SE PUEDE DETERMINAR LA PERTENENCIA CON CLARIDAD\n");
  }
}
