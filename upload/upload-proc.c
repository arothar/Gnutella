/***************************************************************************
 *   Copyright (C) 2008 by Alejandro Rothar   *
 ***************************************************************************/
#include <netinet/in.h>
#include "upload-proc.h"

TipoConfiguracion Config;

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
/* LOG                                                                                    */
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
		nuevo->dato = (TipoPingLista *) malloc(sizeof(TipoPingLista));

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

int func_aleatorio(){
   int c=0;
   srand(time(NULL));
   while (( (c<33) || (c>127) ) && (c<161) )
	c = rand()%256;

   return c;
}


int enviarArchivo(int sock_descriptor,FILE *archivo, char *mensaje, int tamanioBuffer){
	int bytesleidos;
	char buffer[tamanioBuffer];
	int tamanio, bandera=1;
	
		
	while(!feof(archivo)){
		if(bandera){
			tamanio=strlen(mensaje);
			if(send(sock_descriptor,mensaje,sizeof(char)*(tamanio),0)==-1){
				Log("Hubo un error enviando el archivo.",LOGERROR);
				return -1;
			}
			bytesleidos=fread(buffer,sizeof(char),tamanioBuffer-1-tamanio,archivo);
			if(send(sock_descriptor,buffer,sizeof(char)*(bytesleidos),0)==-1){
				Log("Hubo un error enviando el archivo.",LOGERROR);
				return -1;
			}
			bandera=0;
		}
		else{
			bytesleidos=fread(buffer,sizeof(char),tamanioBuffer-1,archivo);
			if(send(sock_descriptor,buffer,sizeof(char)*(bytesleidos),0)==-1){
				Log("Hubo un error enviando el archivo.",LOGERROR);
				return -1;
			}
		}
	} 
	return 0;
}


/***************************************************************************/
/* ESTA FUNCION LA USO EN Genericas.c,  en la funcion "CargarConfiguracion"*/
/* pasandole el puntero de esta funcion para que carge con la estructura   */
/* de configuracion que le corresponde                                     */
/***************************************************************************/
void AsignarConfiguracion(char* etiqueta, char* valor) {
   if (!strcmp(etiqueta, "Nombre")) {
	strcpy(Config.nombreModulo, valor);

   } else if (!strcmp(etiqueta, "IpLocal")) {
	strcpy(Config.ipLocal, valor);

   } else if (!strcmp(etiqueta, "PuertoLocal")) {
	strcpy(Config.puertoLocal, valor);
	
   } else if (!strcmp(etiqueta, "RutaCompartida")) {
	strcpy(Config.rutaCompartida, valor);

   } else if (!strcmp(etiqueta, "TamanioBuffer")) {
	strcpy(Config.tamanioBuffer, valor);

   } else if (!strcmp(etiqueta, "PuertoGnutella")) {
	strcpy(Config.puertoGnutella, valor);

   } else if (!strcmp(etiqueta, "TTL")) {
	strcpy(Config.TTL, valor);

   } else if (!strcmp(etiqueta, "RemoteGnutellaIP_Primaria")) {
	strcpy(Config.remoteGnutellaIP_Primaria, valor);

   } else if (!strcmp(etiqueta, "RemoteGnutellaPort_Primaria")) {
	strcpy(Config.remoteGnutellaPort_Primaria, valor);

   } else if (!strcmp(etiqueta, "RemoteGnutellaIP_Secundaria")) {
	strcpy(Config.remoteGnutellaIP_Secundaria, valor);

   } else if (!strcmp(etiqueta, "RemoteGnutellaPort_Secundaria")) {
	strcpy(Config.remoteGnutellaPort_Secundaria, valor);

   } else if (!strcmp(etiqueta, "TimeSelect")) {
	strcpy(Config.timeSelect, valor);

   } else if (!strcmp(etiqueta, "NumeroDeConexiones")) {
	strcpy(Config.numeroDeConexiones, valor);

   }
}

char *archivoBuscado(char *buf){
    char* nombreArchBuscado;
    char* aux;
    int i=0;
    nombreArchBuscado = strtok(buf, " ");
    nombreArchBuscado = strtok(NULL, " ");
    
    aux = (char *) calloc(strlen(nombreArchBuscado) +1,sizeof(char));
    while (nombreArchBuscado[i+1]!='\0'){
    	aux[i] = nombreArchBuscado[i+1];
    	i++;
    }
    
    
    
    return aux;
}

void mensajeArchivoNoEncontrado(char *nombreArchivo, char **mensaje){
	int largo;
	
	largo = strlen(nombreArchivo);
	*mensaje = (char *) calloc(LARGO_ARCHIVO_BUSCADO + largo,sizeof(char));
	
	strcpy(*mensaje,"HTTP/1.1 404 Not Found \n\n <b>ERROR</b>: File ");
	strcat(*mensaje, nombreArchivo);
        strcat(*mensaje, " was not found\n");

}
void mensajeArchivoEncontrado(char *nombreArchivo,char **mensaje){
	char mensaje2[40], mensaje3[256];
	int largo;
	strcpy(mensaje3, Config.rutaCompartida);
	mensaje3[strlen(Config.rutaCompartida)-1]='\0';
	if(nombreArchivo[0]=='/')
		itoa(tamanioArchivo(&nombreArchivo[1],mensaje3,1),mensaje2,10);
	else
		itoa(tamanioArchivo(nombreArchivo,mensaje3,1),mensaje2,10);
	
	largo = strlen(nombreArchivo);
	*mensaje = (char *) calloc(LARGO_ARCHIVO_BUSCADO + largo,sizeof(char));
	strcpy(*mensaje,"HTTP/1.1 200 OK\n");
	strcat(*mensaje, "Content-Length: ");
	strcat(*mensaje, mensaje2);
	strcat(*mensaje, "\n");
	strcat(*mensaje,"Content-type: application/octet-stream\n\n");
}

char **busquedaArchivos(char *nombreArchivo, char *ruta, int *len, int *cantidadBytes){
	char comando[LARGO_ARCHIVO_BUSCADO]={};
	int archivoEncontrados;
	char **punteroArchivos=NULL;
	int tamanioDelArchivo;
	int i=0, largo, lum=0, contador=0;
	int m=0, n=0;
	char *p, *rutaLocal, *rutaLocal2;
	char armadoVector[BUFSIZ];
	
	rutaLocal=getcwd(NULL, (int)NULL);
	rutaLocal2=getcwd(NULL,(int)NULL);
	
	while((contador<3)&&rutaLocal2[lum]!='\0'){
		if(rutaLocal2[lum]=='/')
			contador++;
		lum++;
	}
	rutaLocal2[lum]='\0';
	chdir(Config.rutaCompartida);
	
	if (strlen(nombreArchivo) > 0) {
		strcpy(comando, "find ");
		strcat(comando, ruta);
		strcat(comando, " -type f -iname \"*");
		strcat(comando, nombreArchivo);
		strcat(comando, "*\" > ");
		strcat(comando, rutaLocal2);
		strcat(comando, "listadearchivos");
	
		system(comando);
		chdir(rutaLocal2);
	
		if((archivoEncontrados=open("listadearchivos", O_RDWR, 0))==-1){
			chdir(rutaLocal);
			Log("Hubo un error abriendo el archivo de resultados.",LOGERROR);
			free(rutaLocal);
			free(rutaLocal2);
			return NULL;
		}
		tamanioDelArchivo=tamanioArchivo("listadearchivos", rutaLocal2,0);
		comando[0]='\0';
		
		while((read(archivoEncontrados, armadoVector, BUFSIZ) >0)&&i<255){
			p=&armadoVector[0];
			n=m=0;
			while((m!=strlen(armadoVector))&&(i<254)&&(tamanioDelArchivo>=0)){
				
				while((n!=strlen(armadoVector))&&tamanioDelArchivo>0){
					if(*p=='\n'){
						p++;
						n++;
						
						goto Seguir;
					}
					else
						if( *p=='\0'){
							p++;
							n++;
							
							goto Seguir;
						}
					p++;
					n++;
					tamanioDelArchivo--;
				}
				Seguir:
				tamanioDelArchivo--;
				comando[0]='\0';
				if(tamanioDelArchivo>=0){
					strncat(comando, &armadoVector[m], n-m);
					comando[n-m]='\0';
					m=n;
					if(m<=strlen(armadoVector)){
						if(i==0)
							punteroArchivos = (char**)realloc(punteroArchivos, 256*sizeof(char*));
						punteroArchivos[i]=NULL;
						punteroArchivos[i+1] = NULL;
						largo=strlen(comando);
						if(comando[largo-1]=='\n')
							comando[--largo]='\0';
						if(comando[largo-1]!='/'){
							punteroArchivos[i]= (char *)realloc(punteroArchivos[i], (largo+1)*sizeof(char));
							
							strcpy(punteroArchivos[i], comando);
							strcat(punteroArchivos[i], "\0");
							comando[0]='\0';
							*cantidadBytes+=strlen(punteroArchivos[i]);
						}
						else{
							comando[0]='\0';
						}
					}
					i++;
				}
			}
		}
		if(comando[0]!='\0'){
			/*punteroArchivos = (char**)realloc(punteroArchivos, (i+2)*sizeof(char*));*/
			punteroArchivos[i]=NULL;
			punteroArchivos[i+1] = NULL;
			largo=strlen(comando);
			punteroArchivos[i]= (char *)realloc(punteroArchivos[i], (largo+1)*sizeof(char));
			
			strcpy(punteroArchivos[i], comando);
			strcat(punteroArchivos[i], "\0");
			
			*cantidadBytes+=strlen(punteroArchivos[i]);
			
		}
			
			
		close(archivoEncontrados);
		chdir(rutaLocal2);
		strcpy(comando, "rm listadearchivos");
		system(comando);
		chdir(rutaLocal);
	}
	if(punteroArchivos==NULL){
		return punteroArchivos;
	}
	*len=i;
	
	free(rutaLocal);
	free(rutaLocal2);
	return punteroArchivos;
}

void liberarPuntero(char **puntero){
	int i=0;
	while(puntero[i]!=NULL){
		free(puntero[i]);
		i++;
	}
	free(puntero[i]);
	free(puntero);
	
}

int tamanioArchivo(char *punteroArchivos, char *ruta, int numero){
	struct stat stbuf;
	char directorio[256];
	char archivo[256];
	
	strcpy(directorio, ruta);
	
	strcpy(archivo, punteroArchivos);
	archivo[strlen(punteroArchivos)]='\0';
	if(numero){
		if(strncmp(directorio, Config.rutaCompartida, strlen(Config.rutaCompartida))!=0)
			chdir(Config.rutaCompartida);
		else{
			if(chdir(directorio)){
				return -1;
			}
		}
	}
	else{
		chdir(directorio);
	}
	if ( stat(archivo, &stbuf) < 0 ){
		fprintf(stderr, "tamanioArchivo: no se tiene acceso a %s\n", punteroArchivos);
		return -1;
	}
	else{
		 
		 /*strcpy(ruta, directorio);
		 strcpy(punteroArchivos, archivo);*/
		 return stbuf.st_size;
	}

	return 0;
}

void achicarCadena(char *cadena){
	while(cadena[strlen(cadena)-1]=='\n')
		cadena[strlen(cadena)-1]='\0';
}

void itoa(int value, char *str, int base){
	static char num[]= "0123456789abcdefghijklmnopqrstuvwxyz";
	char *wstr=str;

	do{
		*wstr++=num[value%base];
	}while(value/=base);

	*wstr='\0';
	strreverse(str,wstr-1);
}

void strreverse(char *begin, char *end){
	char aux;
	while(end>begin){
		aux=*end;
		*end--=*begin;
		*begin++=aux;
	}

}
/*
void ordenarVector (int *tabla, int *n){
	int i,j;

	if ((tabla == NULL) || ((*n) == 0))
		return;

	j=0;
	for (i=0; i<(*n); i++)
	{
		if (tabla[i] != -1)
		{
			tabla[j] = tabla[i];
			j++;
		}
	}
	
	*n = j;
}

int tamanioTabla (int *tabla, int n){
	int i;
	int max;

	if ((tabla == NULL) || (n<1))
		return 0;
		
	max = tabla[0];
	for (i=0; i<n; i++)
		if (tabla[i] > max)
			max = tabla[i];

	return max;
}
*/
void envioRespuesta(int *sock_clienT, char buf[16540]){
	char *nombreArchBuscado;
	char *pathArchivoBuscado;
	FILE *archivo;
	char *mensaje;
	int tamanioBuffer;
	int sock_cliente;
	
	sock_cliente=*sock_clienT;
	
	/*************************************************/
	/*Recibe datos del cliente */
	/*************************************************/

	
		/**********************************************************************/
		/*Obtengo el nombre del archivo que tengo que buscar en mi directorio*/
		/*********************************************************************/
		if((nombreArchBuscado = archivoBuscado(buf)) == NULL){
			Log("El formato del mensaje es incorrecto.",LOGERROR);
			exit(1);
		}
		if(nombreArchBuscado[0]=='/')
			strcpy(nombreArchBuscado, &nombreArchBuscado[1]);
		/******************************************************/
		/*Arma la ruta completa al archivo                    */
		/******************************************************/
		pathArchivoBuscado = (char *) calloc(strlen(Config.rutaCompartida) + LARGO_ARCHIVO_BUSCADO, sizeof(char));
		puts(nombreArchBuscado);
		strcpy(pathArchivoBuscado,Config.rutaCompartida);
		/*Parche por el mal fin de archivo que se pueda producir*/
		if(pathArchivoBuscado[strlen(Config.rutaCompartida)-2]=='/')
			pathArchivoBuscado[strlen(Config.rutaCompartida)-1]='\0';
		strcat(pathArchivoBuscado,nombreArchBuscado);
		puts(pathArchivoBuscado);
		/******************************************************/
		/*Abre el archivo, si no lo encuentra manda el mensaje*/
		/******************************************************/
		archivo=fopen(pathArchivoBuscado,"rb");
		
		if(!archivo){
			int lenlen;
			/******************************************************/
			/*No lo encontro, manda el mensaje de no encontrado   */
			/******************************************************/
			
			mensajeArchivoNoEncontrado(nombreArchBuscado,&mensaje);
			
			if((lenlen=send(sock_cliente,mensaje,strlen(mensaje),0))==-1){
				Log("No se ha encontrado el archivo.",LOGERROR);
			}
			
			
			
		}else{
			
			/*************************************************/
			/*Envia el archivo al cliente                    */
			/*************************************************/
			mensajeArchivoEncontrado(nombreArchBuscado,&mensaje);
			tamanioBuffer = atoi(Config.tamanioBuffer);
			
			if (enviarArchivo(sock_cliente,archivo,mensaje, tamanioBuffer) != -1)
				Log("Se ha completado la descarga del archivo", LOGINFO);
			fclose(archivo);
			
		}
		free(mensaje);
		free(nombreArchBuscado);
		free(pathArchivoBuscado);
		close(sock_cliente);
	
}


void func_envioRespuestaGNUTELLA(int sock_clienT, int *port,char *nombreModulo, ListaQuery **listaQuery, ListaPing **listaPing, ListaVecino **listaVecino, ListaGnu **listaGnu, int *leng, int *chequeador, int vectorDesconectados[10] , int *conexionGnu){
	TipoHeaderTP buf;
	TipoQuery bufferQuery;
	TipoPingPong bufferPingPong;
	char bufferSalida[16540]={};
	
	char nombreArchBuscado[400]={};
	char **vecArchivosBuscados;
	
	int sock_cliente, socket_envio;
	int len;
	int i=0, p=11, k=0;
	char *nom;
	char *rutaArch;
	
	char respaldoNombre[300];
	char respaldoDireccion[300];
	
	int tam;
	char destino[40];
	int x,M, apuntador;
	/****************************/
	char ayuda[9]="00000000";
	char ayuda2[3]="00";
	int largo;
	/****************************/
	sock_cliente=sock_clienT;
	int cantidadBytes=0;
	int w=0, comprobar_isla=1;
	int comportaFuncion=0;
	int torpe=0;
	char *rutaLocal2;
	/*************************************************/
	/*Recibe datos del cliente */
	/*************************************************/
	
	rutaLocal2=getcwd(NULL,(int)NULL);
	
	signal(SIGPIPE, SIG_IGN);
	
	if((recv(sock_cliente,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0)) <= 0){
		*leng= -1;
	}
	else{
		
		if(buf.ucTipo==QUERY){
			Log("Tratando Query.",LOGINFO);
			*chequeador=2;
			if(recv(sock_cliente,(TipoQuery*)&bufferQuery,sizeof(TipoQuery),0) <= 0){
				chdir(rutaLocal2);
				Log("Error recibiendo el cuerpo del Query.",LOGERROR);
				*leng= -1;
			}
			else{
				if(func_buscarQuery(buf, listaQuery)==-1){
					func_agregarQuery(buf , bufferQuery , listaQuery, sock_cliente);
					
					if(bufferQuery.sArchBuscado[0]!=0x00){
						Log("El Query contiene informacion correcta.",LOGINFO);
						
						strcpy(nombreArchBuscado,bufferQuery.sArchBuscado);
						
						vecArchivosBuscados=busquedaArchivos(nombreArchBuscado, Config.rutaCompartida, &len, &cantidadBytes);
						
						if(vecArchivosBuscados!=NULL){
							rutaArch=NULL;
							Log("Hay archivos disponibles para bajar.",LOGINFO);
							while(vecArchivosBuscados[i]!=NULL){
								
								
								nom=buscarNombre(vecArchivosBuscados[i], &rutaArch);
								
								strcpy(respaldoNombre,nom);
								
								strcpy(respaldoDireccion, rutaArch);
									
								tam=tamanioArchivo(nom, rutaArch, 1);
								
								
								strcpy(nom, respaldoNombre);
								strcpy(rutaArch, respaldoDireccion);
								
								sprintf( destino, "%x", tam );
								
								
								k=0;
								while(k==8){
									ayuda[k]='0';
									k++;
								}
								if((largo=strlen(destino))<9){
									k=0;
									while(destino[k]!='\0'){
										ayuda[8-largo]=destino[k];
										k++;
										largo--;
									}
									
									ayuda[8]='\0';
								}
								
								
								/*Creacion del File Index*/
								bufferSalida[p++]=0x50;
								bufferSalida[p++]=0x50;
								bufferSalida[p++]=0x50;
								bufferSalida[p++]=0x50;
								
								/*Creacion del File Size*/
								strncpy(ayuda2,&ayuda[0],2);
								bufferSalida[p++]=x=atoh(ayuda2);
								
								strncpy(ayuda2,&ayuda[2],2);
								bufferSalida[p++]=x=atoh(ayuda2);
								
								strncpy(ayuda2,&ayuda[4],2);
								bufferSalida[p++]=x=atoh(ayuda2);
								
								strncpy(ayuda2,&ayuda[6],2);
								bufferSalida[p++]=x=atoh(ayuda2);
								
								/*Creacion de la ruta al archivo*/
								strcpy(&bufferSalida[p], &respaldoDireccion[strlen(Config.rutaCompartida)-1]);
								p+=strlen(&respaldoDireccion[strlen(Config.rutaCompartida)-1]);
								/*Inclusion del nombre del archivo*/
								strcpy(&bufferSalida[p], respaldoNombre);
								p+=strlen(respaldoNombre);
								/*cvArchivos[strlen(&cvArchivos[8])+8]='\0';
								cvArchivos[(m+=(strlen(&cvArchivos[8])+9))]='\0';*/
								bufferSalida[p++]='\0';
								bufferSalida[p++]='\0';
								
								i++;
								
								
								rutaArch=NULL;
							}
							
							
							
							while(vecArchivosBuscados[torpe]!=NULL){
								free(vecArchivosBuscados[torpe]);
								torpe++;
							}
							free(vecArchivosBuscados[torpe]);
							free(vecArchivosBuscados);
							
						}
						else{
							chdir(rutaLocal2);
							Log("No se han encontrado archivos para bajar.",LOGINFO);
						}
						
						
						bufferSalida[0]=len;
						
						func_armadoRespuesta((char *)&bufferSalida, &p, destino, ayuda, ayuda2);
						
						buf.ucTipo=QUERYHIT;
						buf.nLongMensaje=p;
						chdir(rutaLocal2);
						if(send(sock_cliente,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
							chdir(rutaLocal2);
							Log("Error enviando el header del QueryHit.",LOGERROR);
							*leng= -1;
							
						}
						else{
							if(send(sock_cliente,bufferSalida,p,0) == -1){
								chdir(rutaLocal2);
								Log("Error enviando el cuerpo del QueryHit.",LOGERROR);
								*leng= -1;
							}
							else{
								chdir(rutaLocal2);
								/*puts(rutaLocal2);*/
								Log("Se ha enviado el QueryHit.",LOGINFO);
								apuntador=0;
								*chequeador=-2;
								torpe=0;
								while(torpe<16540){
									bufferSalida[torpe]='\0';
									torpe++;
								}
								buf.ucTipo=QUERY;
								
								if(buf.TTL>0){
									buf.Hops++;
									buf.TTL--;
									while((M=func_buscarListaVecino(buf, listaVecino, &apuntador, sock_cliente))>=0){
										comprobar_isla=0;
										buf.ucTipo=QUERY;
										if(send(M,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
											chdir(rutaLocal2);
											Log("Error enviando el header del Query al vecino.",LOGERROR);
											func_eliminarVecino(listaVecino, M, conexionGnu);
											
											w=0;
											while(vectorDesconectados[w]!=0)
												w++;
											vectorDesconectados[w]=M;
											
										}
										else{
											if(send(M,(TipoQuery*)&bufferQuery,sizeof(TipoQuery),0) == -1){
												chdir(rutaLocal2);
												Log("Error enviando el cuerpo del Query al vecino.",LOGERROR);
												func_eliminarVecino(listaVecino, M, conexionGnu);
												
												w=0;
												while(vectorDesconectados[w]!=0)
													w++;
												vectorDesconectados[w]=M;
											}
											else{
												chdir(rutaLocal2);
												Log("Se reenvio correctamente el Query al vecino.",LOGINFO);
											}
										}
									}
									if(comprobar_isla){
										*leng= -2;
										*chequeador=-2;
									}
								}
							}
						}
					}
					else{
						Log("El cuerpo del Query no tiene una informacion correcta.",LOGINFO);
						buf.ucTipo=QUERYHIT;
						*chequeador=-2;
						if(send(sock_cliente,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
							Log("Error enviando el header del mensaje de error.",LOGERROR);
							*leng =-1;
						}
						else{
							if(send(sock_cliente,GNUTELLA_SRV_ERROR,strlen(GNUTELLA_SRV_ERROR),0)==-1){
								Log("Error enviando el mensaje de error.",LOGERROR);
							}
							else
								Log("Se ha enviado correctamente el mensaje de error.",LOGINFO);
						}
					}
				}
			}
		}
		else{
			
			if(buf.ucTipo==PING){
				int apuntador=0;
				if(recv(sock_cliente,(TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0) <= 0){
					Log("Error recibiendo cuerpo del Ping.",LOGERROR);
					*leng= -1;
				}
				Log("Se ha recibido un Ping.",LOGINFO);
				if((M=func_buscarListaPing(buf, listaPing, &sock_cliente, &comportaFuncion))<0){
					
					func_agregarListaPing(buf, bufferPingPong, listaPing,sock_cliente);
					if(buf.Hops==0)
						func_agregarListaVecino(buf, bufferPingPong, listaVecino, sock_cliente);
					buf.Hops++;
					buf.TTL--;
					if(buf.TTL>0){
						
						if(buf.Hops>=2)
							func_agregarListaGnu(bufferPingPong, listaGnu);
						
						if(M<0){
							char *punteroIP;
							char ayudar[16];
							
							
							while((M=func_buscarListaVecino(buf, listaVecino, &apuntador, sock_cliente))>=0){
								
								if(send(M,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
									Log("Error enviando header Ping a un vecino.",LOGERROR);
									func_eliminarVecino(listaVecino, M, conexionGnu);
									
									w=0;
									while(vectorDesconectados[w]!=0)
										w++;
									vectorDesconectados[w]=M;
								}
								else{
									if(send(M,(TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0) == -1){
										Log("Error enviando cuerpo Ping a un vecino.",LOGERROR);
										func_eliminarVecino(listaVecino, M, conexionGnu);
										
										w=0;
										while(vectorDesconectados[w]!=0)
											w++;
										vectorDesconectados[w]=M;
									}
									else{
										Log("Se ha reenviado el Ping a los vecinos.",LOGINFO);
									}
								}
							}
							
							
							
							
							
							*leng=-2;
							strcpy(ayudar, Config.ipLocal);
							
							punteroIP=strtok(ayudar, ".");
							bufferPingPong.ipLocal[0]=atoi(punteroIP);
							punteroIP=strtok(NULL, ".");
							bufferPingPong.ipLocal[1]=atoi(punteroIP);
							punteroIP=strtok(NULL, ".");
							bufferPingPong.ipLocal[2]=atoi(punteroIP);
							punteroIP=strtok(NULL, ".");
							bufferPingPong.ipLocal[3]=atoi(punteroIP);
							
							bufferPingPong.puerto=atoi(Config.puertoGnutella);
							buf.ucTipo=PONG;
							if(send(sock_cliente,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
								Log("Error enviando el header Pong cuando recibe un Ping...",LOGERROR);
								func_eliminarVecino(listaVecino, sock_cliente, conexionGnu);
								w=0;
								while(vectorDesconectados[w]!=0)
									w++;
								vectorDesconectados[w]=M;
							}
							else{
								if(send(sock_cliente,(TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0) == -1){
									Log("Error enviando el cuerpo Pong cuando recibe un Ping.",LOGERROR);
									func_eliminarVecino(listaVecino, sock_cliente, conexionGnu);
									w=0;
									while(vectorDesconectados[w]!=0)
										w++;
									vectorDesconectados[w]=M;
								}
								else{
									Log("Se ha enviado el Pong correctamente.",LOGINFO);
								}
							}
							
							*chequeador=-2;
						}
					}
					else{
						/*if(send(sock_cliente,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
							Log("Error enviando header Pong.",LOGERROR);
							
						}
						else{
							if(send(sock_cliente,(TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0) == -1){
								Log("Error enviando mensaje el cuerpo QUERYHIT...",LOGERROR);
							}
						}*/
						Log("Se ha llegado al tope del TTL. No se envia Ping.",LOGINFO);
						*leng=-2;
						*chequeador=-2;
					}
				}
				else{
					/*if(send(M,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
						perror("Error enviando mensaje de encabezado M... ");
						puts("ERROR");
						func_eliminarVecino(listaVecino, M);
						*chequeador=-1;
						w=0;
						while(vectorDesconectados[w]!=0)
							w++;
						vectorDesconectados[w]=M;
					}
					else{
						if(send(M,(TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0) == -1){
							perror("Error enviando mensaje de cuerpo M... ");
							puts("ERROR");
							func_eliminarVecino(listaVecino, M);
							*chequeador=-1;
							w=0;
							while(vectorDesconectados[w]!=0)
								w++;
							vectorDesconectados[w]=M;
						}
					}*/
					Log("Se corto la cadena del Ping por estar en lista Pings.",LOGINFO);
					*leng=-2;
					*chequeador=-2;
				}
			}
			else{
				if(buf.ucTipo==PONG){
					int M, lenHops;
					
					if(recv(sock_cliente, (TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0)==-1){
						Log("Error recibiendo header del Pong...",LOGERROR);
					}
					else{
						Log("Se ha recibido correctamente el Pong.",LOGINFO);
						M=func_buscarListaPing(buf, listaPing, &comportaFuncion, &lenHops);
						
						
						if(M<0){
							/*buf.TTL--;
							buf.Hops++;
							if(send(sock_cliente,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
								Log("Error enviando header Pong...",LOGERROR);
							}
							else{
								if(send(sock_cliente,(TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0) == -1){
									Log("Error enviando cuerpo del Pong...",LOGERROR);
								}
							}*/
							if(buf.Hops==1){
								func_agregarListaVecino(buf, bufferPingPong, listaVecino, sock_cliente);
							
							}
							else{
								if(buf.Hops>1)
									func_agregarListaGnu(bufferPingPong, listaGnu);
							}
							(*conexionGnu)++;
							Log("Se recibio correctamente el Pong.",LOGINFO);
							*leng= 0;
							*chequeador=-2;
						}
						else{
							if(buf.Hops==1){
								
								func_agregarListaVecino(buf, bufferPingPong, listaVecino, sock_cliente);
							
							}else{
								if(buf.Hops>1){
									
									func_agregarListaGnu(bufferPingPong, listaGnu);
									/*
									buf.TTL--;
									buf.Hops++;
									*/
									if(lenHops>=0){
										if(send(M,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
											Log("Error enviando encabezado Pong...",LOGERROR);
										}
										else{
											if(send(M,(TipoPingPong*)&bufferPingPong,sizeof(TipoPingPong),0) == -1){
												Log("Error enviando el cuerpo Pong...",LOGERROR);
											}
											else{
												Log("Se reenvió en Pong correctamente.",LOGINFO);
											}
										}
									}
									else{
										Log("Se recibio correctamente el Pong.",LOGINFO);
									}
									*chequeador=-2;
								}
							}
						}
					}
					
					
				}
				else{
					if(buf.ucTipo==QUERYHIT){
						int num;
						if((num=recv(sock_cliente,bufferSalida,16540,0)) <= 0){
							Log("Error recibiendo cuerpo del QUERYHIT...",LOGERROR);
							*leng= -1;
						}
						Log("Se recibio un QueryHit.",LOGINFO);
						socket_envio=func_buscarQuery(buf, listaQuery);
						if(socket_envio==-1)
							Log("El mensaje no tiene destinatario, se descarta el mensaje.",LOGINFO);
						else{
							
							if(send(socket_envio,(TipoHeaderTP*)&buf,sizeof(TipoHeaderTP),0) == -1){
								Log("Error enviando mensaje el encabezado QUERYHIT...",LOGERROR);
							}
							else{
								if(send(socket_envio,bufferSalida,num,0) == -1){
									Log("Error enviando mensaje el cuerpo QUERYHIT...",LOGERROR);
								}
								else{
									Log("Se ha reenviado correctamente el QUERYHIT al destinatario.",LOGINFO);
								}
							}
						}
						*chequeador=-2;
					}
				}
			}
		}
	}
	
}

char *buscarNombre(char *vecArchivosBuscados,char **rutaArch){
	char *nombre="l";
	char *destruir;
	char guardar[300];
	char rito[3000];
	int i=1, m;
	int largo=0;
	if(vecArchivosBuscados[i=(strlen(vecArchivosBuscados)-1)]=='\n'){
		vecArchivosBuscados[i]='\0';
		i=1;
	}
	
	
	if(!strncmp(vecArchivosBuscados, Config.rutaCompartida, (largo=strlen(Config.rutaCompartida))))
		strcpy(guardar,&vecArchivosBuscados[largo]);
	else
		strcpy(guardar,vecArchivosBuscados);
	
	i=1;
	
	while(nombre!=NULL||i){
		/*puts(nombre);*/
		if(i==1){
			nombre=strtok(guardar, "/");
			i++;
		}
		else
			nombre=strtok(NULL, "/\n\0");
		if(nombre!=NULL){
			destruir=nombre;
			m=strlen(nombre);
			largo+=m;
			
			nombre=NULL;
		}
		else{i=0;}
	}
	
	strncpy(rito, vecArchivosBuscados,(largo=(strlen(vecArchivosBuscados)-m)));
	rito[largo]='\0';
	*rutaArch=&rito[0];
	
	return destruir;
}

int atoh(char* buff){
	int accum = 0;

	for(;;buff++){
		if(*buff >= '0' && *buff <= '9'){
			accum <<= 4;
			accum += *buff - '0';
		}
		else
			if(*buff >= 'a' && *buff <= 'f'){
				accum <<= 4;
				accum += *buff - 'a' + 10;
			}
			else
				break;
	}
	return accum;
}

void func_armadoRespuesta(char *bufferSalida, int *m, char *destino, char *ayuda, char *ayuda2){
	int largo, k,x;
	extern char IDServer[16];
	
	strcpy(ayuda, "00000000");
	sprintf(destino,"%x", atoi(Config.puertoGnutella));
	if((largo=strlen(destino))<=4){
		k=0;
		while(destino[k]!='\0'){
			ayuda[4-largo]=destino[k];
			k++;
			largo--;
		}
		ayuda[4]='\0';
	}
	
	
	strncpy(ayuda2,&ayuda[0],2);
	bufferSalida[1]=x=atoh(ayuda2);
	
	strncpy(ayuda2,&ayuda[2],2);
	bufferSalida[2]=x=atoh(ayuda2);
	
	
	strcpy(ayuda, "00000000");
	
	func_armadoIPQueryHit(bufferSalida);
	
	
	bufferSalida[7]=0x30;
	bufferSalida[8]=0x30;
	bufferSalida[9]=0x30;
	bufferSalida[10]=0x30;
	
	memcpy(&bufferSalida[(*m)], IDServer, 16);
	(*m)+=16;
	
}

void func_armadoIPQueryHit(char *bufferSalida){
	char destruir[16]="";
	char *p;
	
	strcpy(destruir, Config.ipLocal);
	
	p=strtok(destruir, ".");
	bufferSalida[3]=atoi(p);
	p=strtok(NULL, ".");
	bufferSalida[4]=atoi(p);
	p=strtok(NULL, ".");
	bufferSalida[5]=atoi(p);
	p=strtok(NULL, ".\0");
	bufferSalida[6]=atoi(p);
	
}

void V(int *n){
	n++;
}

void P(int *n){
	while(n<=0);
	n--;
}

TipoPingPong func_ConfiguracionPingPong(){
	TipoPingPong pingPong;
	char *p;
	char ayuda[16], buffer[BUFSIZ], comando[256];
	int sis, numero;
	
	strcpy(ayuda, Config.ipLocal);
	pingPong.puerto=atoi(Config.puertoGnutella);
	p=strtok(ayuda, ".");
	pingPong.ipLocal[0]=atoi(p);
	p=strtok(NULL, ".");
	pingPong.ipLocal[1]=atoi(p);
	p=strtok(NULL, ".");
	pingPong.ipLocal[2]=atoi(p);
	p=strtok(NULL, ".");
	pingPong.ipLocal[3]=atoi(p);
	
	strcpy(comando, "find ");
	strcat(comando, Config.rutaCompartida);
	strcat(comando, " -type f | wc -l ");
	strcat(comando, "> cantidadArchivos");
	
	system(comando);
	
	sis=open("cantidadArchivos", O_RDONLY,0);
	while(read(sis, buffer,BUFSIZ)>0)
		numero=atoi(buffer);
	close(sis);
	
	pingPong.numeroDeArchivosCompartidos=numero;
	
	system("rm cantidadArchivos");
	
	strcpy(comando, "du -s ");
	strcat(comando, Config.rutaCompartida);
	strcat(comando, "> cantidadKBArchivos");
	
	system(comando);
	
	sis=open("cantidadKBArchivos", O_RDONLY,0);
	while(read(sis, buffer,BUFSIZ)>0)
		numero=atoi(buffer);
	close(sis);
	
	pingPong.numeroDeKBCompartidos=numero;
	system("rm cantidadKBArchivos");
	return pingPong;
}

/*************************************************************************/
/* Esta funcion se usa para hacer el ping y pong cuando se esta levantando
   este servidor*/
/*************************************************************************/
int func_comunicacionPingPong(int *isocket_pingPong, ListaPing **listaPing, ListaVecino **listaVecino, ListaGnu **listaGnu, int manipulador, int *corredor){
	int i=0, num;
	struct sockaddr_in sock_pingPong;
	TipoPingPong pingPong;
	struct hostent *stcNombre_host;
	TipoHeaderTP descriptor;
	int comportamiento=0, len;
	char buffer[1024];
	ListaGnu *nuevo;
	int numeroID;
	extern char IDServer[16];
	srand(time(NULL));
	while(i<16){
		do{
			numeroID=(rand()/256);
		}while( (numeroID<48) || (numeroID>57 && numeroID<65) || numeroID>122);
		descriptor.IDMensaje[i]=numeroID;
		IDServer[i]=numeroID;
		i++;
	}
	Error:;
	if(!manipulador){
		if(comportamiento==0){
			
		
			if((stcNombre_host=gethostbyname(Config.remoteGnutellaIP_Primaria))==NULL){
				Log("Ocurrio un error con la funcion gethostbyname en IP Primaria",LOGERROR);
				return 1;
			}
			
			if((*isocket_pingPong=socket(AF_INET, SOCK_STREAM, 0))==-1){
				Log("Ocurrio un error al abrir el socket en IP Primaria",LOGERROR);
			}
			sock_pingPong.sin_family= AF_INET;
			sock_pingPong.sin_port = htons(num=atoi(Config.remoteGnutellaPort_Primaria));
			/*sock_pingPong.sin_addr.s_addr= inet_addr(Config.remoteGnutellaIP_Primaria);*/
			sock_pingPong.sin_addr = *((struct in_addr *)stcNombre_host->h_addr_list[0]);
			bzero((char *)&(sock_pingPong.sin_zero), 8);
			if(connect(*isocket_pingPong, (struct sockaddr *)&sock_pingPong, sizeof(sock_pingPong))==-1){
				comportamiento++;
				goto Error;
			}
			
			
		}
		else{
			if(comportamiento==1){
				
				if((stcNombre_host=gethostbyname(Config.remoteGnutellaIP_Secundaria))==NULL){
					Log("Ocurrio un error con la funcion gethostbyname en IP Secundaria",LOGERROR);
					return 1; 
				}
				
				if((*isocket_pingPong=socket(AF_INET, SOCK_STREAM, 0))==-1){
					Log("Ocurrio un error al abrir el socket en IP Secundaria",LOGERROR);
				}
				
				sock_pingPong.sin_family= AF_INET; 
				sock_pingPong.sin_port = htons(num=atoi(Config.remoteGnutellaPort_Secundaria));
				/*sock_pingPong.sin_addr.s_addr= inet_addr(Config.remoteGnutellaIP_Secundaria);*/
				
				sock_pingPong.sin_addr = *((struct in_addr *)stcNombre_host->h_addr_list[0]);
				bzero((char *)&(sock_pingPong.sin_zero), 8);
				if(connect(*isocket_pingPong, (struct sockaddr *)&sock_pingPong, sizeof(sock_pingPong))==-1){
					comportamiento++;
					goto Error;
				}
			}
		}
	}
	else{
		
		
			
			nuevo=func_buscarListaGnu(listaGnu, corredor);
			
			if(nuevo!=NULL){
				char ipGnutella[16]={};
				int i=0;
				sprintf(ipGnutella, "%d", nuevo->data->ip[0]);
				strcat(ipGnutella, ".");
				i=strlen(ipGnutella);
				
				sprintf(&ipGnutella[i], "%d", nuevo->data->ip[1]);
				strcat(ipGnutella, ".");
				i=strlen(ipGnutella);
				
				sprintf(&ipGnutella[i], "%d", nuevo->data->ip[2]);
				strcat(ipGnutella, ".");
				i=strlen(ipGnutella);
				
				sprintf(&ipGnutella[i], "%d", nuevo->data->ip[3]);
				
				if((stcNombre_host=gethostbyname(ipGnutella))==NULL){
					Log("Ocurrio un error con la funcion gethostbyname",LOGERROR);
					return 1; 
				}
				
				if((*isocket_pingPong=socket(AF_INET, SOCK_STREAM, 0))==-1){
					Log("Ocurrio un error al abrir el socket",LOGERROR);
				}
				
				sock_pingPong.sin_family= AF_INET; 
				sock_pingPong.sin_port = htons(nuevo->data->puerto);
				/*sock_pingPong.sin_addr.s_addr= inet_addr(ipGnutella);*/
				sock_pingPong.sin_addr = *((struct in_addr *)stcNombre_host->h_addr_list[0]);
				bzero((char *)&(sock_pingPong.sin_zero), 8);
				
				if(connect(*isocket_pingPong, (struct sockaddr *)&sock_pingPong, sizeof(sock_pingPong))==-1){
					comportamiento=2;
				}
				else{
					comportamiento=0;
				}
			}
			else{
				return 1;
			}
		
	}
	if(comportamiento>1){
		return 1;
	}
	else{
		descriptor.ucTipo=HANDSHAKE;
		if(send(*isocket_pingPong,(TipoHeaderTP*)&descriptor,sizeof(TipoHeaderTP),0)<=0){
			goto Error;
		}
		
		if(send(*isocket_pingPong,GNUTELLA_SRV_CONNECT,strlen(GNUTELLA_SRV_CONNECT),0)==-1){
			goto Error;
		}
		
		if((len=recv(*isocket_pingPong,(TipoHeaderTP*)&descriptor,sizeof(TipoHeaderTP),0))==-1){
			goto Error;
		}
		
		if((len=recv(*isocket_pingPong,buffer,1024,0))==-1){
			goto Error;
		}
		if (strncmp(GNUTELLA_SRV_OK,buffer, len) != 0)
			return 1;
		else{
			
			
			
			descriptor.ucTipo=PING;
			descriptor.TTL= (atoi(Config.TTL)-1);
			descriptor.Hops=0;
			descriptor.nLongMensaje=13;
			
			if(send(*isocket_pingPong,(TipoHeaderTP*)&descriptor,sizeof(TipoHeaderTP),0)<=0){
				goto Error;
			}
			
			pingPong=func_ConfiguracionPingPong();
			if(send(*isocket_pingPong, (TipoPingPong *)&pingPong, sizeof(TipoPingPong), 0)<=0){
				goto Error;	
			}
			
			/*func_agregarListaPing(descriptor, pingPong, listaPing, *isocket_pingPong);*/
			
			}
		}
		
	
	return 0;
}

int dameMaximo (int *tabla, int n){
	int i;
	int max;

	if ((tabla == NULL) || (n<1))
		return 0;
		
	max = tabla[0];
	for (i=0; i<n; i++)
		if (tabla[i] > max)
			max = tabla[i];

	return max;
}


void compactaClaves (int *tabla, int *n){
	int i,j;

	if ((tabla == NULL) || ((*n) == 0)){
		return;
	}
	j=0;
	for (i=0; i<(*n); i++){
		if (tabla[i] != -1){
			tabla[j] = tabla[i];
			j++;
		}
	}
	
	*n = j;
}



TipoPingLista func_cargarTipoPingLista(TipoPingPong pingPong,char *IdMensaje,unsigned char hops,unsigned char ttl,int socketOrigen){
   TipoPingLista pingAux;
   
   pingAux.Hops = hops;
   pingAux.TTL = ttl;
   memcpy(pingAux.IDMensaje,IdMensaje, 16);
   pingAux.socketOrigen = socketOrigen;
   
   pingAux.puerto = pingPong.puerto;
   strcpy(((char *)pingAux.ipLocal),(char *)pingPong.ipLocal);
   pingAux.numeroDeArchivosCompartidos = pingPong.numeroDeArchivosCompartidos;
   pingAux.numeroDeKBCompartidos = pingPong.numeroDeKBCompartidos;
   
   return pingAux;

}

ListaQuery *func_agregarQuery(TipoHeaderTP header, TipoQuery query,ListaQuery **lista, int isocket){
	ListaQuery *auxiliar;
	auxiliar=(ListaQuery*)malloc(sizeof(ListaQuery));
	auxiliar->data=(struct _ListaDatosQuery *)malloc(sizeof(ListaDatosQuery));
	
	auxiliar->siguiente=(int*)*lista;
	
	memcpy(auxiliar->data->IDMensaje, header.IDMensaje, 16);
	
	auxiliar->data->socketOrigen=isocket;
	auxiliar->data->TTL=header.TTL;
	auxiliar->data->Hops=header.Hops;
	auxiliar->data->borrado=0;
	
	auxiliar->data->iSpeed=query.iSpeed;
	strncpy(auxiliar->data->sArchBuscado, query.sArchBuscado,LARGO_ARCHIVO_BUSCADO);
	*lista=auxiliar;
	
	return *lista;
}



ListaPing *func_agregarListaPing(TipoHeaderTP header, TipoPingPong ping,ListaPing **lista, int isocket){
	ListaPing *auxiliar;
	auxiliar=(ListaPing*)malloc(sizeof(ListaPing));
	auxiliar->data=(struct _ListaDatosPing *)malloc(sizeof(ListaDatosPing));
	
	auxiliar->siguiente=(int*)*lista;
	
	memcpy(auxiliar->data->IDMensaje, header.IDMensaje, 16);
	auxiliar->data->socketOrigen=isocket;
	auxiliar->data->TTL=header.TTL;
	auxiliar->data->Hops=header.Hops;
	auxiliar->data->puerto= ping.puerto;
	memcpy(auxiliar->data->ip,ping.ipLocal,4);
	auxiliar->data->numberOfFileShared=ping.numeroDeArchivosCompartidos;
	auxiliar->data->numberOfKBShared=ping.numeroDeKBCompartidos;
	
	*lista=auxiliar;
	
	return *lista;
}

void func_agregarListaPong(TipoHeaderTP header, TipoPingPong pong,ListaPong **lista, int isocket){
	ListaPong *auxiliar;
	auxiliar=(ListaPong*)malloc(sizeof(ListaPong));
	auxiliar->data=(struct _ListaDatosPong *)malloc(sizeof(ListaDatosPong));
	
	auxiliar->siguiente=(int *)*lista;
	
	memcpy(auxiliar->data->IDMensaje, header.IDMensaje, 16);
	auxiliar->data->socketOrigen=isocket;
	auxiliar->data->TTL=header.TTL;
	auxiliar->data->Hops=header.Hops;
	
	*lista=auxiliar;
	
	
}

ListaVecino *func_agregarListaVecino(TipoHeaderTP header, TipoPingPong pingPong,ListaVecino **lista, int isocket){
	ListaVecino *auxiliar;
	ListaVecino *p;
	int i=0;
	
	auxiliar=(ListaVecino*)malloc(sizeof(ListaVecino));
	auxiliar->data=(struct _ListaDatosVecino *)malloc(sizeof(ListaDatosVecino));
	
	auxiliar->siguiente=(int *)*lista;
	
	memcpy(auxiliar->data->IDMensaje, header.IDMensaje, 16);
	auxiliar->data->socketOrigen=isocket;
	auxiliar->data->puerto=pingPong.puerto;
	memcpy(auxiliar->data->ip, pingPong.ipLocal,4);
	
	p=*lista;
	
	while(p!=NULL){
		
		for(i=0; ((p->data->ip[i])==(auxiliar->data->ip[i])) && i<4 ;i++);
		
		if((i==4) && ((p->data->puerto)==(auxiliar->data->puerto)))
			return *lista;
		p=(ListaVecino *)p->siguiente;
	}
	*lista=auxiliar;
	
	return *lista;
}

ListaGnu *func_agregarListaGnu(TipoPingPong pingPong, ListaGnu **lista){
	ListaGnu *auxiliar;
	auxiliar=(ListaGnu*)malloc(sizeof(ListaGnu));
	auxiliar->data=(struct _ListaDatosGnu *)malloc(sizeof(ListaDatosGnu));
	
	auxiliar->siguiente=(int*)(*lista);
	
	auxiliar->data->puerto=pingPong.puerto;
	memcpy(auxiliar->data->ip, pingPong.ipLocal, 4);
	/*listaAux=*lista;
	while(listaAux!=NULL)
		listaAux=listaAux->siguiente;
	if(((*lista))==NULL)
	*/	*lista=auxiliar;
	/*else
		listaAux=auxiliar;
	*/
	return *lista;
}


int func_buscarQuery(TipoHeaderTP buf, ListaQuery **lista){
	ListaQuery *aux;
	
	aux=*lista;
	
	while(aux!=NULL){
		if( memcmp(aux->data->IDMensaje, buf.IDMensaje, 16)==0)
			return aux->data->socketOrigen;
		aux=(ListaQuery *)aux->siguiente;
	}
	if(aux==NULL)
		return -1;
	
	return aux->data->socketOrigen;
}


int func_buscarListaPing(TipoHeaderTP buf, ListaPing **lista, int *socket , int *lenHops){
	ListaPing *aux;
	
	aux=*lista;
	if((*lenHops)==0){
		if((*socket)!=0){
			while(aux!=NULL){
				if(memcmp(aux->data->IDMensaje, buf.IDMensaje, 16)==0 && aux->data->socketOrigen==(*socket))
					return aux->data->socketOrigen;
				aux=(ListaPing *)aux->siguiente;
			}
		}
		else{
			while(aux!=NULL){
				if(memcmp(aux->data->IDMensaje, buf.IDMensaje, 16)==0)
					return aux->data->socketOrigen;
				aux=(ListaPing *)aux->siguiente;
			}
		}
	}
	else{
		if((*socket)!=0){
			while(aux!=NULL){
				if(memcmp(aux->data->IDMensaje, buf.IDMensaje, 16)==0 && aux->data->socketOrigen==(*socket)){
					*lenHops=aux->data->Hops;
					return aux->data->socketOrigen;
				}
				aux=(ListaPing *)aux->siguiente;
			}
		}
		else{
			while(aux!=NULL){
				if(memcmp(aux->data->IDMensaje, buf.IDMensaje, 16)==0){
					*lenHops=aux->data->Hops;
					return aux->data->socketOrigen;
				}
				aux=(ListaPing *)aux->siguiente;
			}
		}
	
	}
	if(aux==NULL)
		return -1;
	
	return aux->data->socketOrigen;
}


int func_buscarListaPong(TipoHeaderTP buf, ListaPong **lista){
	ListaPong *aux;
	
	aux=*lista;
	
	while(aux!=NULL){
		if(memcmp(aux->data->IDMensaje, buf.IDMensaje, 16)==0)
			return aux->data->socketOrigen;
		aux=(ListaPong *)aux->siguiente;
	}
	if(aux==NULL)
		return -1;
	
	return aux->data->socketOrigen;
}


int func_buscarListaVecino(TipoHeaderTP buf, ListaVecino **lista, int *apuntador, int socket){
	ListaVecino *aux;
	int i=0;
	aux=*lista;
	
	while(aux!=NULL){
		if((memcmp(aux->data->IDMensaje, buf.IDMensaje, 16)!=0)&& i>=(*apuntador) && aux->data->socketOrigen!=socket){
			(*apuntador)=(++i);
			return aux->data->socketOrigen;
		}
		aux=(ListaVecino *)aux->siguiente;
		i++;
	}
	(*apuntador)=i;
	if(aux==NULL)
		return -1;
	
	return aux->data->socketOrigen;
}


ListaGnu *func_buscarListaGnu(ListaGnu **lista, int *apuntador){
	ListaGnu *aux;
	/*NuevoServer *buscado;*/
	int i=0;
	
	aux=*lista;
	
	while(aux!=NULL){
		if(i<*apuntador){
			aux=(ListaGnu *)aux->siguiente;
			i++;
		}
		else{
			/*buscado->puerto=aux->data->puerto;
			strncpy((char*)buscado->ip, (char *)aux->data->ip, 4);*/
			(*apuntador)++;
			return aux;
		}
	}
	if(aux==NULL){
		(*apuntador)=0;
		return NULL;
	}
	return NULL;
}

int func_eliminarPing(ListaPing **lista, int socket){
	ListaPing *aux, *aux2;
	int i=0;
	
	aux=*lista;
	
	while(aux!=NULL){
		if(i==0){
			if(aux->data->socketOrigen == socket){
				*lista=(ListaPing *)aux->siguiente;
				
				return 0;
			}
			i++;
		}
		else{
			if(aux->data->socketOrigen==socket){
				aux2->siguiente=aux->siguiente;
				
				return 0;
			}
		}
		aux2=aux;
		aux=(ListaPing *)aux->siguiente;
	}
	if(aux==NULL)
		return -1;
	
	return 0;
}

int func_eliminarPong(ListaPong **lista, int socket){
	ListaPong *aux, *aux2;
	int i=0;
	
	aux=*lista;
	
	while(aux!=NULL){
		if(i==0){
			if(aux->data->socketOrigen == socket){
				*lista=(ListaPong *)aux->siguiente;
				return 0;
			}
		}
		else{
			if(aux->data->socketOrigen==socket){
				aux2->siguiente=aux->siguiente;
				return 0;
			}
		}
		aux2=aux;
		aux=(ListaPong *)aux->siguiente;
	}
	if(aux==NULL)
		return -1;
	
	return 0;
}

int func_eliminarQuery(ListaQuery **lista){
	ListaQuery *aux, *aux2;
	int i=0;
	
	aux=*lista;
	while(aux!=NULL){
		aux->data->borrado++;
		aux=(ListaQuery *)aux->siguiente;
	}
	
	aux=*lista;
	while(aux!=NULL){
		if(i==0){
			if(aux->data->borrado == 2){
				*lista=(ListaQuery *)aux->siguiente;
				return 0;
			}
			i++;
		}
		else{
			if(aux->data->borrado==2){
				aux2->siguiente=aux->siguiente;
				return 0;
			}
		}
		aux2=aux;
		aux=(ListaQuery *)aux->siguiente;
	}
	if(aux==NULL)
		return -1;
	
	return 0;
}

int func_eliminarVecino(ListaVecino **lista, int socket, int *conexionGnu){
	ListaVecino *aux, *aux2;
	int i=0;
	
	aux=*lista;
	
	while(aux!=NULL){
		if(i==0){
			if(aux->data->socketOrigen == socket){
				*lista=(ListaVecino *)aux->siguiente;
				(*conexionGnu)--;
				return 0;
			}
			i++;
		}
		else{
			if(aux->data->socketOrigen==socket){
				aux2->siguiente=aux->siguiente;
				(*conexionGnu)--;
				return 0;
			}
		}
		aux2=aux;
		aux=(ListaVecino *)aux->siguiente;
	}
	if(aux==NULL)
		return -1;
	(*conexionGnu)--;
	return 0;
}
