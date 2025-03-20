#ifndef _UNIRAIL_GESTION_REQ_H_
	#define _UNIRAIL_GESTION_REQ_H_
	#include "../../utility/include/comm.h"

	typedef struct {
		int sd;
		struct sockaddr_in * client_adr;
		message_t recv_message;
		int can_socket;
	} handle_and_respond_args_t;

	/**
	 * @brief Handles a request and sends a response. Entry point for a new thread.
	 * @param args A handle_and_respond_args_t struct containing the received message, the client address and the socket descriptor
	 */
	void * handle_and_respond(void * args);

	/**
	 * @brief Handles a request and prepares a response
	 * @param recv_message The message that was received to handle
	 * @param send_message The message in which to store the response
	 * @param can_socket The CAN socket to use
	 * @param recv_adr The address of the sender
	 */
	void handle_request(message_t recv_message, message_t * send_message, int can_socket, struct sockaddr_in * recv_adr, int sd);

	typedef struct {
		int can_socket;
		message_t recv_message;
		struct sockaddr_in * recv_adr;
		int sd;
	} handle_eoa_request_args_t;

	/**
	 * @brief Handles a request for a new EOA (needs to be called in a separate thread)
	 * @param args A handle_eoa_request_args_t struct containing the received message and the CAN socket
	 */
	void * handle_eoa_request(void * args);
#endif