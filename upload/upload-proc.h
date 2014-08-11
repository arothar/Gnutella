/***************************************************************************
 *   Copyright (C) 2008 by Alejandro Rothar   *
 ***************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <stdarg.h>

#define STDIN 0

#define LOGINFO 1
#define LOGDEBUG 2
#define LOGERROR 3
#define LOGFIN 4

#define LARGO_CADENA 300
#define LARGO_NOMBRE_NODO 50
#define LARGO_NOMBRE_MODULO 50
#define LARGO_USUARIO 50
#define LARGO_IP 16
#define LARGO_PUERTO 10
#define LARGO_PASS 50
#define LARGO_PROGRAMA 50 /* Cantidad maxima de instrucciones */
#define LARGO_SENTENCIA 300 /* Largo maximo de una sentencia */
#define LARGO_COMANDO 50
#define LARGO_CODIGO 1500
#define LARGO_MAXIMO_ARCHIVO 10000
#define LARGO_PILA 200
#define LARGO_MENSAJE 200
#define LARGO_LINEALOG 350
#define LARGO_CONTENIDO 5000
#define LARGO_CONFIGURACION 1000
/*------------------------------------*/
#define LARGO_ARCHIVO_BUSCADO 256

#define GNUTELLA_OK "GNUTELLA 200 OK\n\n"
#define GNUTELLA_ERROR "GNUTELLA 404 ERROR\n\n"
#define GNUTELLA_CONNECT "GNUTELLA CONNECT/0.4\n\n"
#define GNUTELLA_SRV_OK "GNUTELLA_SRV 200 OK\n\n"
#define GNUTELLA_SRV_ERROR "GNUTELLA_SRV 404 ERROR\n\n"
#define GNUTELLA_SRV_CONNECT "GNUTELLA_SRV CONNECT/0.4\n\n"
#define HANDSHAKE 0x04
#define QUERY 0x80
#define QUERYHIT 0x81
#define PING 0x00
#define PONG 0x01


#define MAXIMO_RECURSOS 50

#define COLOR_LIMPIAR_PANTALLA "\e[2J\e[1;1H"
#define COLOR_POR_DEFECTO "\e[37;40;0m"
#define COLOR_VERDE "\e[32;40;1m"
#define COLOR_AMARILLO "\e[33;40;1m"
#define COLOR_AZUL "\e[34;40;1m"
#define COLOR_FUCSIA "\e[35;40;1m"


typedef struct {
	unsigned short int puerto;
	unsigned char ipLocal[4];
	unsigned int numeroDeArchivosCompartidos;
	unsigned int numeroDeKBCompartidos;	
} TipoPingPong;

typedef struct {
	char IDMensaje[16];
	int socketOrigen;
	unsigned char TTL;
	unsigned char Hops;
	unsigned short int puerto;
	unsigned char ipLocal[4];
	unsigned int numeroDeArchivosCompartidos;
	unsigned int numeroDeKBCompartidos;	
} TipoPingLista;

typedef struct _TipoNodo {
	struct _TipoNodo *siguiente;
	TipoPingLista *dato;
} TipoNodo;

/*
typedef struct  {
	int *siguiente;
	TipoPingPong *dato;
} TipoNodo;
*/

typedef struct {
	TipoNodo *primero;
	TipoNodo *ultimo;
} TipoLista;

typedef struct {
	int iSpeed;
	char sArchBuscado[LARGO_ARCHIVO_BUSCADO];
} TipoQuery;

typedef struct {
	char IDMensaje[16];
	unsigned char ucTipo; /*PING-PONG-QUERY-QUERYHIT*/
	unsigned char TTL;
	unsigned char Hops;
	int nLongMensaje;
} TipoHeaderTP;

typedef struct{
 int numerOfHits;
 short int port;
 unsigned char ipAddress[4];
 int speed;
 char *resultados;
 unsigned char serventIdentifier[16];
}TipoQueryHit;

typedef struct {
	TipoHeaderTP *primero;
	TipoHeaderTP *ultimo;
} TipoListaPing;

typedef struct{
 unsigned char fileIndex[4];
 int fileSize;
 char *fileName;
} TipoResultSet;

typedef struct {
	char nombreModulo[LARGO_NOMBRE_MODULO];
	char ipLocal[LARGO_IP];
	char puertoLocal[LARGO_PUERTO];
	char puertoGnutella[LARGO_PUERTO];
	char rutaCompartida[LARGO_CADENA];
	char tamanioBuffer[5];
	char numeroDeConexiones[4];
	char timeSelect[4];
	char TTL[4];
	char remoteGnutellaIP_Primaria[LARGO_IP];
	char remoteGnutellaPort_Primaria[LARGO_PUERTO];
	char remoteGnutellaIP_Secundaria[LARGO_IP];
	char remoteGnutellaPort_Secundaria[LARGO_PUERTO];
} TipoConfiguracion;

typedef struct{
	int *siguiente;
	struct _ListaDatosQuery *data;
}ListaQuery;

typedef struct{
	int *siguiente;
	struct _ListaDatosPing *data;
}ListaPing;

typedef struct{
	int *siguiente;
	struct _ListaDatosPong *data;
}ListaPong;

typedef struct{
	int *siguiente;
	struct _ListaDatosVecino *data;
}ListaVecino;

typedef struct{
	int *siguiente;
	struct _ListaDatosGnu *data;
}ListaGnu;

typedef struct _ListaDatosQuery{
	char IDMensaje[16];
	int socketOrigen;
	unsigned char TTL;
	unsigned char Hops;
	int iSpeed;
	char sArchBuscado[LARGO_ARCHIVO_BUSCADO];
	int borrado;
}ListaDatosQuery;

typedef struct _ListaDatosPing{
	char IDMensaje[16];
	int socketOrigen;
	unsigned char TTL;
	unsigned char Hops;
	unsigned short int puerto;
	unsigned char ip[4];
	unsigned int numberOfFileShared;
	unsigned int numberOfKBShared;
}ListaDatosPing;

typedef struct _ListaDatosPong{
	char IDMensaje[16];
	int socketOrigen;
	unsigned char TTL;
	unsigned char Hops;
}ListaDatosPong;

typedef struct _ListaDatosVecino{
	char IDMensaje[16];
	int socketOrigen;
	unsigned short int puerto;
	unsigned char ip[4];
}ListaDatosVecino;

typedef struct _ListaDatosGnu{
	unsigned short int puerto;
	unsigned char ip[4];
	unsigned char hops;
}ListaDatosGnu;

typedef struct _NuevoServer{
	char ip[4];
	unsigned short int puerto;
}NuevoServer;


extern char IDServergnu[16];
char *itoh (int i);
int atoh( char* ptr);
void CargarConfiguracion(char *archivo);
void ParsearConfiguracion(char *contenido);
int Log(char* descripcion, int tipoEvento); /* Esta funcion va a tener un id estatico que incrementa con cada logueo, de forma que solo se reinicia en una nueva ejecucion */
int LogInt(char* descripcion, long int valor, int tipoEvento); /* Esta funcion llama a Log */
int LogFloat(char* descripcion, float valor, int tipoEvento); /* Esta funcion llama a Log */
int LogString(char* descripcion, char* valor, int tipoEvento); /* Esta funcion llama a Log */
int LogChar(char* descripcion, char valor, int tipoEvento); /* Esta funcion llama a Log */
int FileLog(int tipo, char *mensaje, char *nombreModulo);
char *LeerArchivoCompleto(char *nombreArchivo);
void TrimLeft(char *cadena);
void Copiar(char *destino, char *origen, int cantidad);
void TrimRight(char *cadena);
void Trim(char *cadena);
void LCase(char *cadena);
int Split(char *cadena, char separador, char **resultado);
void AsignarConfiguracion(char *, char *);
int func_aleatorio ();
/**/
char *strsep(char **pcadena, const char *delim);

/* Prototipos de funciones de PPCB */

/* Funciones de manejo de listas */
int func_esMismoId(void *dato,TipoNodo *elemento);
int func_esMismoIpDestino(void *dato,TipoNodo *elemento);
TipoNodo* BuscarElemento(TipoLista *lista, void* dato, int (*punteroFuncion) (void*, void*));
TipoNodo* CrearElemento(int tamanio);
int AgregarElemento(TipoLista *lista, TipoNodo* elemento);
int ContarElementosLista(TipoLista *lista);
void bzero (char *, int );


/*Trae la configuracio del modulo*/
void AsignarConfiguracion(char *, char *);
/*Envia un archivo por el descriptor del socket*/
int enviarArchivo(int sock_descriptor,FILE *nombreArchivo,char *mensaje, int tamanioBuffer);
/*Extrae el nombre del archivo desde el buffer que le estoy mandando*/
	char *archivoBuscado(char *buf);
/*Arma el mensaje que luego se manda por socket de archivo no encontrado*/
void mensajeArchivoNoEncontrado(char *nombreArchivo,char **mensaje);
/*Arma el mensaje que luego se manda por socket de archivo encontrado (OK)*/
void mensajeArchivoEncontrado(char *nombreArchivo,char **mensaje);
/*Busca en el directorio indicado, y dentro de los subdirectorio, la cadena que le indiquemos.
  Se pasa la cadena a buscar, la ruta en la cual quiere buscar el archivo y un puntero a punteros
  de strings, la devolucion es a un puntero de puntero. El paso de argumentos es como el que sigue:
  punteroDePuntero=busquedaArchivos(nomDir, ruta, &(*punteroDePuntero));*/
char **busquedaArchivos(char *nombreArchivo, char *ruta, int *len, int *cantidadBytes);
/*Para la utilizacion de popen()*/
FILE *popen(char *, char *);
/*Funcion que es para la liberacion de la memoria de busquedaArchivos*/
void liberarPuntero(char **puntero);
/*Funcon que dado un nombre de archivo y una direccion, regresa el peso de tal archivo*/
int tamanioArchivo(char *,char *, int );
/*Saca la basura luego de la terminacion del array, osea saca los \n del final*/
void achicarCadena(char *);
/*Funcion para pasar de int a char*/
void itoa(int value, char *string, int base);
void strreverse(char *, char *);
/*int tamanioTabla (int *tabla, int n);
void ordenarVector (int *tabla, int *n);*/
/*Se trata el envia al cliente*/
void envioRespuesta(int *sock_cliente, char buf[16540]);
void func_envioRespuestaGNUTELLA(int sock_cliente, int *port,char *nombreModulo, ListaQuery **, ListaPing **listaPing, ListaVecino **listaVecino, ListaGnu **listaGnu, int *leng,int *chequeador, int vectorDesconectados[10], int *conexionGnu);
char *buscarNombre(char *vecArchivosBuscados,char **rutaArch);
void func_armadoRespuesta(char *bufferSalida, int *m, char *destino, char *ayuda, char *ayuda2);
void func_armadoIPQueryHit(char *bufferSalida);
void V(int*);
void P(int*);
TipoPingPong func_configuracionPingPong();
int func_comunicacionPingPong(int *isocket_pingPong, ListaPing **listaPing, ListaVecino **listaVecino, ListaGnu **listaGnu, int manipulador, int *corredor);
int dameMaximo (int *tabla, int n);
void compactaClaves (int *tabla, int *n);
/*int func_agregarListaPing(TipoLista *lista, TipoPingLista pingPong);*/
TipoPingLista func_cargarTipoPingLista(TipoPingPong pingPong,char *IdMensaje,unsigned char hops,unsigned char ttl,int sockOrigen);
ListaQuery *func_agregarQuery(TipoHeaderTP , TipoQuery ,ListaQuery **, int);
int func_buscarQuery(TipoHeaderTP , ListaQuery **);
ListaPing *func_agregarListaPing(TipoHeaderTP header, TipoPingPong ping,ListaPing **lista, int isocket);
void func_agregarListaPong(TipoHeaderTP header, TipoPingPong pong,ListaPong **lista, int isocket);
ListaVecino *func_agregarListaVecino(TipoHeaderTP header, TipoPingPong pingPong,ListaVecino **lista, int isocket);
ListaGnu *func_agregarListaGnu(TipoPingPong pingPong,ListaGnu **lista);
int func_buscarListaPing(TipoHeaderTP buf, ListaPing **lista, int *socket, int *lenHops);
int func_buscarListaPong(TipoHeaderTP buf, ListaPong **lista);
int func_buscarListaVecino(TipoHeaderTP buf, ListaVecino **lista, int *apuntador, int socket);
ListaGnu *func_buscarListaGnu(ListaGnu **lista, int *apuntador);
int func_eliminarPing(ListaPing **lista, int socket);
int func_eliminarPong(ListaPong **lista, int socket);
int func_eliminarQuery(ListaQuery **lista);
int func_eliminarVecino(ListaVecino **lista, int socket, int *conexionGnu);
int func_eliminarGnu(ListaGnu **lista, int socket);
