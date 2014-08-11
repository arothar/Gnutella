/*

Aca van todas las funciones y declaraciones genericas que puede llegar a utilizar
cualquier modulo. Se podria renombrar Estructuras.h a Genericas.h.

*/

#ifndef Genericash
#define Genericash

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

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

#define MAXIMO_RECURSOS 50

#define COLOR_LIMPIAR_PANTALLA "\e[2J\e[1;1H"
#define COLOR_POR_DEFECTO "\e[37;40;0m"
#define COLOR_VERDE "\e[32;40;1m"
#define COLOR_AMARILLO "\e[33;40;1m"
#define COLOR_AZUL "\e[34;40;1m"
#define COLOR_FUCSIA "\e[35;40;1m"

/*MENSAJES GNUTELLA*/
#define GNUTELLA_OK "GNUTELLA 200 OK\n\n" 
#define GNUTELLA_ERROR "GNUTELLA 404 ERROR\n\n" 
#define GNUTELLA_CONNECT "GNUTELLA CONNECT/0.4\n\n" 
#define GNUTELLA_SRV_OK "GNUTELLA_SRV 200 OK\n\n"
#define GNUTELLA_SRV_ERROR "GNUTELLA_SRV 404 ERROR\n\n" 
#define GNUTELLA_SRV_CONNECT "GNUTELLA_SRV CONNECT/0.4\n\n"

/***************************MENSAJES PROTOCOLO ANEXO*****************************/
#define PING 0x00
#define PONG 0x01
#define QUERY 0x80
#define QUERYHIT 0x81
#define IPC 0x90
#define IRC 0x91
#define IPC_CACHE_CONNECT 0x92
#define IPC_OK_CONNECT 0x93
#define IPC_QUERY 0x95
#define IPC_QUERYHIT 0x96
#define GNU_HAND 0x04

#define YES 1
#define NO 0

/***************************  ESTRUCTURAS  *****************************/
/*typedef struct _TipoNodo {
	struct _TipoNodo *siguiente;
} TipoNodo;*/
typedef struct {
	char IDMensaje[16];
	int iSock_ClienteWin;
	char *strResultados;
	char *strNombreBuscado;
	int iTamResultados;
	time_t tTiempoLlegada;
} TipoClienteWin;

typedef struct _TipoNodo {
        TipoClienteWin *dato;
	struct _TipoNodo *siguiente;
} TipoNodo;

typedef struct {
	TipoNodo *primero;
	TipoNodo *ultimo;
} TipoLista;

/*Cuerpo del msg QUERY		*/
typedef struct {
	int iSpeed;
	char sArchBuscado[LARGO_ARCHIVO_BUSCADO];
	} TipoQuery;

/*Cabeza de msgs del protocolo*/
typedef struct {
	char IDMensaje[16];
	unsigned char ucTipo; /*PING-PONG-QUERY-QUERYHIT*/
	unsigned char TTL;
	unsigned char Hops;
	int nLongMensaje;
	} TipoHeaderTP;

/*Estructura de mensaje intercambiada con el cliente*/
typedef struct {
	char IDMensaje[16];
	unsigned char IDProtocolo;
	int longitud;
	char nombreArchivo[LARGO_ARCHIVO_BUSCADO];
} TipoMensaje;

/*Cabeza de msgs del protocolo*/
typedef struct {
	char IDMensaje[16];
	unsigned char ucTipo; /*PING-PONG-QUERY-QUERYHIT*/
	int nLongMensaje;
	} TipoHeaderIPC;
	
typedef struct {
	char IDMensaje[16];
	unsigned char ucTipo; /*PING-PONG-QUERY-QUERYHIT*/
	int nLongMensaje;
	char *mensaje;
	} TipoMensajeIPC;

/***************************  FUNCIONES GENERICAS *****************************/
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
char *strsep(char **pcadena, const char *delim);
void ArmarId (unsigned char *Id);
int Aleatorio ();

void compactaClaves (int *tabla, int *n);
void nuevoCliente (int servidor, int *clientes, int *nClientes);
int Acepta_Conexion_Cliente (int Descriptor);
int dameMaximo (int *tabla, int n);

/* Prototipos de funciones de PPCB */

/* Funciones de manejo de listas */
int func_esMismoId(void *dato,TipoNodo *elemento);
int func_esMismoIpDestino(void *dato,TipoNodo *elemento);
TipoNodo* BuscarElemento(TipoLista *lista, void* dato, int (*punteroFuncion) (void*, void*));
TipoNodo* CrearElemento(int tamanio);
int AgregarElemento(TipoLista *lista, TipoNodo* elemento);
int ContarElementosLista(TipoLista *lista);
TipoNodo* ExtraerPrimerElemento(TipoLista *lista);

#endif
