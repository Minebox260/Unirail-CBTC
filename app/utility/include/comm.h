#ifndef _UNIRAIL_COMM_H
	#define _UNIRAIL_COMM_H
	
	/**
	 * UNIRAIL - Communication
	 * ========================
	 * Requêtes
	 * ---------
	 * 100 - Vérification de la connexion au RBC
	 * 101 - Rapport de position au RBC (DATA: [balise, position relative])
	 * 102 - Demande d'autorisation de mouvement au RBC
	 * 103 - Envoi d'une nouvelle mission à l'EVC (DATA: [nombre de tours])
	 * 104 - Envoi d'une nouvelle autorisation de mouvement à l'EVC (DATA: [balise EOA, position relative EOA])
	 * ---------
	 * Acknowledges
	 * ---------
	 * 200 - Connexion établie
	 * 201 - Rapport de position reçu
	 * 202 - Demande d'autorisation de mouvement reçue et en cours de traitement
	 * 203 - Mission reçue
	 * 204 - Nouvelle autorisation de mouvement reçue
	 * ---------
	 * Erreurs
	 * ---------
	 * 400 - Code de requête inconnu
	 * 401 - Impossible de lire la position reçue
	 * 404 - Impossible de lire l'EOA reçue
	 * 405 - ID de train inconnue
	 * ========================
	 */

	#define MAXOCTETS 150
	#define MAXDATA 2
	#define CHECK_ERROR(val1,val2,msg)   if (val1==val2) \
										{ perror(msg); \
											exit(EXIT_FAILURE); }
	
	#include <arpa/inet.h>
	#include <netinet/in.h>

	typedef struct {
		int req_id;
		int train_id;
		int code;
		char *data[MAXDATA];
	} message_t;

	typedef struct {
		int sd;
		struct sockaddr_in adr_serv;
		socklen_t adr_serv_len;
	} client_udp_init_t;

	/**
	 *	@brief Generates a unique request ID
	 *	@return The generated request ID
	**/
	int generate_unique_req_id();

	/** 
	 *	@brief Creates and binds a UDP server socket
	 *	@param port The port to bind the server socket to
	 *	@return The file descriptor of the server socket 
	**/
	int setup_udp_server(int port);

	/** 
	 *	@brief Creates a UDP client socket and initializes the client struct
	 *	@param client The client_udp_init_t struct to initialize
	 *	@param server_ip The IP address of the server
	 *	@param server_port The port of the server
	 *	@param client_port The port of the client. If 0, the OS will assign a port
	**/
	void setup_udp_client(client_udp_init_t * client, char * server_ip, int server_port, int client_port);

	/** 
	 *	@brief Sends a message to the server
	 *	@param sd The file descriptor of the client socket
	 *	@param send_adr The address of the server
	 *	@param message The message struct to send
	**/
	void send_data(int sd, struct sockaddr_in send_adr, message_t message);

	/**
	 * 	@brief Awaits a message (blocking)
	 * 	@param sd The file descriptor of the socket to use
	 *  @param recv_adr The address in which to store the sender's address
	 *  @param message The message struct to store the received message
	 */
	void receive_data(int sd, struct sockaddr_in * recv_adr, message_t *message, int timeout_sec);

	message_t * copy_message(message_t *src);
#endif