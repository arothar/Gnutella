/***************************************************************************
 *   Copyright (C) 2008 by Intrepid Programmers  *
 ***************************************************************************/
#include "cache-proc.h"

TipoConfiguracion Config;
TipoListaRta *listaResultados;

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
	
   } else if (!strcmp(etiqueta, "IpSrvNet")) {
	strcpy(Config.ipSrvNet, valor);
   } else if (!strcmp(etiqueta, "ArchivoHeader")) {
	strcpy(Config.archivoHeader, valor);
   } else if (!strcmp(etiqueta, "ArchivoBody")) {
	strcpy(Config.archivoBody, valor);
   } else if (!strcmp(etiqueta, "PuertoSrvNet")) {
	strcpy(Config.puertoSrvNet, valor);
   } else if (!strcmp(etiqueta, "Algoritmo")) {
        if (!strcmp(valor,"LRU"))
	   Config.iAlgoritmo = LRU;
	else if (!strcmp(valor,"LFU"))
	   Config.iAlgoritmo = LFU;
   } else if (!strcmp(etiqueta, "CantMaximaCache")){
        Config.iCantMaximo = atoi(valor);
   } else if (!strcmp(etiqueta, "TiempoVida")){
        Config.iTiempoVida = atoi(valor);
   }
}

void func_PreparaAtSeniales(){

    signal (SIGINT,  func_AtiendeSenial);
    signal (SIGSEGV, func_AtiendeSenial);
    signal (SIGUSR1, func_AtiendeSenial);
    signal (SIGUSR2, func_AtiendeSenial);

}

void func_AtiendeSenial ( const int senial ){
    switch ( senial )
    {
    
        case SIGUSR1:
            signal (SIGUSR1, func_AtiendeSenial);
            Log("Se recibio la senial SIGUSR1",LOGINFO);
            Log("Se seteo el algoritmo LRU",LOGINFO);
            Config.iAlgoritmo = LRU;
            break;
        
        case SIGUSR2:
            signal (SIGUSR2, func_AtiendeSenial);
            Log("Se recibio la senial SIGUSR2",LOGINFO);
            Log("Se seteo el algoritmo LFU",LOGINFO);
            Config.iAlgoritmo = LFU;
            break;
        case SIGINT:
            Log("Se recibio la senial SIGINT...",LOGINFO);
            Log("Se guardara la lista de respuesta en el disco...",LOGINFO);
            func_guardarResultados((TipoListaRta *) listaResultados);
            exit(0);
            break;
    } 
}


void func_preparoMensajeIPC(TipoMensajeIPC *MensajeIPC,unsigned char iTipo, int iTamanioMensaje){
   MensajeIPC->ucTipo = iTipo;
   MensajeIPC->nLongMensaje = iTamanioMensaje;
   ArmarId((unsigned char *)MensajeIPC->IDMensaje);
}

/* Se conecta con el servidor de red, para que este lo empiece a utilizar desde este momento.*/
int func_Identificacion(int isock_srvNet, struct sockaddr_in *sckSrvNet){
   int ibuf_len;
   ibuf_len=sizeof(TipoMensajeIPC)+1;
   TipoMensajeIPC stcMensajeIpc;

   if(connect(isock_srvNet,(struct sockaddr *)sckSrvNet,sizeof((*sckSrvNet))) == -1){
	perror("Error de conexion con el servidor de Red");
	Log("Error de conexion con el servidor de Red",LOGERROR);
	return -1;
   }
   
   func_preparoMensajeIPC(&stcMensajeIpc,IPC_CACHE_CONNECT,0);
   
   if(send(isock_srvNet,&stcMensajeIpc,sizeof(stcMensajeIpc),0)==-1){
	perror("Ocurrio un error al conectarse con el servidor de Red");
	Log("No se pudo conectar con el servidor de Red",LOGERROR);
	return -1;
   }

   if(recv(isock_srvNet,&stcMensajeIpc,ibuf_len,0)==-1){
	perror("Ocurrio un error al conectarse con el servidor de archivos");
	Log("No se pudo conectar con el servidor de archivos",LOGERROR);
	return -1;
   }
   
    if(stcMensajeIpc.ucTipo == IPC_OK_CONNECT){
        Log("Se conecto correctamente al servidor de Red.", LOGINFO);
        return 0;
    }
    else
        Log("No pudo conectarse al servidor de Red.", LOGERROR);
        return -1;
}


/************************************************************************************************/
/************************************************************************************************/
int func_recibirSelect(int isock_local,fd_set *descriptoresLectura,int *isock_client, 
			int *visockCliente,int *numeroClientes, TipoMensajeIPC *stcMensaje){
    /*int isock_client;*/
    int iaddress_size;
    int itipoMensaje_size;
    /*******************************/
    /*PARA IMPLEMENTACION DE SELECT*/
    /*******************************/
    /*int buffer; Buffer para leer de los socket */
    int maximo;	/* Número de descriptor más grande */
    int i;
    int iCantRecv;
    unsigned char ucTipoRespuesta;
    int iResultSelect;
    int iTotalRecv;
    char *buffer;
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
    iResultSelect = select (maximo + 1, descriptoresLectura, NULL, NULL, NULL);
    if (iResultSelect == -1)
        return -1;
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
		
		iCantRecv=recv(visockCliente[i],(TipoMensajeIPC *) stcMensaje,sizeof(TipoMensajeIPC),0);
		stcMensaje->mensaje = (char *) calloc(stcMensaje->nLongMensaje, sizeof(char));
		buffer = (char *) calloc(stcMensaje->nLongMensaje, sizeof(char));
				
		iCantRecv = recv(visockCliente[i],(char *)buffer,stcMensaje->nLongMensaje,0);
		

		memcpy(stcMensaje->mensaje,buffer,iCantRecv);
		iTotalRecv = iCantRecv;
		while (iTotalRecv < stcMensaje->nLongMensaje){
		        iCantRecv = recv(visockCliente[i],(char *)buffer,stcMensaje->nLongMensaje,0);
			memcpy(stcMensaje->mensaje+iTotalRecv,buffer,iCantRecv);
			iTotalRecv +=iCantRecv;
		}   
		
		
		
		ucTipoRespuesta = stcMensaje->ucTipo;
		
		if(iCantRecv==-1){
			perror("Ocurrio un error al recibir el encabezado del servidor de red");
			Log("No pudo recibir el encabezado del servidor de red",LOGERROR);
			return -1;
		}
		if(iCantRecv==0){
			/* Se indica que el cliente ha cerrado la conexión y se
				* marca con -1 el descriptor para que compactaClaves() lo
				* elimine */
			Log("Un cliente cerro la coneccion",LOGINFO);
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
   return ucTipoRespuesta;
}

/************************************************************************************************/
/************************************************************************************************/

void func_levantarResultados(TipoListaRta * listaResultados){
    FILE *fdHeader;
    FILE *fdBody;
    TipoArchHeader *header;
    TipoArchBody *body;
    TipoNodoRta *nuevoNodoRta;
    int leidosHeader;
    int leidosBody;
    int iTamanioReg;
    char *pQuery;
    char *pResultados;
    
    
   fdHeader = fopen(Config.archivoHeader,"rb");
   fdBody = fopen(Config.archivoBody,"rb");
   
   if(!(fdHeader) || !(fdBody))
	Log("No existe archivo de cache guardado...",LOGINFO);
   else{
        Log("Se abrio el archivo de cache para recuperar las respuestas guardadas...",LOGINFO);
        
        header = (TipoArchHeader *) malloc(sizeof(TipoArchHeader));
        leidosHeader = fread(header, sizeof(TipoArchHeader), 1, fdHeader);
        iTamanioReg = header->iTamResultados + header->iTamQuery + 1;
        
        body= (TipoArchBody *) malloc(sizeof(TipoArchBody));
        body->strResultados = (char *) malloc(iTamanioReg);
        leidosBody = fread(body->strResultados , iTamanioReg, 1, fdBody);
        
        /*EL FEOF NO VA SOLO ESTA A MODO DE ENTENDER EL ALGORITMO PARA SABER CON QUE TENGO QUE COMPLETAR*/	
	while (leidosHeader != 0) {
	   nuevoNodoRta = CrearElementoRta( sizeof(TipoNodoRta));
	   pQuery = strtok(body->strResultados,"|");
	   pResultados = strtok(NULL,"|");
	   
	   nuevoNodoRta->dato->iTamResultados = header->iTamResultados ;
	   nuevoNodoRta->dato->tTiempoUltimoUso = header->tTiempoUltimoUso;
	   nuevoNodoRta->dato->iCantUtilizado = header->iCantUtilizado;
	   
	   nuevoNodoRta->dato->query = pQuery;
	   iTamanioReg = header->iTamResultados + header->iTamQuery + 1;
	   
	   nuevoNodoRta->dato->strResultados = (char *) malloc(iTamanioReg);
           memcpy(nuevoNodoRta->dato->strResultados,pResultados ,nuevoNodoRta->dato->iTamResultados );
            
	   AgregarElementoRta((TipoListaRta * ) listaResultados,(TipoNodoRta *) nuevoNodoRta);
           
           leidosHeader = fread(header, sizeof(TipoArchHeader), 1, fdHeader);
           iTamanioReg = header->iTamResultados + header->iTamQuery + 1;
           
           body->strResultados = (char *) malloc(iTamanioReg);
           leidosBody = fread(body->strResultados , iTamanioReg, 1, fdBody);
	}
	fclose(fdHeader);
	fclose(fdBody);
   }
}

/************************************************************************************************/
/************************************************************************************************/


void func_guardarResultados(TipoListaRta * listaResultados){
    FILE *fdHeader;
    FILE *fdBody;
    TipoArchHeader header;
    TipoArchBody body;
    int iTamanioReg;
        
    TipoNodoRta* puntero = (TipoNodoRta *) listaResultados->primero;
    TipoResultados *nodoResultado;
    
   fdHeader = fopen(Config.archivoHeader,"wb");
   fdBody = fopen(Config.archivoBody,"wb");
   
   if(!(fdHeader) || !(fdBody))
	Log("Error en la apertura del archivo de cache en disco para ser grabado...",LOGERROR);
   else{
        Log("Se abrio el archivo de cache en disco para ser grabado...",LOGINFO);
	while (puntero != NULL) {
	
            nodoResultado = puntero->dato;
            header.iTamResultados = nodoResultado->iTamResultados;
            header.tTiempoUltimoUso = nodoResultado->tTiempoUltimoUso;
            header.iCantUtilizado = nodoResultado->iCantUtilizado ;
            header.iTamQuery= strlen(nodoResultado->query);
            
            iTamanioReg = header.iTamResultados + header.iTamQuery + 1;
            
            body.strResultados = (char *) calloc(iTamanioReg,sizeof(char));
            strcat(body.strResultados,nodoResultado->query);
            strcat(body.strResultados,"|");
            memcpy(body.strResultados + (header.iTamQuery + 1),nodoResultado->strResultados,header.iTamResultados);
            
            fwrite(&header, sizeof(header), 1, fdHeader);
            fwrite(body.strResultados, iTamanioReg , 1, fdBody);
            free(body.strResultados);
            puntero = puntero->siguiente;
	}
	fclose(fdHeader);
	fclose(fdBody);
   }
}

/************************************************************************************************/
/************************************************************************************************/

void func_agregarResultado(TipoListaRta * listaResultados,TipoNodoRta * nodoRta,TipoMensajeIPC stcMensaje){
    char * pcharQuery;
    char * pcharResultados;
    char *pcharSeparador = "|";
    
    pcharQuery = strtok(stcMensaje.mensaje, pcharSeparador);
		
    pcharResultados = (char *) malloc(strlen(stcMensaje.mensaje));
    pcharResultados =strtok(NULL, pcharSeparador);
    
    /****************************************************/
    /*Agrego el cliente que hizo la peticion a la lista*/
    /***************************************************/
    nodoRta = (TipoNodoRta *) CrearElementoRta(sizeof(TipoNodoRta));
    nodoRta->dato->query = (char *) malloc(strlen(pcharQuery));
    strcpy(nodoRta->dato->query,pcharQuery);
    nodoRta->dato->iTamResultados = stcMensaje.nLongMensaje - strlen(pcharQuery);
    nodoRta->dato->strResultados = (char *) malloc(nodoRta->dato->iTamResultados);
    memcpy(nodoRta->dato->strResultados,pcharResultados,nodoRta->dato->iTamResultados );
    nodoRta->dato->tTiempoUltimoUso = time(NULL);
    nodoRta->dato->tTiempoCreacion = time(NULL);
    
    nodoRta->dato->iCantUtilizado = 0;
    AgregarElementoRta((TipoListaRta * ) listaResultados, (TipoNodoRta *) nodoRta);
/*    
    free(pcharResultados);
    free(nodoRta->dato->query);
    free(nodoRta->dato->strResultados);
*/    
}
/************************************************************************************************/
/************************************************************************************************/
int func_responderSrvNet(TipoMensajeIPC stcMensaje, TipoNodoRta *stcNodoBuscado, int isock_srvNet){
    TipoMensajeIPC stcMensajeIPC;

    strcpy(stcMensajeIPC.IDMensaje,stcMensaje.IDMensaje);
    stcMensajeIPC.ucTipo = IPC_QUERYHIT;
    stcMensajeIPC.nLongMensaje = 0;
    
    if (stcNodoBuscado == NULL){/*NO ESTA EN CACHE*/
	/* Envio respuesta al servidor de red que no 
	   se encontro QueryHit para esta peticion*/
	if(send(isock_srvNet,(TipoMensajeIPC *)&stcMensajeIPC,sizeof(TipoMensajeIPC),0)==-1){
	    perror("Ocurrio un error al enviar el Header del Query al Cache");
	    Log("No se pudo mandar el Header del Query al Cache",LOGERROR);
	    return -1;
        }
        Log("El query NO se encuentra en Cache...",LOGINFO);
        return 0;
    }else{ /*LA RESPUESTA ESTA EN CACHE*/
        stcMensajeIPC.nLongMensaje = stcNodoBuscado->dato->iTamResultados;
        stcMensajeIPC.mensaje = (char *) malloc(stcMensajeIPC.nLongMensaje);
        memcpy(stcMensajeIPC.mensaje,stcNodoBuscado->dato->strResultados,stcMensajeIPC.nLongMensaje);
        Log("El query se encuentra en Cache...",LOGINFO);
	/***********************************************************************/
	/*Envia el header IPC al servidor cache */
	/***********************************************************************/
	if(send(isock_srvNet,(TipoMensajeIPC *)&stcMensajeIPC,sizeof(TipoMensajeIPC),0)==-1){
	   perror("Ocurrio un error al enviar el Header del Query al Cache");
	   Log("No se pudo mandar el Header del Query al Cache",LOGERROR);
	   return -1;
	}
	/*Envio el queryhit que se encontro para esta peticion*/
	if(send(isock_srvNet,(char *) stcMensajeIPC.mensaje,stcMensajeIPC.nLongMensaje,0)==-1){
	    perror("Ocurrio un error al mandar la respuesta al servidor de red");
	    Log("No pudo mandar el mensaje de Query Hit al servidor de Red",LOGERROR);
	   return -1;
	}
        Log("Fue enviada la respuesta...",LOGINFO);
	/*Actualizo los datos para las estadisticas que despues usan los algoritmos LRU y LFU*/
	stcNodoBuscado->dato->iCantUtilizado++;
	stcNodoBuscado->dato->tTiempoUltimoUso = time(NULL);
	return 0;
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


/* ====================================================================================== */
/* FUNCIONES DE LISTAS                                                                    */
/* ====================================================================================== */

TipoNodoRta* CrearElementoRta(int tamanio) {
	TipoNodoRta *nuevo;
	/* Reserva memoria para el nuevo elemento */
	if ((nuevo = (TipoNodoRta *) malloc(tamanio)) != NULL) {

		nuevo->siguiente = NULL;
                nuevo->dato = (TipoResultados *) malloc(sizeof(TipoResultados));

		return nuevo;
	} else {

		return NULL;
	}


}


int AgregarElementoRta(TipoListaRta *lista, TipoNodoRta* elemento) {
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

int AgregarElementoPilaRta(TipoListaRta *lista, TipoNodoRta* elemento) {
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

TipoNodoRta* ExtraerPrimerElementoRta(TipoListaRta *lista) {
    TipoNodoRta* auxiliar = NULL;
    
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

int func_esMismoQuery(void *dato,TipoNodoRta *elemento){
	if (strcmp(((TipoResultados *)dato)->query,elemento->dato->query) == 0)
		return 1;
	else
		return 0;
}

int func_MenosRecientemente(void *dato,TipoNodoRta *elemento){
	if (((TipoResultados *)dato)->tTiempoUltimoUso < elemento->dato->tTiempoUltimoUso)
		return 1;
	else
		return 0;
}

int func_MenosFrecuentemente(void *dato,TipoNodoRta *elemento){
	if (((TipoResultados *)dato)->iCantUtilizado < elemento->dato->iCantUtilizado)
		return 1;
	else
		return 0;
}

TipoNodoRta* ExtraerElementoRta(TipoListaRta *lista, void* dato, int (*punteroFuncion) (void*, void*)) {
	TipoNodoRta* auxiliar = NULL;
	TipoNodoRta* puntero = lista->primero;
	TipoNodoRta* anterior = NULL;

	while (auxiliar == NULL && puntero != NULL) {
		if (punteroFuncion(((TipoNodoRta *)dato)->dato, puntero)) { /* Lo encontro */
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

TipoNodoRta* BuscarElementoRta(TipoListaRta *lista, void* dato, int (*punteroFuncion) (void*, void*)) {
	TipoNodoRta* auxiliar = NULL;
	TipoNodoRta* puntero = lista->primero;

	while (auxiliar == NULL && puntero != NULL) {
		if (punteroFuncion(dato, puntero)) { /* Lo encontro */
			auxiliar = puntero;
		} else {
			puntero = puntero->siguiente;
		}
	}

	return auxiliar;
}

TipoNodoRta* BuscarRta(TipoListaRta *lista, int (*punteroFuncion) (void*, void*)) {
    TipoNodoRta* auxiliar = NULL;
    TipoNodoRta* puntero = lista->primero;

    while (puntero != NULL && puntero->siguiente != NULL) {
        if (punteroFuncion(puntero->dato, puntero->siguiente)) { /* Lo encontro */
            if (Config.iAlgoritmo == LRU && auxiliar != NULL)
                if (auxiliar->dato->tTiempoUltimoUso > puntero->dato->tTiempoUltimoUso)
                     auxiliar = puntero;
            if (Config.iAlgoritmo == LFU && auxiliar != NULL)
                if (auxiliar->dato->iCantUtilizado > puntero->dato->iCantUtilizado)
                     auxiliar = puntero;
            if (auxiliar == NULL)
                auxiliar = puntero;
                
            /*auxiliar = puntero;*/
        } else {
            if (Config.iAlgoritmo == LRU){
                if (auxiliar != NULL){
                    if (auxiliar->dato->tTiempoUltimoUso > puntero->siguiente->dato->tTiempoUltimoUso)
                        auxiliar = puntero->siguiente;
                }else
                    auxiliar = puntero->siguiente;
            }
            if (Config.iAlgoritmo == LFU){
                if (auxiliar != NULL){
                    if (auxiliar->dato->iCantUtilizado > puntero->siguiente->dato->iCantUtilizado)
                        auxiliar = puntero->siguiente;
                }else
                    auxiliar = puntero->siguiente;
            }
            /*auxiliar = puntero->siguiente;*/
        }
        puntero = puntero->siguiente;
    }

    return auxiliar;
}


void EliminarListaRta(TipoListaRta *lista) {
	TipoNodoRta* auxiliar = NULL;
	TipoNodoRta* puntero = lista->primero;

	while (puntero != NULL) {
		auxiliar = puntero;
		puntero = puntero->siguiente;
		free(auxiliar);
	}

	free(lista);

	lista = NULL;
}


int ContarElementosListaRta(TipoListaRta *lista) {
	TipoNodoRta* puntero = lista->primero;
	int cantidad = 0;

	while (puntero != NULL) {
		puntero = puntero->siguiente;
		cantidad++;
	}

	return cantidad;
}

int func_ListaLlena(TipoListaRta *lista){
    func_ActualizaTiempoVida(lista);
    if (Config.iCantMaximo <= ContarElementosListaRta(lista)){
        return 1;
    }else{
        return 0;
    }
}

void func_ActualizaTiempoVida(TipoListaRta *lista){
	TipoNodoRta* puntero = lista->primero;
	TipoNodoRta* pExtraer = NULL;
	time_t tTranscurrido;
        
	while (puntero != NULL) {
	   tTranscurrido = (time(NULL) - puntero->dato->tTiempoCreacion);
	   if (tTranscurrido > Config.iTiempoVida){
	       pExtraer = puntero;
	       puntero = puntero->siguiente;
	       ExtraerElementoRta(lista, pExtraer, (void *) func_esMismoQuery) ;
	       free(pExtraer);
	       pExtraer = NULL;
	   }else{
	       puntero = puntero->siguiente;
	   }
	   
	}

}
