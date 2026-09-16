#include <windows.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;


#define MAXPETICIONES 100
#define MAXUSUARIOS 100

int numUsuarios;
int numPeticiones;
float tReflex;

// Estructura de almacenamiento 

struct datos {
	int contPet;
	float reflex[MAXPETICIONES];
};

datos datoHilo[MAXUSUARIOS];


//------------------------------------------------------------------------
float NumeroAleatorio(float limiteInferior, float limiteSuperior) {
	float num = (float)rand();
	num = num * (limiteSuperior - limiteInferior) / RAND_MAX;
	num += limiteInferior;
	return num;
}

//------------------------------------------------------------------------
float DistribucionExponencial(float media) {
	float numAleatorio = NumeroAleatorio(0, 1);
	while (numAleatorio == 0 || numAleatorio == 1)
		numAleatorio = NumeroAleatorio(0, 1);
	return (-media) * logf(numAleatorio);
}

//------------------------------------------------------------------------
// Funcion preparada para ser un thread

DWORD WINAPI Usuario(LPVOID parametro) {

	DWORD dwResult = 0;
	int numHilo = *((int*)parametro);
	int i;
	float tiempo;

	

	srand(71 + numHilo * 2);

	datoHilo[numHilo].contPet = 0;

	// ... Resto de cosas comunes para cada usuario

	for (i = 0; i < numPeticiones; i++) {
		// PRINTF solo para depuraci�n NUNCA en medici�n
		printf("Peticion %d para el usuario %d\n", i, numHilo);
		// Hacer peticion cuando se implementen los sockets
		// ----
		// Calcular el tiempo de reflexi�n antes de la siguiente petici�n
		tiempo = DistribucionExponencial((float)tReflex);

		// Guarda los valores de la petici�n
		datoHilo[numHilo].reflex[i] = tiempo;
		datoHilo[numHilo].contPet++;

		// Espera los milisegundos calculados previamente
		Sleep((unsigned int)(tiempo*1000));
	}
	return dwResult;
}


int main(int argc, char* argv[])
{
	int i, j;
	HANDLE handleThread[MAXUSUARIOS];
	int parametro[MAXUSUARIOS];

	FILE* archivo = NULL;
	const char* nombreArchivo = "resultados.txt";
	

	// Leer por teclado los valores para realizar la prueba o asignarlos

	if (argc != 4) {
		cerr << "Numero de argumnentos incorrecto." << endl;
		cout << "USO: P1 <Numero usuarios> <Tiempo reflexion> <Numero peticiones>" << endl;
		return EXIT_FAILURE;
	}


	numUsuarios = atoi(argv[1]);
	numPeticiones = atoi(argv[2]);
	tReflex = atof(argv[3]);

	if (numUsuarios <= 0 || numUsuarios > MAXUSUARIOS) {
		cerr << "El numero de usuarios debe de ser mayor que 0 y menor que " << MAXUSUARIOS << endl;
		return EXIT_FAILURE;
	}

	if (numPeticiones <= 0 || numPeticiones > MAXPETICIONES) {
		cerr << "El numero de peticiones debe de ser mayor que 0 y menor que " << MAXPETICIONES << endl;
		return EXIT_FAILURE;
	}

	if (tReflex <= 0) {
		cerr << "El tiempo de reflexion debe de ser mayor que 0" << endl;
		return EXIT_FAILURE;
	}

		// Lanza los hilos
		for (i = 0; i < numUsuarios; i++) {
			parametro[i] = i;
			handleThread[i] = CreateThread(NULL, 0, Usuario, &parametro[i], 0, NULL);
			if (handleThread[i] == NULL) {
				cerr << "Error al lanzar el hilo" << endl;
				exit(EXIT_FAILURE);
			}
		}

	// Hacer que el Thread principal espere por sus hijos

	for (i = 0; i < numUsuarios; i++)
		WaitForSingleObject(handleThread[i], INFINITE);


	// Recopilar resultados y mostrarlos a pantalla o 
	// guardarlos en disco

	errno_t error = fopen_s(&archivo, nombreArchivo, "w");

	if (error != 0 || archivo == NULL) {
		printf("No se ha podido abrir el archivo. Código de error: %d\n", error);
		return EXIT_FAILURE;
	}

	fprintf(archivo, "RESULTADOS\n");
	for (i = 0; i < numUsuarios; i++) {
		fprintf(archivo, "-Datos del usuario numero %d:\n", i);
		fprintf(archivo, "\t-Numero de peticiones: %d\n", datoHilo[i].contPet);

		for (j = 0; j < numPeticiones; j++) {
			fprintf(archivo, "\t-Tiempo peticion %d: %.2fs\n", j, datoHilo[i].reflex[j]);
		}
	}
	fprintf(archivo, "");
	fclose(archivo);

		return 0;
}

