/***************************************************************************
 *   Copyright (C) 2008 by Intrepid Programmers   *
 ***************************************************************************/
#include "Genericas.h"
#define MAXCLIENTES 20
#define LRU 1
#define LFU 2

typedef struct {
	char nombreModulo[LARGO_NOMBRE_MODULO];
	char ipLocal[LARGO_IP];
	char puertoLocal[LARGO_PUERTO];
	char ipSrvNet[LARGO_IP];
	char puertoSrvNet[LARGO_PUERTO];
	int  iAlgoritmo;
	int  iCantMaximo;
	int  iTiempoVida;
	char archivoHeader[LARGO_ARCHIVO_BUSCADO];
	char archivoBody[LARGO_ARCHIVO_BUSCADO];
} TipoConfiguracion;

typedef struct {
        int  iTamResultados;
        time_t tTiempoUltimoUso;
	int iCantUtilizado;
	int iTamQuery;
} TipoArchHeader;

typedef struct {
	char *strResultados;
} TipoArchBody;

void AsignarConfiguracion(char* etiqueta, char* valor);
void func_preparoMensajeIPC(TipoMensajeIPC *MensajeIPC,unsigned char iTipo, int iTamanioMensaje);
int func_Identificacion(int isock_srvNet, struct sockaddr_in *sckSrvNet);
int func_recibirSelect(int isock_local,fd_set *descriptoresLectura,int *isock_client, 
                       int *visockCliente,int *numeroClientes, TipoMensajeIPC *stcMensaje);
void func_PreparaAtSeniales();
void func_AtiendeSenial (const int senial);


void nuevoCliente (int servidor, int *clientes, int *nClientes);
int Acepta_Conexion_Cliente (int Descriptor);
int dameMaximo (int *tabla, int n);
void compactaClaves (int *tabla, int *n);


TipoNodoRta* CrearElementoRta(int tamanio);
int AgregarElementoRta(TipoListaRta *lista, TipoNodoRta* elemento);
int AgregarElementoPilaRta(TipoListaRta *lista, TipoNodoRta* elemento);
TipoNodoRta* ExtraerPrimerElementoRta(TipoListaRta *lista);
TipoNodoRta* ExtraerElementoRta(TipoListaRta *lista, void* dato, int (*punteroFuncion) (void*, void*)) ;
TipoNodoRta* BuscarElementoRta(TipoListaRta *lista, void* dato, int (*punteroFuncion) (void*, void*));
void EliminarListaRta(TipoListaRta *lista) ;
int ContarElementosListaRta(TipoListaRta *lista);

int func_ListaLlena(TipoListaRta *lista);
int func_esMismoQuery(void *dato,TipoNodoRta *elemento);
int func_MenosRecientemente(void *dato,TipoNodoRta *elemento);
int func_MenosFrecuentemente(void *dato,TipoNodoRta *elemento);
void func_guardarResultados(TipoListaRta * listaResultados);
void func_levantarResultados(TipoListaRta * listaResultados);
int func_responderSrvNet(TipoMensajeIPC stcMensaje, TipoNodoRta *stcNodoBuscado, int isock_srvNet);
TipoNodoRta* BuscarRta(TipoListaRta *lista, int (*punteroFuncion) (void*, void*));
void func_agregarResultado(TipoListaRta * listaResultados,TipoNodoRta * nodoRta,TipoMensajeIPC stcMensaje);
void func_ActualizaTiempoVida(TipoListaRta *lista);

