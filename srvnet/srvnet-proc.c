/***************************************************************************
 *   Copyright (C) 2008 by Intrepid Programmers  *
 ***************************************************************************/
#include "srvnet-proc.h"

TipoConfiguracion Config;
TipoGlobales DatosGlobales;
int AlarmaLocal;
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
	
   } else if (!strcmp(etiqueta, "TamanioBuffer")) {
	strcpy(Config.tamanioBuffer, valor);
	
   } else if (!strcmp(etiqueta, "IpSrvFile")) {
	strcpy(Config.ipSrvFile, valor);
	
   } else if (!strcmp(etiqueta, "PuertoSrvFile")) {
	strcpy(Config.puertoSrvFile, valor);
	
   } else if (!strcmp(etiqueta, "TiempoEspera")) {
	Config.iTiempoEspera = atoi(valor);
   
   } else if (!strcmp(etiqueta, "TTL")) {
	Config.iTTL = (atoi(valor) - 1);
   }
}

int func_Identificacion(int isock_srvFile, struct sockaddr_in *sckSrvFile){
   int ibuf_len;
   ibuf_len=strlen(GNUTELLA_SRV_OK)+1;
   TipoHeaderTP *header;
   /*char vcharBuffer[ibuf_len];*/
   char *scharBuffer;

   scharBuffer = (char *) calloc(ibuf_len, sizeof(char));
   if(connect(isock_srvFile,(struct sockaddr *)sckSrvFile,sizeof((*sckSrvFile))) == -1){
	perror("Error de conexion con el servidor de archivos");
	Log("Error de conexion con el servidor de archivos",LOGERROR);
	free(scharBuffer);
	return -1;
   }
   
   header = (char *) malloc(sizeof(TipoHeaderTP));
   func_preparoHeaderHand(header);
   
   if(send(isock_srvFile,header ,sizeof(TipoHeaderTP),0)==-1){
	perror("Ocurrio un error al conectarse con el servidor de archivos");
	Log("No se pudo conectar con el servidor de archivos",LOGERROR);
	free(scharBuffer);
	free(header);
	return -1;
   }
      
   if(send(isock_srvFile,GNUTELLA_SRV_CONNECT,strlen(GNUTELLA_SRV_CONNECT),0)==-1){
	perror("Ocurrio un error al conectarse con el servidor de archivos");
	Log("No se pudo conectar con el servidor de archivos",LOGERROR);
	free(scharBuffer);
	free(header);
	return -1;
   }

   if(recv(isock_srvFile,(TipoHeaderTP *) header,sizeof(TipoHeaderTP),0)==-1){
	perror("Ocurrio un error al conectarse con el servidor de archivos");
	Log("No se pudo conectar con el servidor de archivos",LOGERROR);
	free(scharBuffer);
	free(header);
	return -1;
   }

   if(recv(isock_srvFile,scharBuffer,ibuf_len,0)==-1){
	perror("Ocurrio un error al conectarse con el servidor de archivos");
	Log("No se pudo conectar con el servidor de archivos",LOGERROR);
	free(scharBuffer);
	free(header);
	return -1;
   }
   if (strcmp(GNUTELLA_SRV_OK,scharBuffer) == 0){
   	free(scharBuffer);
   	free(header);
	return 0;
   }else{
   	free(scharBuffer);
   	free(header);
	return -1;
   }
}

void func_preparoHeaderPeticion(TipoHeaderTP *HeaderTP){
   HeaderTP->ucTipo = QUERY;
   HeaderTP->nLongMensaje = sizeof(TipoQuery);
   HeaderTP->TTL = Config.iTTL; /*(unsigned char)atoi(&Config.sTTL);*/
   HeaderTP->Hops = 0;
   /*ArmarId((unsigned char *)HeaderTP->IDMensaje);*/
}
void func_preparoHeaderHand(TipoHeaderTP *HeaderTP){
   HeaderTP->ucTipo = GNU_HAND;
   HeaderTP->nLongMensaje = sizeof(GNU_HAND);
   HeaderTP->TTL = 0; /*(unsigned char)atoi(&Config.sTTL);*/
   HeaderTP->Hops = 0;
   ArmarId((unsigned char *)HeaderTP->IDMensaje);
}
int func_recibirDeCliente(int isock_local,struct sockaddr_in *sckClient,TipoMensaje *stcPeticion){
   int isock_client;
   int iaddress_size;
   int itipoMensaje_size;

   iaddress_size = sizeof(struct sockaddr_in);
   /*Se queda aceptando a los clientes, este va a devolver el descriptor del nuevo cliente.*/
   isock_client = accept(isock_local, (struct sockaddr *)sckClient, (size_t *) &iaddress_size);
   if(isock_client == -1){
	perror("llamada para aceptar la conexion");
	Log("La conexion que se acepto no es correcta",LOGDEBUG);
	/*Comento este exit para que el programa no termine y pueda  */
	/*aceptar mas conexiones si le es posible                    */
	/*exit(1);*/
   }
   /***********************************************************************/
   /*Recibe la peticion de archivos desde el cliente     */
   /***********************************************************************/
   itipoMensaje_size = sizeof(TipoMensaje);
   if(recv(isock_client,(TipoMensaje *) stcPeticion,itipoMensaje_size,0)==-1){
	perror("Ocurrio un error al recibir el encabezado del servidor gnutella");
	Log("No pudo recibir el encabezado del servidor gnutella",LOGERROR);
	return -1;
   }
   return isock_client;
}

void func_preparoHeaderIPC(TipoHeaderIPC *HeaderIPC, int iTamanioMensaje){
   HeaderIPC->ucTipo = QUERY;
   HeaderIPC->nLongMensaje = iTamanioMensaje;
   ArmarId((unsigned char *)HeaderIPC->IDMensaje);
}

/************************************************************************************************/
/************************************************************************************************/
int func_recibirSelect(int isock_local,struct sockaddr_in *sckClient,fd_set *descriptoresLectura, 
			int *visockCliente,int *numeroClientes,char *strHead,int *isock_client, TipoLista *listaCliente){
   int iaddress_size;
   int itipoMensaje_size;
   /*******************************/
   /*PARA IMPLEMENTACION DE SELECT*/
   /*******************************/
   /*int buffer;		 Buffer para leer de los socket */
   int maximo;			/* Número de descriptor más grande */
   int i;
   int iCantRecv;
   unsigned char ucTipoRespuesta;
   int iResultSelect;
   /*******************************/
   iaddress_size = sizeof(struct sockaddr_in);
   /*-------------------------------------------------------------------------------------*/
   /*-------------------------------------------------------------------------------------*/

   /* Se eliminan todos los clientes que hayan cerrado la conexión */
   compactaClaves (visockCliente, numeroClientes);

   /* Se inicializa descriptoresLectura */
   FD_ZERO (descriptoresLectura);

   /* Se añade para select() el socket servidor */
   FD_SET (isock_local, descriptoresLectura);

   /* Se añaden para select() los sockets con los clientes ya conectados */
   for (i=0; i<*numeroClientes; i++)
	FD_SET (visockCliente[i], descriptoresLectura);

   /* Se el valor del descriptor más grande. Si no hay ningún cliente,
	* devolverá 0 */
   maximo = dameMaximo (visockCliente, *numeroClientes);
   if (maximo < isock_local)
	maximo = isock_local;

   /* Espera indefinida hasta que alguno de los descriptores tenga algo
    * que decir: un nuevo cliente o un cliente ya conectado que envía un
    * mensaje */
   func_PreparaAtSeniales();
   alarm(Config.iTiempoEspera);
   iResultSelect = select (maximo + 1, descriptoresLectura, NULL, NULL, NULL);
    if ( iResultSelect == -1 )
    {
	if ( errno == EINTR )
	{
	    if (func_ResponderClientes(listaCliente,0) == -1){
                Log("Error al dar la respuesta al cliente",LOGERROR);
            }
	}
    }else{
        signal (SIGALRM, SIG_IGN);
	/* Se comprueba si algún cliente ya conectado ha enviado algo */
	for (i=0; i<*numeroClientes; i++)
	{
	    if (FD_ISSET (visockCliente[i], descriptoresLectura))
	    {
		    /***********************************************************************/
		    /*Recibe la peticion de archivos desde el cliente     */
		    /***********************************************************************/
		    itipoMensaje_size = sizeof(TipoMensaje);
		    *isock_client = visockCliente[i];
		    /*ES EL SOCKET DEL SERVIDOR DE ARCHIVOS*/
		    if (i == 0){
		          iCantRecv=recv(visockCliente[i],(char *) strHead,24,0);
		          ucTipoRespuesta = strHead[16];
		    }else{
		          iCantRecv=recv(visockCliente[i],(char *) strHead,1024,0);
		          if (iCantRecv == 280 || iCantRecv == 28)
		              ucTipoRespuesta = strHead[16];
		          else
		          if (iCantRecv == 0)
		              ucTipoRespuesta = 0;
		          else
		              ucTipoRespuesta = 255;
		    }
		    if(iCantRecv==-1){
			    /*perror("Ocurrio un error al recibir el encabezado del servidor gnutella");*/
			    Log("No pudo recibir el encabezado del servidor gnutella",LOGINFO);
			    return -1;
		    }
		    if(iCantRecv==0){
			    /* Se indica que el cliente ha cerrado la conexión y se
				    * marca con -1 el descriptor para que compactaClaves() lo
				    * elimine */
                            if (visockCliente[i] == DatosGlobales.isock_Cache){
                                DatosGlobales.isock_Cache = 0;
                            }else{
                                close(visockCliente[i]);
                            }
                            
			    Log("Un cliente cerro la coneccion",LOGINFO);
			    visockCliente[i] = -1;
			    return 0;
		    }
		    if (ucTipoRespuesta == 255){
		      close(visockCliente[i]);
		      Log("Se rechazo una conexion no autorizada... OK",LOGINFO);
		      visockCliente[i] = -1;
		      return 0;
		    }
	    }
        }
	    
	/* Se comprueba si algún cliente nuevo desea conectarse y se le admite */
	if (FD_ISSET (isock_local, descriptoresLectura)){
	    nuevoCliente (isock_local, visockCliente, numeroClientes);
	    return 0;
	}
	
	/*-------------------------------------------------------------------------------------*/
	/*-------------------------------------------------------------------------------------*/
	if(*isock_client == -1){
		perror("llamada para aceptar la conexion");
		Log("La conexion que se acepto no es correcta",LOGDEBUG);
		return -1;
		/*Comento este exit para que el programa no termine y pueda  */
		/*aceptar mas conexiones si le es posible                    */
		/*exit(1);*/
	}
   }
   return ucTipoRespuesta;
}

void func_preparoMensajeIPC(TipoMensajeIPC *MensajeIPC,unsigned char iTipo, int iTamanioMensaje){
   MensajeIPC->ucTipo = iTipo;
   MensajeIPC->nLongMensaje = iTamanioMensaje;
   MensajeIPC->mensaje = (char *) malloc(iTamanioMensaje);
   ArmarId((unsigned char *)MensajeIPC->IDMensaje);
}

void func_PreparaAtSeniales(){

    /*signal (SIGINT,  func_AtiendeSenial);*/
    /*signal (SIGSEGV, func_AtiendeSenial);*/
    signal (SIGALRM, func_AtiendeSenial);

}

void func_AtiendeSenial ( const int senial ){
    switch ( senial )
    {
    
    case SIGALRM:

        signal (SIGALRM, func_AtiendeSenial);
	AlarmaLocal = ALARMA_ACTIVADA;
	break;
    }    
}

int func_ResponderClientes(TipoLista *listaCliente,int iRespondeCache){
    
    TipoNodo *nodoCliente;
    double dTranscurrido;
    int isock_clientWin;
    TipoHeaderIPC HeaderIPC;
    TipoMensajeIPC MensajeIPC;
    int iCantSend;
    int iTamanioRtaCache;
    int iIndiceTest;
    char vTempMensaje[20000];
    
    
    if (AlarmaLocal == ALARMA_ACTIVADA){
	/*Agarro el primero de la lista que es el primero que entro, o sea el mas antiguo.*/
	nodoCliente = listaCliente->primero;
	/*Si hay algun cliente en espera*/
	if (nodoCliente != NULL && nodoCliente->dato->strResultados != NULL){
	    dTranscurrido = difftime(time(NULL),nodoCliente->dato->tTiempoLlegada);
	    /*Verifico si el tiempo transcurrido desde que este cliente hizo una peticion 
	    es mayor al tiempo de espera configurado*/
	    if (DatosGlobales.isock_Cache != 0 && iRespondeCache == YES){
	       dTranscurrido = Config.iTiempoEspera + 1;
	    }
	    if (dTranscurrido >= Config.iTiempoEspera){
		/***************************************************************/
		/*Configuro Header del ipc-irc*/
		/***************************************************************/
		nodoCliente = (TipoNodo *) ExtraerPrimerElemento(listaCliente);
		func_preparoHeaderIPC(&HeaderIPC,nodoCliente->dato->iTamResultados);
		printf("Tamanio resultados: %d",nodoCliente->dato->iTamResultados);

		/***********************************************************************/
		/*Envia el header de respuesta al cliente que la solicito                        */
		/***********************************************************************/
		isock_clientWin = nodoCliente->dato->iSock_ClienteWin;
		iCantSend = send(isock_clientWin,(TipoHeaderIPC *)&HeaderIPC,sizeof(TipoHeaderIPC),0);
		if(iCantSend ==-1){
			perror("Ocurrio un error al enviar el header de la respuesta al cliente");
			Log("No se pudo mandar el header de respuesta al cliente",LOGERROR);
			return -1;
		}
		
		/***********************************************************************/
		/*Envia la respuesta al cliente que la solicito                        */
		/***********************************************************************/
		iCantSend = send(isock_clientWin,(char *)nodoCliente->dato->strResultados ,HeaderIPC.nLongMensaje,0);
		if(iCantSend ==-1){
			perror("Ocurrio un error al enviar la respuesta al cliente");
			Log("No se pudo mandar la respuesta al cliente",LOGERROR);
			return -1;
		}
                for (iIndiceTest=0;iIndiceTest<HeaderIPC.nLongMensaje;iIndiceTest++){
                    vTempMensaje[iIndiceTest] = nodoCliente->dato->strResultados[iIndiceTest];
	       }

		/*Si hay un Cache conectado se envia la respuesta al servidor cache.*/
		if (DatosGlobales.isock_Cache != 0){
		    if(iRespondeCache == NO){
			iTamanioRtaCache = HeaderIPC.nLongMensaje + strlen(nodoCliente->dato->strNombreBuscado);
			func_preparoMensajeIPC(&MensajeIPC,IPC_QUERYHIT,iTamanioRtaCache );
			func_armoResultadosCache(nodoCliente->dato->strNombreBuscado,nodoCliente->dato->strResultados,HeaderIPC.nLongMensaje ,MensajeIPC.mensaje);
			iCantSend = send(DatosGlobales.isock_Cache,(TipoMensajeIPC *)&MensajeIPC,sizeof(MensajeIPC),0);
			iCantSend = send(DatosGlobales.isock_Cache,(char *) MensajeIPC.mensaje,MensajeIPC.nLongMensaje,0);
			if(iCantSend ==-1){
				perror("Ocurrio un error al enviar el QueryHit al servidor Cache");
				Log("No se pudo mandar los resultados al servidor Cache",LOGERROR);
				return -1;
			}
			Log("Respuesta hecha por el servidor Gnutella...",LOGINFO);
		    }else{
		        Log("Respuesta hecha por el servidor Cache...",LOGINFO);
		    }
		    /*free(nodoCliente);*/
		}
	    }
	}
    }
    /*free(nodoCliente);*/
    return 0;

}

/************************************************************************************************/
/************************************************************************************************/

void func_armoResultadosCache(char * nombreBuscado,char * resultados,int iLargoMensaje,char * mensaje){
    strcpy(mensaje,nombreBuscado);
    strcat(mensaje,"|");
    memcpy(mensaje + strlen(nombreBuscado) + 1,resultados,iLargoMensaje);
}

/************************************************************************************************/
/************************************************************************************************/

int func_envioPeticionGnutella(TipoMensaje stcPeticion,char * pcharArchivoBuscado,int isock_srvFile){
    TipoHeaderTP HeaderTP;
    TipoQuery Query;
    /***************************************************************/
    /*Configuro Header de Query para mandar la peticion de busqueda*/
    /***************************************************************/
    func_preparoHeaderPeticion(&HeaderTP);
    strcpy(HeaderTP.IDMensaje,stcPeticion.IDMensaje);
    
    /***************************************************************/
    /*Seteo el Query con el archivo solicitado                     */
    /***************************************************************/
    strcpy(Query.sArchBuscado,pcharArchivoBuscado);
    Query.iSpeed = 0;
    
    /***********************************************************************/
    /*Envia el Header del query al servidor Gnutella conectado directamete */
    /***********************************************************************/
    if(send(isock_srvFile,(TipoHeaderTP *)&HeaderTP,sizeof(TipoHeaderTP),0)==-1){
	    perror("Ocurrio un error al enviar el Header del Query");
	    Log("No se pudo mandar el Header del Query",LOGERROR);
	    return -1;
    }
    
    /***********************************************************************/
    /*Envia la peticion de busqueda de archivo                             */
    /***********************************************************************/
    if(send(isock_srvFile,(TipoQuery *)&Query,HeaderTP.nLongMensaje,0)==-1){
	    perror("Ocurrio un error al enviar la peticionde archivo");
	    Log("No se pudo mandar la peticion de archivo",LOGERROR);
	    return -1;
    }
    return 0;
}

/************************************************************************************************/
/************************************************************************************************/

int func_envioPeticionCache(TipoLista * listaCliente,TipoMensaje stcPeticion,char * pcharArchivoBuscado){
    TipoMensajeIPC stcMensajeIPC;
    int iCantRecv;
    TipoClienteWin datoNodoBuscado;
    TipoClienteWin *nodoCliente;
    char *buffer;
    int iTotalRecv;
    func_preparoMensajeIPC(&stcMensajeIPC,IPC_QUERY,strlen(pcharArchivoBuscado));
    strcpy(stcMensajeIPC.IDMensaje,stcPeticion.IDMensaje);
    strcpy(stcMensajeIPC.mensaje,pcharArchivoBuscado);

    /***********************************************************************/
    /*Envia el header IPC al servidor cache */
    /***********************************************************************/
    if(send(DatosGlobales.isock_Cache,(TipoMensajeIPC *)&stcMensajeIPC,sizeof(TipoMensajeIPC),0)==-1){
	    perror("Ocurrio un error al enviar el Header del Query al Cache");
	    Log("No se pudo mandar el Header del Query al Cache",LOGERROR);
	    return -1;
    }
    if(send(DatosGlobales.isock_Cache,(char *) stcMensajeIPC.mensaje,stcMensajeIPC.nLongMensaje,0)==-1){
	    perror("Ocurrio un error al enviar el mensaje al Cache");
	    Log("No se pudo mandar el mensaje al Cache",LOGERROR);
	    return -1;
    }
    iCantRecv = recv(DatosGlobales.isock_Cache,(TipoMensajeIPC *)&stcMensajeIPC,sizeof(TipoMensajeIPC),0);
    if(iCantRecv ==-1){
	    perror("Ocurrio un error al recibir la cabecera de respuesta del Cache");
	    Log("No se pudo recibir la cabecera de respuesta del Cache",LOGERROR);
	    return -1;
    }
    if (iCantRecv > 0){
        iCantRecv = stcMensajeIPC.nLongMensaje;
        if (stcMensajeIPC.nLongMensaje > 0){
            stcMensajeIPC.mensaje = (char *) malloc(stcMensajeIPC.nLongMensaje);
            buffer = (char *) malloc(stcMensajeIPC.nLongMensaje);
	    iCantRecv = recv(DatosGlobales.isock_Cache,(char *) buffer,stcMensajeIPC.nLongMensaje,0);
	    	    
            memcpy(stcMensajeIPC.mensaje,buffer,iCantRecv);
	    iTotalRecv = iCantRecv;
	    while (iTotalRecv < stcMensajeIPC.nLongMensaje){
		    iCantRecv = recv(DatosGlobales.isock_Cache,(char *) buffer,stcMensajeIPC.nLongMensaje,0);
		    memcpy(stcMensajeIPC.mensaje+iTotalRecv,buffer,iCantRecv);
		    iTotalRecv +=iCantRecv;
	    }
	    

	    if(iCantRecv ==-1){
		    perror("Ocurrio un error al recibir la cabecera de respuesta del Cache");
		    Log("No se pudo recibir la cabecera de respuesta del Cache",LOGERROR);
		    return -1;
	    }
	    
	    strcpy(datoNodoBuscado.IDMensaje,stcMensajeIPC.IDMensaje);
	    nodoCliente = (TipoClienteWin *) BuscarElemento((TipoLista *) listaCliente, (void *) &datoNodoBuscado,(void *) func_esMismoId);
            func_agregarResultado((TipoNodo *) nodoCliente,stcMensajeIPC.mensaje,stcMensajeIPC.nLongMensaje);
            func_ResponderClientes(listaCliente,YES);
        }
    }
    return iCantRecv;

}

void func_agregarResultado(TipoNodo * nodoCliente,char * strBufferRta,int iTamResultados){
    char *strTemp;
    if(nodoCliente->dato->strResultados == NULL){
	nodoCliente->dato->strResultados = (char *) realloc(nodoCliente->dato->strResultados, iTamResultados+1);
	nodoCliente->dato->iTamResultados = iTamResultados;
	/*strcpy(nodoCliente->dato->strResultados,strBufferRta);*/
	memcpy(nodoCliente->dato->strResultados,strBufferRta,iTamResultados);
    }else{
	/*CUANDO SE PRUEBA CON MAS DE UN SERVIDOR HAY QUE PROBAR ESTA PARTE DEL CODIGO A VER SI
	ESTA ASIGNANDO BIEN LA MEMORIO*/
	strTemp = (char *) calloc(nodoCliente->dato->iTamResultados + iTamResultados + 2, sizeof(char));
	memcpy(strTemp,nodoCliente->dato->strResultados,nodoCliente->dato->iTamResultados);
	memcpy(strTemp + nodoCliente->dato->iTamResultados,"||",2);
	nodoCliente->dato->iTamResultados += 2;
	memcpy(strTemp  + nodoCliente->dato->iTamResultados,strBufferRta,iTamResultados);
	nodoCliente->dato->iTamResultados += iTamResultados ; 
	
	free(nodoCliente->dato->strResultados);
	nodoCliente->dato->strResultados = (char *) calloc(nodoCliente->dato->iTamResultados, sizeof(char));
	memcpy(nodoCliente->dato->strResultados,strTemp,nodoCliente->dato->iTamResultados);
	free(strTemp);
	/*
	nodoCliente->dato->strResultados = (char *) realloc(nodoCliente->dato->strResultados, iTamResultados + 2);
	memcpy(nodoCliente->dato->strResultados + nodoCliente->dato->iTamResultados,"||",2);
	nodoCliente->dato->iTamResultados += 2; 
	memcpy(nodoCliente->dato->strResultados + nodoCliente->dato->iTamResultados,strBufferRta,iTamResultados);
	nodoCliente->dato->iTamResultados += iTamResultados ; 
	*/
    }

}

/************************************************************************************************/
/************************************************************************************************/

/*
 * Crea un nuevo socket cliente.
 * Se le pasa el socket servidor y el array de clientes, con el número de
 * clientes ya conectados.
 */
void nuevoCliente (int servidor, int *clientes, int *nClientes)
{
	/* Acepta la conexión con el cliente, guardándola en el array */
	clientes[*nClientes] = Acepta_Conexion_Cliente (servidor);
	(*nClientes)++;

	/* Si se ha superado el maximo de clientes, se cierra la conexión,
	 * se deja todo como estaba y se vuelve. */
	if ((*nClientes) >= MAXCLIENTES)
	{
		close (clientes[(*nClientes) -1]);
		(*nClientes)--;
		return;
	}
		
	/* Envía su número de cliente al cliente */
	/*Escribe_Socket (clientes[(*nClientes)-1], (char *)nClientes, sizeof(int));*/

	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	printf ("Aceptado cliente %d\n", *nClientes);
	return;
}
/*
* Se le pasa un socket de servidor y acepta en el una conexion de cliente.
* devuelve el descriptor del socket del cliente o -1 si hay problemas.
* Esta funcion vale para socket AF_INET o AF_UNIX.
*/
int Acepta_Conexion_Cliente (int Descriptor){
	socklen_t Longitud_Cliente;
	struct sockaddr Cliente;
	int Hijo;

    /*
    * La llamada a la funcion accept requiere que el parametro 
    * Longitud_Cliente contenga inicialmente el tamano de la
    * estructura Cliente que se le pase. A la vuelta de la
    * funcion, esta variable contiene la longitud de la informacion
    * util devuelta en Cliente
    */
    Longitud_Cliente = sizeof (Cliente);
    Hijo = accept (Descriptor, &Cliente, &Longitud_Cliente);
    if (Hijo == -1)
	return -1;

    /*
    * Se devuelve el descriptor en el que esta "enchufado" el cliente.
    */
    return Hijo;
}
/*
 * Función que devuelve el valor máximo en la tabla.
 * Supone que los valores válidos de la tabla son positivos y mayores que 0.
 * Devuelve 0 si n es 0 o la tabla es NULL */
int dameMaximo (int *tabla, int n)
{
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

/*
 * Busca en array todas las posiciones con -1 y las elimina, copiando encima
 * las posiciones siguientes.
 * Ejemplo, si la entrada es (3, -1, 2, -1, 4) con *n=5
 * a la salida tendremos (3, 2, 4) con *n=3
 */
void compactaClaves (int *tabla, int *n){
    int i,j;

    if ((tabla == NULL) || ((*n) == 0))
	return;

    j=0;
    for (i=0; i<(*n); i++){
	if (tabla[i] != -1)
	{
		tabla[j] = tabla[i];
		j++;
	}
    }
    *n = j;
}
