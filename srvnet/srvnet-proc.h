/***************************************************************************
 *   Copyright (C) 2008 by Intrepid Programmers   *
 ***************************************************************************/
#include "Genericas.h"
#include <netinet/in.h>

#define MAXCLIENTES 20

#define CLIWIN 1
#define CLIGNUTELLA 2

#define REQUEST 0x02
#define RESPONSE 0x03

#define ALARMA_ACTIVADA 1
#define ALARMA_DESACTIVADA 0

typedef struct {
	char nombreModulo[LARGO_NOMBRE_MODULO];
	char ipLocal[LARGO_IP];
	char puertoLocal[LARGO_PUERTO];
	char tamanioBuffer[5];
	char ipSrvFile[LARGO_IP];
	char puertoSrvFile[LARGO_PUERTO];
	int  iTiempoEspera;
	int  iTTL;
} TipoConfiguracion;

typedef struct {
    int isock_Cache;
    int iport_Cache;
}TipoGlobales;

/*Trae la configuracio del modulo*/
void AsignarConfiguracion(char *, char *);
/*Identifica al srvNet como tal al servidor Gnutella directamente conectado*/
int func_Identificacion(int isock_srvFile, struct sockaddr_in *sckSrvFile);
/*Prepara el header de peticion para mandarlo al servidor Gnutella.*/
void func_preparoHeaderPeticion(TipoHeaderTP *HeaderTP);
int func_recibirDeCliente(int isock_local,struct sockaddr_in *sckClient,TipoMensaje *stcPeticion);
void func_preparoHeaderIPC(TipoHeaderIPC *HeaderIPC, int iTamanioMensaje);
int func_recibirSelect(int isock_local,struct sockaddr_in *sckClient,fd_set *descriptoresLectura, 
			int *visockCliente,int *numeroClientes,char *strHead,int *isock_client,TipoLista *listaCliente);
void func_AtiendeSenial ( const int senial );
void func_PreparaAtSeniales();
int func_ResponderClientes(TipoLista *listaCliente,int iRespondeCache);
void func_preparoMensajeIPC(TipoMensajeIPC *MensajeIPC,unsigned char iTipo, int iTamanioMensaje);
void func_armoResultadosCache(char * nombreBuscado,char * resultados,int iLargoMensaje,char * mensaje);
int func_envioPeticionGnutella(TipoMensaje stcPeticion,char * pcharArchivoBuscado,int isock_srvFile);
int func_envioPeticionCache(TipoLista * listaCliente,TipoMensaje stcPeticion,char * pcharArchivoBuscado);
void func_agregarResultado(TipoNodo * nodoCliente,char * strBufferRta,int iTamResultados);
void func_preparoHeaderHand(TipoHeaderTP *HeaderTP);

