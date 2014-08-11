/***************************************************************************
 *   Copyright (C) 2008 by Alejandro Rothar *
 ***************************************************************************/

#include <netinet/in.h>
#include "upload.h"

int port; /*Puerto de escucha*/
extern TipoConfiguracion Config; /*estructura de configuracion*/
char nombreModulo[LARGO_NOMBRE_MODULO];
char IDServer[16];


int main(int argc, char *argv[]){
	struct sockaddr_in local;
	struct sockaddr_in remoto;
	int sock_escucha; /*Descriptor de server*/
	int address_size; /*Se usa para saber el tamanio de sockaddr_in*/
	int pid_child; /*Se usa para saber el pid del hijo*/
	int conexion=1, conexionGnu=0;
	char buf[16384];
	int leng=0;
	fd_set descriptoresLectura;
	int socketCliente[100];
	int numeroClientes;
	char optimizador[1024];
	int i, maximo;
	ListaQuery *listaQuery=NULL;
	ListaPing *listaPing=NULL;
	ListaVecino *listaVecino=NULL;
	ListaGnu *listaGnu=NULL;
	char buffer04[1000];
	TipoHeaderTP *buffer4;
	struct timeval tiempoSelect;
	int opcionSelect;
	int islas=0;
	int lenrecv;
	int chequeador;
	int corredor=0, basura=55;
	Log("Iniciando Gnutella", LOGINFO);
	sleep(1);
	
	chequeador=0;
	/*************************************************/
	/*CAMBIO EL NOMBRE DEL PROCESO                   */
	/*************************************************/
	strcpy(argv[0],"Gnutella_SRV");
	strcpy(nombreModulo,argv[0]);
	sprintf(nombreModulo, "Gnutella_SRV[%d]", getppid());
	Log("Renombrando... OK!", LOGINFO);
	
	/*************************************************/
	/*CARGA LA CONFIGURACION DESDE EL ARCHIVO DE CONF*/	
	/*************************************************/
	CargarConfiguracion("configuracion");
	port = atoi(Config.puertoGnutella);
	Log("Archivo de Configuracion cargado... OK!", LOGINFO);
	
	/*************************************************/
	/* CREA EL SOCKET PARA ESCUCHAR                  */
	/*************************************************/
	sock_escucha = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_escucha == -1){
		Log("Error creando el socket", LOGERROR);
		exit(1);
	}
	Log("Se ha creado el socket... OK!", LOGINFO);
	
	/*************************************************/
	/*Configura la estructura local                  */
	/*************************************************/
	bzero((char *)&local, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(port);
	Log("Configuracion del socket... OK!", LOGINFO);
	
	/***************************************************/
	/*Asocia el sock de escucha con la estructura local */
	/***************************************************/
	while(bind(sock_escucha, (struct sockaddr *)&local, sizeof(local)) == -1){
		local.sin_port=htons(++port);
		printf("\a");
		Log("Se ha incrementado el puerto", LOGINFO);
	}
	Log("Se asocio el sock de escucha... OK!", LOGINFO);
	
	
	if (listen(sock_escucha,20) == -1){
		Log("Error en la funcion listen.", LOGERROR);
		exit(1);
	}
	Log("Funcion listen... OK!", LOGINFO);
	
	/*Intento de conexion con los nodos*/
	if(func_comunicacionPingPong(&socketCliente[0], &listaPing, &listaVecino, &listaGnu, 0, &corredor)){
		Log("No hay nodos disponibles", LOGINFO);
	}
	else{
		numeroClientes++;
		Log("Conexion con nodo... OK!", LOGINFO);
	}
	
	printf("\nNombre del proceso: %s\n", "Gnutella");
	printf("Direccion ip local: %s\n", Config.ipLocal);
	printf("El puerto de escucha: %d\n", port);
	
	/*Este signal lo que hace es ignorar las llamadas de los hijos cuando terminan, de esta forma,*/
	/*el hijo puede terminar bien y no convertirse en un zombie.                                  */
	signal(SIGCHLD, SIG_IGN); 
	
	/*Se queda aceptando a los clientes, este va a devolver el descriptor del nuevo cliente.*/
	strcpy(optimizador, "find ");
	strcat(optimizador, Config.rutaCompartida);
	strcat(optimizador, " -iname ···· > optimizador");
	
	system(optimizador);
	system("rm optimizador");
	
	Log("El servidor GNUTELLA esta aceptando conexiones", LOGINFO);	
	while (1){
		compactaClaves (socketCliente, &numeroClientes);
		if((listaGnu!=NULL)&&listaVecino==NULL){
			
			if(conexionGnu>0){
				listaPing=NULL;
				listaVecino=NULL;
				if(func_comunicacionPingPong(&socketCliente[numeroClientes], &listaPing, &listaVecino, &listaGnu, 1, &corredor)){
					Log("No se pudo conectar a ningun nodo.", LOGINFO);
				}
				else{
					numeroClientes++;
					conexionGnu=0;
					compactaClaves (socketCliente, &numeroClientes);
					Log("Se reconecto a una red Gnutella.", LOGINFO);
				}
			}
		}
		
		FD_ZERO (&descriptoresLectura);
		FD_SET (sock_escucha, &descriptoresLectura);
		
		tiempoSelect.tv_sec=atoi(Config.timeSelect);
		tiempoSelect.tv_usec=0;
		
		for (i=0; i<numeroClientes; i++)
			FD_SET (socketCliente[i], &descriptoresLectura);
	
		maximo = dameMaximo (socketCliente, numeroClientes);
		
		if (maximo < sock_escucha)
			maximo = sock_escucha;
		
		islas=0;
		chequeador=0;
		leng=0;
		opcionSelect=select (maximo + 1, &descriptoresLectura, NULL, NULL, &tiempoSelect);
		if(opcionSelect){
			for (i=0; i<numeroClientes; i++){
				if (FD_ISSET (socketCliente[i], &descriptoresLectura)){
					int  apuntador1=0, apuntador2=0;;
					int vectorDesconectados[10]={};
					func_envioRespuestaGNUTELLA(socketCliente[i], &port, nombreModulo, &listaQuery, &listaPing, &listaVecino, &listaGnu, &leng, &chequeador, vectorDesconectados, &conexionGnu);
					if(leng<=-1 || chequeador<=-1){
						conexion=1;
						if(leng==-1){
							if(conexionGnu!=0){
								if(func_eliminarVecino(&listaVecino, (int)socketCliente[i], &conexionGnu)==-1)
									Log("Un Cliente cerro la conexion", LOGINFO);
								else{
									Log("Un vecino cerro conexion", LOGINFO);
									
								}
							}
							else{
								if(func_eliminarVecino(&listaVecino, (int)socketCliente[i], &basura)==-1)
									Log("Un Cliente cerro la conexion", LOGINFO);
								else{
									Log("Un vecino cerro conexion", LOGINFO);
									
								}
							
							}
							func_eliminarPing(&listaPing, socketCliente[i]);
							close(socketCliente[i]);
							socketCliente[i] = -1;
							islas=1;
						}
						else{
							if(chequeador==-1){
								Log("Un vecino cerro conexion", LOGINFO);
								while(vectorDesconectados[apuntador1]>0){
									while(socketCliente[apuntador2]!=vectorDesconectados[apuntador1])
										apuntador2++;
									if(apuntador2<10){
										func_eliminarPing(&listaPing, socketCliente[i]);
										close(socketCliente[apuntador2]);
										socketCliente[apuntador2]=-1;
									}
									apuntador1++;
								}
								islas=1;
							}
							else{
								islas=1;
							}
						}
					}
				}
			}
			if((islas==0)&&(chequeador==0)){
				if (FD_ISSET (sock_escucha, &descriptoresLectura)){
					address_size=sizeof(struct sockaddr_in);
					socketCliente[numeroClientes] =	accept(sock_escucha, (struct sockaddr *)&remoto,(unsigned int *) &address_size);
					if(socketCliente[numeroClientes] == -1){
						Log("Error al aceptar un nuevo cliente !!!", LOGERROR);
					}
					else{
						Log("Recibiendo informacion",LOGINFO);
						if((lenrecv=recv(socketCliente[numeroClientes],(TipoHeaderTP *)&buffer04,sizeof(TipoHeaderTP),0)) <= 0){
							Log("Error recibiendo heander de conexion", LOGERROR);
						}
						Log("La informacion se ha recibido correctamente",LOGINFO);
						buffer4=(TipoHeaderTP *)buffer04;
						if(buffer4->ucTipo==HANDSHAKE){
							Log("Encabezado del HANDSHAKE recibido",LOGINFO);
							Log("Recibiendo cuerpo del HANDSHAKE",LOGINFO);
							if(recv(socketCliente[numeroClientes],buf,16384,0) == -1){
								Log("Error recibiendo informacion de HandShake... ERROR", LOGERROR);
							}
							else{
								Log("Cuerpo de Handshake recibido",LOGINFO);
								Log("Comprobando el cuerpo del HANDSHAKE",LOGINFO);
								if((strcmp(buf, GNUTELLA_SRV_CONNECT)) == 0){
									Log("Comprobacion del HANDSHAKE correcta",LOGINFO);
									Log("Envio del Header de respuesta al HANDSHAKE",LOGINFO);
									if(send(socketCliente[numeroClientes],(TipoHeaderTP*)&buffer04,sizeof(TipoHeaderTP),0)==-1){
										Log("Erro enviando header de confirmacion de conexion.", LOGERROR);
									}
									else{
										Log("Envio del Header del HANDSHAKE se completo de forma correcta",LOGINFO);
										Log("Envio del Cuerpo del HANDSHAKE",LOGINFO);
										if(send(socketCliente[numeroClientes],GNUTELLA_SRV_OK,strlen(GNUTELLA_SRV_OK),0)==-1){
											Log("Error enviando mensaje de confirmacion de conexion.", LOGERROR);
										}
										else{
											Log("Envio del cuerpo del HANDSHAKE se completo de forma correcta",LOGINFO);
											numeroClientes++;
											Log("Estableciendo conexion con Servidor... OK", LOGINFO);
										}
									}
									
								}
							}
						}
						else{
							if(strncmp(buffer04, "GET",3)==0){
								Log("Se ha recibido un mensaje HTTP",LOGINFO);
								Log("Cargando toda la informacion de la peticion HTTP",LOGINFO);
								if((memcmp(&buffer04[lenrecv-3], "\r\n\r\n", 4))){
									Log("Recibiendo la peticion HTTP", LOGINFO);
									recv(socketCliente[numeroClientes],&buffer04[lenrecv],(1000-lenrecv),0);
									Log("Carga de la informacion de la peticion HTTP se realizo correctamente",LOGINFO);
								}
								Log("Gestionando la descarga",LOGINFO);
								pid_child=fork();
								if(pid_child==0){
									char *nombreArchBuscado;
									char *pathArchivoBuscado;
									FILE *archivo;
									char *mensaje;
									int tamanioBuffer;
									int sock_clienT;
									
									sock_clienT=socketCliente[numeroClientes];
									/*sleep(1);*/
									Log("El servidor Upload esta lista para mandar la respuesta.",LOGINFO);
									
									/**********************************************************************/
									/*Obtengo el nombre del archivo que tengo que buscar en mi directorio*/
									/*********************************************************************/
									if((nombreArchBuscado = archivoBuscado(buffer04)) == NULL){
										Log("El formato del mensaje es incorrecto.",LOGERROR);
										exit(1);
									}
									if(nombreArchBuscado[0]=='/')
										strcpy(nombreArchBuscado, &nombreArchBuscado[1]);
									
									/******************************************************/
									/*Arma la ruta completa al archivo                    */
									/******************************************************/
									pathArchivoBuscado = (char *) calloc(strlen(Config.rutaCompartida) + LARGO_ARCHIVO_BUSCADO, sizeof(char));
									
									strcpy(pathArchivoBuscado,Config.rutaCompartida);
									/*Parche por el mal fin de archivo que se pueda producir*/
									if(pathArchivoBuscado[strlen(Config.rutaCompartida)-2]=='/')
										pathArchivoBuscado[strlen(Config.rutaCompartida)-1]='\0';
									strcat(pathArchivoBuscado,nombreArchBuscado);
									
									/******************************************************/
									/*Abre el archivo, si no lo encuentra manda el mensaje*/
									/******************************************************/
									archivo=fopen(pathArchivoBuscado,"rb");
									
									if(!archivo){
										
										/******************************************************/
										/*No lo encontro, manda el mensaje de no encontrado   */
										/******************************************************/
										mensajeArchivoNoEncontrado(nombreArchBuscado,&mensaje);
										Log("No se ha encontrado el archivo", LOGINFO);
										if((send(sock_clienT,mensaje,strlen(mensaje),0))==-1){
											Log("No se ha encontrado el archivo.",LOGERROR);
										}
									}else{
										/*************************************************/
										/*Envia el archivo al cliente                    */
										/*************************************************/
										mensajeArchivoEncontrado(nombreArchBuscado,&mensaje);
										tamanioBuffer = atoi(Config.tamanioBuffer);
										Log("Se ha encontrado el archivo", LOGINFO);
										if (enviarArchivo(sock_clienT,archivo,mensaje, tamanioBuffer) != -1)
											Log("Se ha completado la descarga del archivo", LOGINFO);
										fclose(archivo);
										
									}
									free(mensaje);
									free(nombreArchBuscado);
									free(pathArchivoBuscado);
									sleep(1);
									close(sock_clienT);
									exit(0);
								}
								else{
									if(pid_child>0){
										close(socketCliente[numeroClientes]);
									}
									else
										Log("Hubo un error al crear hijo.",LOGERROR);
								}
							}
							else{
								/*if(send(socketCliente[numeroClientes],GNUTELLA_SRV_ERROR,strlen(GNUTELLA_SRV_ERROR),0)==-1){
									Log("Error enviando mensaje de error... ERROR", LOGERROR);
								}
								else
									Log("Error en la informacion para aceptar conexion, enviando mensaje de error... OK", LOGERROR);*/
								Log("Se ha intentado conectar al Gnutella, pero la conexion es rechazada", LOGINFO);
								close(socketCliente[numeroClientes]);
								socketCliente[numeroClientes]=-1;
							}
							
						}
					}
				}
			}
		}
		else{
			func_eliminarQuery(&listaQuery);
		}
	}
	return 0;
}

