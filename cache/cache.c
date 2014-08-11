/***************************************************************************
 *   Copyright (C) 2008 by Intrepid Programmers  *
 ***************************************************************************/


#include "cache.h"
extern TipoConfiguracion Config; /*estructura de configuracion*/
char nombreModulo[LARGO_NOMBRE_MODULO];
extern TipoListaRta *listaResultados;

int main(int argc, char *argv[])
{
    struct sockaddr_in sckLocal;
    struct sockaddr_in sckSrvNet;
    struct hostent *stcNombre_host;
    int iPuertoSrvNet;
    int iPuertoLocal;
    int isock_srvNet;
    int isock_local;
    int iTipoMensaje;
    TipoMensajeIPC stcMensaje;
    TipoNodoRta *nodoRta;
    /*TipoListaRta *listaResultados;*/
    TipoNodoRta *stcNodoBuscado;
    /*TipoNodoRta stcNodoBusqueda;*/
    TipoResultados stcBuscado;
    char *mensaje;
    
    /*******************************/
    /*PARA IMPLEMENTACION DE SELECT*/
    /*******************************/
    int visockCliente[MAXCLIENTES]; /* Descriptores de sockets con clientes */
    fd_set descriptoresLectura;	    /* Descriptores de interes para select() */
    int numeroClientes = 0;	    /* Número clientes conectados */
    /*******************************/
    /*char vcharQuery[1024];*/
    int (*funcionSeleccion) (void*, void*);
    /*int iIndice;*/
    
    
    /**************************************************/
    /*CAMBIO EL NOMBRE DEL PROCESO                   */	
    /*************************************************/
    CargarConfiguracion("configuracion");
    strncpy(argv[0],Config.nombreModulo,strlen(argv[0]));
    sprintf(nombreModulo, "Cache[%d]", getpid());
    
    iPuertoSrvNet = atoi(Config.puertoSrvNet);
    iPuertoLocal = atoi(Config.puertoLocal);
    
    if((stcNombre_host=gethostbyname(Config.ipSrvNet))==NULL){
            perror("Ocurrio un error al abrir el socket");
            Log("Ocurrio un error al abrir el socket",LOGERROR);
            return 1; 
    }
    
    /*************************************************************/
    /* CREA EL SOCKET PARA CONECTARSE CON EL SERVIDOR DE RED*/
    /*************************************************************/
    if ((isock_srvNet=socket(AF_INET, SOCK_STREAM, 0))==-1){
            perror("Ocurrio un error al abrir el socket");
            Log("Ocurrio un error al abrir el socket",LOGERROR);
            exit(0);
    }
    
    
    /**************************************************/
    /*Configura la estructura del servidor de Red*/
    /**************************************************/
    sckSrvNet.sin_family= AF_INET; 
    sckSrvNet.sin_port = htons(iPuertoSrvNet); 
    sckSrvNet.sin_addr= *((struct in_addr *)stcNombre_host->h_addr); 
    
    /*************************************************/
    /*Configura la estructura local                  */
    /*************************************************/
    Log("Configurando socket local... ",LOGINFO);
    sckLocal.sin_family = AF_INET;
    sckLocal.sin_addr.s_addr = INADDR_ANY;
    sckLocal.sin_port = htons(atoi(Config.puertoLocal));
    Log("OK Configuracion Socket Local",LOGINFO);
    
        
    /*************************************************************/
    /* CREA EL SOCKET PARA PARA BINDEARLO CON EL CLIENTE WINDOWS */
    /*************************************************************/
    if ((isock_local=socket(AF_INET, SOCK_STREAM, 0))==-1){
	    perror("Ocurrio un error al crear el socket Local");
	    Log("Ocurrio un error al crear el socket Local",LOGERROR);
	    exit(0);
	}
    /*************************************************/
    /*Configura la estructura local                  */
    /*************************************************/
    Log("Configurando socket local... ",LOGINFO);
    sckLocal.sin_family = AF_INET;
    sckLocal.sin_addr.s_addr = INADDR_ANY;
    sckLocal.sin_port = htons(atoi(Config.puertoLocal));
    Log("OK Configuracion Socket Local",LOGINFO);
/*    if (bind(isock_local, (struct sockaddr *)&sckLocal, sizeof(sckLocal)) == -1){
	    perror("Error en la llamada para unir");
	    Log("Error en la funcion Bind del srvNet (isock_local)",LOGERROR);
	    exit(-1);
    }
*/
    Log("Funcion listen... ",LOGINFO);
    if (listen(isock_local,20) == -1){
	    perror("llamada para escuchar");
	    Log("No pudo ejecutarse con exito la funcion LISTEN",LOGERROR);
	    exit(1);
    }
    
    /***********************************/
    /* Inicializo la lista de clientes */
    /***********************************/
    listaResultados = (TipoListaRta *) malloc(sizeof(NULL)*2);
    listaResultados->primero = NULL;
    listaResultados->ultimo = NULL;
    
    /*
    Esta funcion deberia levantar el archivo 
    de respuestas guardado, y llenar la lista
    */
    /*func_llenarLista(listaResultados)*/
    
    /*Inicializo las alarmas.*/
    func_PreparaAtSeniales();
    /***********************************************************************/
    /*Se identifica el servidor de Cache*/
    /***********************************************************************/
    if (func_Identificacion(isock_srvNet, &sckSrvNet) == -1){	
            perror("No se pudo identificar el servidor");
            Log("No se pudo identificar el servidor de red",LOGERROR);
            exit(0);
    }

    /* Acepta la conexión con el cliente, guardándola en el array */
    visockCliente[numeroClientes] = isock_srvNet;
    numeroClientes++;
    func_levantarResultados(listaResultados);
    while (1){
    
        iTipoMensaje=func_recibirSelect(isock_local,&descriptoresLectura,visockCliente,&isock_srvNet,&numeroClientes,&stcMensaje);
        
        if (iTipoMensaje == IPC_QUERY){

            func_ActualizaTiempoVida(listaResultados);
            stcBuscado.query = (char *) calloc( strlen(stcMensaje.mensaje), sizeof(char));
            strcpy(stcBuscado.query,stcMensaje.mensaje);
            
            stcNodoBuscado = BuscarElementoRta((TipoListaRta *)listaResultados, (void *) &stcBuscado, (void *) func_esMismoQuery );
            func_responderSrvNet(stcMensaje, stcNodoBuscado, isock_srvNet);
            free(stcBuscado.query);
        }
            
        /*Cuando se le mando la peticion al servidor Gnutella
          este me responde y yo tengo que agregar la respuesta
          a mi lista */
        if (iTipoMensaje == IPC_QUERYHIT){
            /*SELECCIONO UNA VICTIMA SEGUN EL ALGORITMO UTILIZADO Y LA BORRO PARA QUE ENTRE EL QUERYHIT QUE ACABA DE ENTRAR*/
            if (func_ListaLlena(listaResultados)){
                
                /*func_guardarResultados(listaResultados);*/
                
                if (Config.iAlgoritmo == LRU){
                    funcionSeleccion = (void *) func_MenosRecientemente;
                }else if (Config.iAlgoritmo == LFU){
                    funcionSeleccion = (void *) func_MenosFrecuentemente;
                }
                stcNodoBuscado = (TipoNodoRta *) BuscarRta((TipoListaRta *) listaResultados,(void *) funcionSeleccion);
                ExtraerElementoRta(listaResultados, stcNodoBuscado , (void *) func_esMismoQuery) ;
                mensaje = (char *) malloc(strlen("Se removio el query: ") + strlen(stcNodoBuscado->dato->query));
                sprintf(mensaje, "Se removio el query: [%s]", stcNodoBuscado->dato->query);
                Log(mensaje,LOGINFO);
                free(mensaje);
                /*free(stcNodoBuscado);*/
            }
            
            /*PARA TEST*/
            /*for (iIndice=0;iIndice<stcMensaje.nLongMensaje;iIndice++){*/
            /*    vcharQuery[iIndice] = stcMensaje.mensaje[iIndice];*/
            /*}*/
            /* */    
            /**/
            /**************/
            Log("Se agrego el queryhit a la lista de cache...",LOGINFO);
            func_agregarResultado(listaResultados,nodoRta,stcMensaje);

        }
    }
	
}
