#ifndef _UNIRAIL_MESSAGE_LISTENER_H
	#define _UNIRAIL_MESSAGE_LISTENER_H

	#include <stdio.h>
	#include <stdlib.h>
	#include <pthread.h>
	#include <time.h>
	#include <sys/time.h>
	#include <errno.h>

	#include "../../utility/include/comm.h"

	/**
	 * @brief Awaits a response to a given request
	 * @param req_id The ID of the sent request to await the response for
	 * @param recv_message The message struct in which the received message will be stored
	 */
	void wait_for_response(int req_id, message_t *recv_messag, int timeout_sec);

	/**
	 * @struct pending_message_t
	 * @brief Linked list structure to store pending messages
	 * @param req_id The ID of the request to await the response for, if it is a pending response, or -1 if it is a pending request
	 * @param message The message struct in which the received message will be stored
	 * @param recv_adr The address of the sender of the pending message
	 * @param ready Flag to indicate if the message is ready to be read
	 * @param cond Thread condition to signal when the pending message is ready
	 * @param mutex Thread mutex to lock the pending message struct
	 * @param next Pointer to the next pending message in the linked list
	 */
	typedef struct pending_message_t {
		int req_id;
		message_t message;
		struct sockaddr_in *recv_adr;
		int ready;
		pthread_cond_t cond;
		pthread_mutex_t mutex;
		struct pending_message_t *next;
	} pending_message_t;

	/**
	 * @struct message_listener_args_t
	 * @brief Structure to pass arguments to the message listener thread
	 * @param sd The file descriptor of the socket to use
	 * @param pending_request The pending_message_t struct in which to store an incomming request
	 */
	typedef struct {
		int sd;
		pending_message_t * pending_request;
	} message_listener_args_t;

	/**
	 * @brief Entry point for the response listener thread
	 * @param args The response_listener_args_t struct containing the client struct
	 */
	void *message_listener(void * args);

#endif