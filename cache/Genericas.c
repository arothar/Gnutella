/*

Aca van toas las funciones genericas que puede llegar a utilizar cualquier
modulo. Se podria renombrar Estructuras.h a Genericas.h.

*/

#include "Genericas.h"

extern char nombreModulo[LARGO_NOMBRE_MODULO];
extern int logToggle;


/* ====================================================================================== */
/* CONFIGURACION                                                                          */
/* ====================================================================================== */

void CargarConfiguracion(char *nombreArchivo) {
	char *contenido;

	contenido = LeerArchivoCompleto(nombreArchivo);

	ParsearConfiguracion(contenido);

	free(contenido);
}

void ParsearConfiguracion(char *contenido) {
	char *separadorLinea = "\n";
	char *separadorValor = "=\n";
	char *cadena;
	char cadenaOriginal[LARGO_CONFIGURACION];
	char *tokenLinea, *tokenEtiqueta, *tokenValor;
	int i = 1;

	/*cadenaOriginal = (char *) malloc(strlen(contenido) + 1);*/
	strcpy(cadenaOriginal, contenido);
	cadena = cadenaOriginal; /* Copia la direccion de la primer posicion */
 	cadena[strlen(contenido)] = '\0';

	Trim(cadena);

	while ((tokenLinea = (char *) strsep(&cadena, separadorLinea)) != NULL) {

		/* --------------------------------- */
		/* Se dija si la linea no esta vacia */
		/* --------------------------------- */
		if (tokenLinea != NULL && strlen(tokenLinea) > 0) {
			/*printf("Linea %d: '%s'\n", i, tokenLinea);*/

			tokenEtiqueta = (char *) strsep(&tokenLinea, separadorValor);
			/*LCase(tokenEtiqueta);*/
			Trim(tokenEtiqueta);

			/* ------------------------------------------------- */
			/* Se fija que la etiqueta de la linea no este vacia */
			/* ------------------------------------------------- */
			if (tokenEtiqueta != NULL && strlen(tokenEtiqueta) > 0) {
				/*printf("Nombre %d: '%s'\n", i, tokenEtiqueta);*/

				tokenValor = (char *) strsep(&tokenLinea, separadorValor);
				Trim(tokenValor);

				/* ---------------------------------- */
				/* Se fija que el valor no este vacio */
				/* ---------------------------------- */
				if (tokenValor != NULL && strlen(tokenValor) > 0) {
					/*printf("Valor %d: '%s'\n", i, tokenValor);*/

					/*punteroFuncion(tokenEtiqueta, tokenValor);*/
					AsignarConfiguracion(tokenEtiqueta, tokenValor);
				}
			}

			i++;
		}
	}

	/*free(cadenaOriginal);*/
}

/* ====================================================================================== */
/* LOG                                                                          */
/* ====================================================================================== */

int Log(char* descripcion, int tipoEvento) {
	static unsigned long int indice = 0;

	/* Primero loguea en el archivo */
	if (strlen(nombreModulo) > 0)
		FileLog(tipoEvento, descripcion, nombreModulo);


	if (tipoEvento == LOGDEBUG)
		return 0;


	if (tipoEvento != 0)
		indice++;

	printf("\r");

	if (strlen(nombreModulo) > 0)
		printf("%s.", nombreModulo);

	switch (tipoEvento) {
	case LOGINFO:
		printf("INFO: ""\e[37;40;1m""%s""\e[30;1m"" (%ld)"COLOR_POR_DEFECTO, descripcion, indice);
		break;

	case LOGDEBUG:
		printf("DEBUG: ""\e[33;40;1m""%s""\e[30;1m"" (%ld)"COLOR_POR_DEFECTO, descripcion, indice);
		break;

	case LOGERROR:
		printf("ERROR: ""\e[31;40;1m""%s""\e[30;1m"" (%ld)"COLOR_POR_DEFECTO, descripcion, indice);
		break;

       case LOGFIN:
		printf("FIN: ""\e[35;40;1m""%s""\e[30;1m"" (%ld)"COLOR_POR_DEFECTO, descripcion, indice);
		break;
	
	default:
		printf("%s", descripcion);
		break;
	}
	printf("\n");
	fflush(stdout);
	return 0;
}

int FileLog(int tipo, char *mensaje, char *nombreModulo) {
	time_t tFecha;
	char buffer[LARGO_LINEALOG];
	char nombreArchivo[LARGO_CADENA];
	char sIDProc[6];
	FILE *fArchivo=NULL;

	/* Genera el nombre del archivo */
	sprintf(nombreArchivo, "%s.log", nombreModulo);

	/* Toma la fecha y la convierte a string */
	time (&tFecha);

	/* Toma el id de proceso */
	sprintf(sIDProc, "%d", getpid());

	/* Pone la fecha */
	strcpy(buffer, ctime (&tFecha));
	buffer[strlen(buffer) - 1] = '\0';
	strcat(buffer, " ");
	strcat(buffer, nombreModulo);
	strcat(buffer, " ");
	strcat(buffer, sIDProc);

	switch (tipo) {
	case LOGINFO:
		strcat(buffer, " INFO: ");
		break;

	case LOGDEBUG:
		strcat(buffer, " DEBUG: ");
		break;

	case LOGERROR:
		strcat(buffer, " ERROR: ");
		break;

    case LOGFIN:
		strcat(buffer, " FIN: ");
		break;
    
	default:
		strcat(buffer, ": ");
		break;
	}

	strcat(buffer, mensaje);
	strcat(buffer, "\n");

	/* Abre el archivo de Logs */
	fArchivo = fopen (nombreArchivo, "a");

	if (fArchivo == NULL){
		printf("Error de apertura del archivo Log.\n");
		fflush(stdout);
		return -1;
	} else if (fprintf (fArchivo, buffer) < 0) { /* Escribo la Cadena en el archivo */
		printf ("Error al guardar archivo Log.\n");
		fflush(stdout);
		return -1;
		fclose (fArchivo);
	} else
		fclose (fArchivo);

	return 0;
}

	
void Copiar(char *destino, char *origen, int cantidad) {
	unsigned int i;

	/*LogInt("cantidad: ", cantidad, LOGDEBUG);*/

	for (i = 0; i < cantidad; i++) {
		printf("%d ", i);
		fflush(stdout);
		destino[i] = origen[i];
	}
	printf("\n\n");
	fflush(stdout);
}

int VerificarArchivo(char *archivo) {
	FILE* fArchivo = NULL;

	fArchivo = fopen(archivo, "r");
	if (fArchivo == NULL) {
		return 0;
	}
	fclose(fArchivo);
	return 1;
}

float ObtenerCarga() {
	/*int  nLong, i = 0;*/
	FILE* fArchivo  = NULL;
	char  Buffer[LARGO_CADENA];
	char *ptrLoadAvg;

	fArchivo = fopen("/proc/loadavg", "r");

	if (fArchivo == NULL) {
		return -1;
	}

	while(fgets(Buffer, LARGO_CADENA, fArchivo) != NULL) {
		ptrLoadAvg = strtok(Buffer, " ");    /*  Primera llamada => Primer token*/

		if (ptrLoadAvg != NULL) {
			fclose(fArchivo);
			return atof(ptrLoadAvg);
		}
	}

	fclose(fArchivo);
	return -1;
}


void IgnorarSeniales() {
	int i;

	for (i = 1; i < 64; i++) {
		signal(i, SIG_IGN);
	}
}

char *LeerArchivoCompleto(char *nombreArchivo) {
	FILE *file;
	char *resultado;
	int leido, total;
	char buffer[10]; /* lee de a 10 caracteres */

	file = fopen(nombreArchivo, "rt");

	/*resultado = (char *) malloc(1);*/

	resultado = (char *) malloc(LARGO_MAXIMO_ARCHIVO * sizeof(char));
	total = 0;

	while ((leido = fread(buffer, sizeof(char), 10, file)) != 0) {
		total += leido;

		/*printf("ACA: realloc(%d, %d)\n", (int)resultado, total * sizeof(char) + 1);
		fflush(stdout);

		resultado = (char *) realloc(resultado, total * sizeof(char) + 1);
		printf("DESPUES\n");
		fflush(stdout);
		*/

		strncat(resultado, buffer, leido);
	}

	if (total == 0) {
		free(resultado);
		resultado = NULL;
	} else {
		resultado[total] = '\0';
	}

	fclose(file);

	return resultado;
}

void TrimLeft(char *cadena) {
	int i, j, longitud;
	int espacios;
	char recorto;

	if (cadena == NULL)
		return;

	longitud = strlen(cadena) + 1;

	recorto = 0;
	espacios = 0;
	for (i = 0; i < longitud && !recorto; i++) {
		if (cadena[i] == ' ' || cadena[i] == '\t') {
			/* Hay que recortarlo */
			espacios++;

		} else {
			if (!recorto) { /* Si todavia no recorto */
				for (j = 0; j < (longitud - espacios); j++) {
					cadena[j] = cadena[j + espacios];
				}
				recorto = 1;
			}
		}
	}
}

void TrimRight(char *cadena) {
	int i, longitud;
	int espacios;

	if (cadena == NULL)
		return;

	longitud = strlen(cadena);

	espacios = 0;
	for (i = 0; i < longitud; i++) {
		if (cadena[i] == ' ' || cadena[i] == '\t') {
			espacios++;
		} else {
			espacios = 0;
		}
	}

	if (espacios > 0) {
		cadena[longitud - espacios] = '\0';
	}
}

void Trim(char *cadena) {
	TrimLeft(cadena);
	TrimRight(cadena);
}

void LCase(char *cadena) {
	int i, longitud;

	if (cadena == NULL)
		return;

	longitud = strlen(cadena);

	for (i = 0; i < longitud; i++) {
		if (cadena[i] >= 65 && cadena[i] <= 90) {
			cadena[i] += 32;
		}
	}
}




/* ====================================================================================== */
/* FUNCIONES DE LISTAS                                                                    */
/* ====================================================================================== */

TipoNodo* CrearElemento(int tamanio) {
	TipoNodo *nuevo;

	/* Reserva memoria para el nuevo elemento */
	if ((nuevo = (TipoNodo *) malloc(tamanio)) != NULL) {

		nuevo->siguiente = NULL;
                nuevo->dato = (TipoClienteWin *) malloc(sizeof(TipoClienteWin));

		return nuevo;
	} else {

		return NULL;
	}


}


int AgregarElemento(TipoLista *lista, TipoNodo* elemento) {
	if (elemento != NULL) {
		elemento->siguiente = NULL;

		/* Mete al final de la lista el nuevo elemento */
		if (lista->primero == NULL) {
			lista->primero = elemento;
			lista->ultimo = elemento;
		} else {
			lista->ultimo->siguiente = elemento;
			lista->ultimo = elemento;
		}

		return 1;
	} else {
		return 0;
	}
}

int AgregarElementoPila(TipoLista *lista, TipoNodo* elemento) {
	if (elemento != NULL) {
		elemento->siguiente = lista->primero;
		lista->primero = elemento;

		/* Mete al final de la lista el nuevo elemento */
		if (lista->ultimo == NULL) {
			lista->ultimo = elemento;
		}

		return 1;
	} else {
		return 0;
	}
}

TipoNodo* ExtraerPrimerElemento(TipoLista *lista) {
	TipoNodo* auxiliar = NULL;

	auxiliar = lista->primero;

	if (auxiliar != NULL) { /* La lista no estaba vacia */
		lista->primero = auxiliar->siguiente;

		if (lista->primero == NULL) { /* Si el que saque es el unico */
			lista->ultimo = NULL;
		}

		auxiliar->siguiente = NULL;
	}

	return auxiliar;
}


TipoNodo* ExtraerElemento(TipoLista *lista, void* dato, int (*punteroFuncion) (void*, void*)) {
	TipoNodo* auxiliar = NULL;
	TipoNodo* puntero = lista->primero;
	TipoNodo* anterior = NULL;

	while (auxiliar == NULL && puntero != NULL) {
		if (punteroFuncion(dato, puntero)) { /* Lo encontro */
			auxiliar = puntero;
			if (anterior != NULL) { /* El que encontre no es el primero */
				anterior->siguiente = auxiliar->siguiente;
				if (auxiliar->siguiente == NULL) {
					lista->ultimo = anterior;
				}
			} else { /* Es el primero */
				if (auxiliar->siguiente == NULL) {/* Si ademas es el ultimo, osea unico */
					lista->ultimo = NULL;
					lista->primero = NULL;
				} else {
					lista->primero = auxiliar->siguiente;
				}
			}
			auxiliar->siguiente = NULL;
		} else {
			anterior = puntero;
			puntero = puntero->siguiente;
		}
	}

	return auxiliar;
}

int func_esMismoId(void *dato,TipoNodo *elemento){
	if (strcmp(((TipoClienteWin *)dato)->IDMensaje,elemento->dato->IDMensaje) == 0)
		return 1;
	else
		return 0;
}

TipoNodo* BuscarElemento(TipoLista *lista, void* dato, int (*punteroFuncion) (void*, void*)) {
	TipoNodo* auxiliar = NULL;
	TipoNodo* puntero = lista->primero;

	while (auxiliar == NULL && puntero != NULL) {
		if (punteroFuncion(dato, puntero)) { /* Lo encontro */
			auxiliar = puntero;
		} else {
			puntero = puntero->siguiente;
		}
	}

	return auxiliar;
}

void EliminarLista(TipoLista *lista) {
	TipoNodo* auxiliar = NULL;
	TipoNodo* puntero = lista->primero;

	while (puntero != NULL) {
		auxiliar = puntero;
		puntero = puntero->siguiente;
		free(auxiliar);
	}

	free(lista);

	lista = NULL;
}


int ContarElementosLista(TipoLista *lista) {
	TipoNodo* puntero = lista->primero;
	int cantidad = 0;

	while (puntero != NULL) {
		puntero = puntero->siguiente;
		cantidad++;
	}

	return cantidad;
}
/* Arma el ID del mensaje. Vamos a tener que modificarlo para hacerlo mas generalizado (Son 3 o 4 tipos de aleatorios. Lo tengo en la cabeza) */
void ArmarId (unsigned char *Id){
   int i;

   srand((unsigned)time(NULL));

   for(i=0;i<15;i++)
	(Id[i]) = Aleatorio();
   Id[15]='\0';
}


/* Genera el Numero aleatorio. (Vamos a tener que ver el rango) */
int Aleatorio (){
   int c=0;

   while (( (c<33) || (c>127) ) && (c<161) )
	c = rand()%256;

   return c;
}
/*void MostrarLista(TipoLista *lista) {
	TipoNodo* auxiliar = lista->primero;
	int indice = 0;

	printf(" #          Pid          Nombre\n--------------------------------------------------\n");
	if (auxiliar == NULL) {
		printf("No hay elementos en la lista.\n");
	} else {
		while (auxiliar != NULL) {
			printf("%2d        %5d          %.50s\n", indice, auxiliar->pid, auxiliar->nombre);
			auxiliar = auxiliar->siguiente;
			indice++;
		}
	}

	printf("\n");
	fflush(stdout);
}*/




