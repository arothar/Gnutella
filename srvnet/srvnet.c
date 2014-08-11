/***************************************************************************
 *   Copyright (C) 2008 by Intrepid Programmers  *
 ***************************************************************************/


#include "srvnet.h"

extern TipoConfiguracion Config;
extern int AlarmaLocal;
extern TipoGlobales DatosGlobales;

char nombreModulo[LARGO_NOMBRE_MODULO];

int main(int argc, char *argv[])
{
    struct sockaddr_in sckLocal;
    struct sockaddr_in sckSrvFile;
    struct sockaddr_in sckClient;
    /*******************************/
    /*PARA IMPLEMENTACION DE SELECT*/
    /*******************************/
    int visockCliente[MAXCLIENTES];/* Descriptores de sockets con clientes */
    fd_set descriptoresLectura;	/* Descriptores de interes para select() */
    int numeroClientes = 0;			/* Número clientes conectados */
    /*******************************/
    struct hostent *stcNombre_host;
    TipoHeaderTP HeaderResp /*, HeaderTP*/; /*encabezado PING-PONG-QUERY-QUERYHIT*/
    TipoMensaje stcPeticion;
    int iPuertoSrvFile;
    int iPuertoLocal;
    int isock_srvFile;
    int isock_local;
    int isock_client;
    int isock_clientWin;
    int iResult;
    char pcharArchivoBuscado[LARGO_ARCHIVO_BUSCADO];
    char *strBufferRta;
    /*IMPLEMENTACION ENTREGA 4*/
    int iTipoMensaje=0;
    char strHead[1024];
    TipoNodo *nodoCliente;
    TipoLista *listaCliente;
    TipoClienteWin datoNodoBuscado;
    TipoMensajeIPC stcMensajeIpc;
    char ipUpload[13];
    /*SOLO PARA TESTING*/
    /*char vTempMensaje[1024];
    int iIndiceTest;*/

    /**************************************************/
    /*CAMBIO EL NOMBRE DEL PROCESO                   */	
    /*************************************************/
    strcpy(nombreModulo,argv[0]);
    sprintf(nombreModulo, "Srvnet[%d]", getpid());
    strncpy(argv[0],nombreModulo,strlen(argv[0]));

    /*************************************************/
    /*CARGA LA CONFIGURACION DESDE EL ARCHIVO DE CONF*/	
    /*************************************************/
    CargarConfiguracion("configuracion");
    Log("Se cargo el archivo de configuracion...",LOGINFO);
    /*strcpy(argv[0],Config.nombreModulo);*/
    
    iPuertoSrvFile = atoi(Config.puertoSrvFile);
    iPuertoLocal = atoi(Config.puertoLocal);
    Copiar(ipUpload,Config.ipSrvFile, 13);
    Log(ipUpload, LOGINFO);
    Trim(Config.ipSrvFile);
    if((stcNombre_host=gethostbyname("192.168.0.102"))==NULL){
            perror("Ocurrio un error al resolver el nombre...");
            Log("Ocurrio un error al resolver el nombre...",LOGERROR);
	    exit(0); 
    }
    
    /**************************************************/
    /* SETEO LA ATENCION DE LAS SENIALES              */
    /**************************************************/
    /*func_PreparaAtSeniales();*/
    signal (SIGALRM, func_AtiendeSenial);
    
    /*************************************************************/
    /* CREA EL SOCKET PARA CONECTARSE CON EL SERVIDOR DE ARCHIVOS*/
    /*************************************************************/
    if ((isock_srvFile=socket(AF_INET, SOCK_STREAM, 0))==-1){
	    perror("Ocurrio un error al crear el socket para Gnutella");
	    Log("Ocurrio un error al crear el socket para Gnutella",LOGERROR);
	    exit(0);
    }
    
    /*************************************************************/
    /* CREA EL SOCKET PARA PARA BINDEARLO CON EL CLIENTE WINDOWS */
    /*************************************************************/
    if ((isock_local=socket(AF_INET, SOCK_STREAM, 0))==-1){
	    perror("Ocurrio un error al crear el socket Local");
	    Log("Ocurrio un error al crear el socket Local",LOGERROR);
	    exit(0);
    }
    /**************************************************/
    /*Configura la estructura del servidor de archivos*/
    /**************************************************/
    sckSrvFile.sin_family= AF_INET; 
    sckSrvFile.sin_port = htons(iPuertoSrvFile); 
    sckSrvFile.sin_addr= *((struct in_addr *)stcNombre_host->h_addr_list[0]); 
    
    /*************************************************/
    /*Configura la estructura local                  */
    /*************************************************/
    Log("Configurando socket local... ",LOGINFO);
    sckLocal.sin_family = AF_INET;
    sckLocal.sin_addr.s_addr = INADDR_ANY;
    sckLocal.sin_port = htons(atoi(Config.puertoLocal));
    Log("OK Configuracion Socket Local",LOGINFO);
    
    /***************************************************/
    /*Asocia el sock de escucha con la estructura local */
    /***************************************************/
    Log("Bindeando el socket local...",LOGINFO);
    if (bind(isock_local, (struct sockaddr *)&sckLocal, sizeof(sckLocal)) == -1){
	    perror("Error en la llamada para unir");
	    Log("Error en la funcion Bind del srvNet (isock_local)",LOGERROR);
	    exit(-1);
    }
    
    Log("Funcion listen... ",LOGINFO);
    if (listen(isock_local,20) == -1){
	    perror("llamada para escuchar");
	    Log("No pudo ejecutarse con exito la funcion LISTEN",LOGERROR);
	    exit(0);
    }
    /***********************************************************************/
    /*Se identifica el servidor de Red como tal con el servidor de archivos*/
    /***********************************************************************/
    if (func_Identificacion(isock_srvFile, &sckSrvFile) == -1){	
	    perror("No se pudo identificar el servidor");
	    Log("No se pudo identificar el servidor de red",LOGERROR);
	    exit(0);
    }
    Log("Se identifico correctamente con el servidor gnutella...",LOGINFO);
    /* Acepta la conexión con el cliente, guardándola en el array */
    visockCliente[numeroClientes] = isock_srvFile;
    numeroClientes++;
    
    /* Si se ha superado el maximo de clientes, se cierra la conexión,
	* se deja todo como estaba y se vuelve. */
    if ((numeroClientes) >= MAXCLIENTES)
    {
	    close (visockCliente[numeroClientes -1]);
	    numeroClientes--;
    }
    
    /***********************************/
    /* Inicializo la lista de clientes */
    /***********************************/
    listaCliente = (TipoLista *) malloc(sizeof(NULL)*2);
    listaCliente->primero = NULL;
    listaCliente->ultimo = NULL;

    alarm(Config.iTiempoEspera);
    Log("Alarma cargada...",LOGINFO);
   while(1){

   	/*strHead = (char *) malloc(1024);*/
   	iTipoMensaje=func_recibirSelect(isock_local,&sckClient,&descriptoresLectura,visockCliente,&numeroClientes,strHead,&isock_client,listaCliente);
        
        /****************************************************/
        /*VERIFICO SI HAY ALARMA PARA ATENDER               */
        /****************************************************/
        if (func_ResponderClientes(listaCliente,NO) == -1){
            Log("Error al dar la respuesta al cliente",LOGERROR);
        }

	if (iTipoMensaje== -1){
		/*perror("Ocurrio un error en la funcion func_recibirSelect");	*/
		Log("No se pudo recibir una respuesta correcta del cliente. Error en func_recibirDeCliente",LOGINFO);
	}
	if (iTipoMensaje == REQUEST){
	   
	    stcPeticion = *((TipoMensaje *) strHead);
	    strcpy(pcharArchivoBuscado,stcPeticion.nombreArchivo);
	    
	    /****************************************************/
	    /*Agrego el cliente que hizo la peticion a la lista*/
	    /***************************************************/
	    nodoCliente = CrearElemento(sizeof(TipoNodo));
	    nodoCliente->dato->strNombreBuscado = (char *) malloc(strlen(stcPeticion.nombreArchivo));
	    strcpy(nodoCliente->dato->IDMensaje,stcPeticion.IDMensaje);
	    nodoCliente->dato->iSock_ClienteWin = isock_client;
	    strcpy(nodoCliente->dato->strNombreBuscado, stcPeticion.nombreArchivo);
	    nodoCliente->dato->strResultados = NULL;
	    nodoCliente->dato->tTiempoLlegada = time(NULL);
	    AgregarElemento(listaCliente, nodoCliente);
	    
	    if(DatosGlobales.isock_Cache != 0){
	       if (func_envioPeticionCache(listaCliente,stcPeticion,pcharArchivoBuscado) <= 0){
	           /*Log("Se envio la peticion al servidor gnutella...",LOGINFO);*/
	           func_envioPeticionGnutella(stcPeticion,pcharArchivoBuscado,isock_srvFile);
	       }
	    }else{
	       /*Log("Se envio la peticion al servidor gnutella...",LOGINFO);*/
	       func_envioPeticionGnutella(stcPeticion,pcharArchivoBuscado,isock_srvFile);
	    }
	    
	}else /*End if (iTipoMensaje == REQUEST)*/
	if (iTipoMensaje == QUERYHIT){
	    /***********************************************************************/
	    /*Recibe la cabecera del la respuesta que envia el Server Gnutella     */
	    /***********************************************************************/
	    HeaderResp = *((TipoHeaderTP *) strHead);
	    memcpy(datoNodoBuscado.IDMensaje,HeaderResp.IDMensaje,16);
	    nodoCliente = BuscarElemento(listaCliente, &datoNodoBuscado, (void *)func_esMismoId);
	    
	    if (nodoCliente != NULL){
		isock_clientWin = nodoCliente->dato->iSock_ClienteWin;
		Log("Recibio Header QueryHit",LOGINFO);
		
		
		/***********************************************************************/
		/*Se verifica que el header sea del formato correcto                   */
		/***********************************************************************/
		if (HeaderResp.ucTipo != QUERYHIT){
			perror("El formato del Header no es el esperado");
			/*Log("El formato del header no es el esperado",LOGERROR);	*/
		}		
		
		Log("El formato de Header Query Hit es CORRECTO se esta recibiendo la respuesta a la peticion",LOGINFO);
		    
	    
		/***********************************************************************/
		/*Se reciben los datos del servidor Gnutella                           */
		/***********************************************************************/
		strBufferRta = (char *)calloc(HeaderResp.nLongMensaje, sizeof(char));
		iResult = recv(isock_srvFile,(char *)strBufferRta,HeaderResp.nLongMensaje,0);
		
		
		if (iResult == -1){
			perror("Error al recibir los datos del servidor Gnutella");
			Log("No se pudo recibir la respuesta del servidor Gnutella exitosamente",LOGERROR);
		}
    
		/*for (iIndiceTest=0;iIndiceTest<65;iIndiceTest++){
		    vTempMensaje[iIndiceTest] = nodoCliente->dato->strResultados[iIndiceTest];
		}*/
    
		func_agregarResultado(nodoCliente,strBufferRta,iResult);
            }else{
                Log("Se recibio un QUERYHIT luego de que se haya enviado la respuesta...",LOGINFO);
            }

	}else/*End if (iTipoMensaje == QUERYHIT)*/
	if (iTipoMensaje == IPC_CACHE_CONNECT){
	
            func_preparoMensajeIPC(&stcMensajeIpc,IPC_OK_CONNECT,0);
	    if(send(isock_client,&stcMensajeIpc,sizeof(stcMensajeIpc),0)==-1){
		    perror("Ocurrio un error al conectarse con el servidor de Cache");
		    Log("No se pudo mandar el mensaje al servidor Cache",LOGERROR);
		    return -1;
	    }else{
	       DatosGlobales.isock_Cache = isock_client;
	       Log("Conexion con servidor cache...OK",LOGINFO);
	    }
	}else/*End if (iTipoMensaje == IPC_OK_CONNECT)*/
        if (iTipoMensaje == IPC_QUERYHIT){
	
            /***********************************************************************/
	    /*Recibe la cabecera del la respuesta que envia el Server Gnutella     */
	    /***********************************************************************/
	    HeaderResp = *((TipoHeaderTP *) strHead);
	    strcpy(datoNodoBuscado.IDMensaje,HeaderResp.IDMensaje);
	    nodoCliente = BuscarElemento(listaCliente, &datoNodoBuscado, (void *)func_esMismoId);
	    isock_clientWin = nodoCliente->dato->iSock_ClienteWin;
	    Log("Recibio Header QueryHit",LOGINFO);
	    
	    
	}
	strHead[0]='\0';
	/*free(strHead);*/
	/*End if (iTipoMensaje == IPC_OK_CONNECT)*/
	/*else
	if (iTipoMensaje == 255 && isock_client != 0){
	   close(isock_client);
	}*/
	
   }   
   
   
  return 0;
}
