#include "../include/comm_message_listener.h"
#include "../include/comm.h"

// Linked list of waiting responses
pending_message_t *message_list = NULL;
pthread_mutex_t message_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void wait_for_response(int req_id, message_t *recv_message, int timeout_sec) {
    struct timespec ts;
	// Allocate a new response struct
    pending_message_t *response = malloc(sizeof(pending_message_t));
    if (!response) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

	// Initialize a waiting response struct
    response->req_id = req_id;
    response->ready = 0;
    pthread_cond_init(&response->cond, NULL);
    pthread_mutex_init(&response->mutex, NULL);
    response->next = NULL;


	// Add it at the beginning of the linked list
    pthread_mutex_lock(&message_list_mutex);
    response->next = message_list;
    message_list = response;
    pthread_mutex_unlock(&message_list_mutex);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_sec;
	// Awaits for the condition to be signaled
    pthread_mutex_lock(&response->mutex);
    while (!response->ready) {
        if (timeout_sec == 0) {
            pthread_cond_wait(&response->cond, &response->mutex);
        } else {
            int result = pthread_cond_timedwait(&response->cond, &response->mutex, &ts);
            if (result == ETIMEDOUT) {
                response->message.code = -1;
                response->message.data[0] = NULL;
                response->ready = 1;
                fprintf(stderr, "EVC - Pas de réponse reçue la requête %d après %d secondes\n", req_id, timeout_sec);
                break;
            }
        }
    }
    pthread_mutex_unlock(&response->mutex);
	
	// Copies the received message to the provided struct
	*recv_message = response->message;

	// Finds and removes the response from the linked list
    pthread_mutex_lock(&message_list_mutex);
    if (message_list == response) {
        message_list = response->next;
    } else {
        pending_message_t *prev = message_list;
        while (prev && prev->next != response)
            prev = prev->next;
        if (prev)
            prev->next = response->next;
    }
    pthread_mutex_unlock(&message_list_mutex);

	// Cleanup
    pthread_cond_destroy(&response->cond);
    pthread_mutex_destroy(&response->mutex);
    free(response);

}

void *message_listener(void * args) {
    message_t recv_message;
	message_listener_args_t *mla = (message_listener_args_t *) args;

    while (1) {

        struct sockaddr_in *recv_adr = malloc(sizeof(struct sockaddr_in));
		
		receive_data(mla->sd, recv_adr, &recv_message, 0);

        pthread_mutex_lock(&message_list_mutex);
        pending_message_t *pending_message = message_list;
        // Looks for the received message in the waiting list, adds it to the pending message struct and signals the condition
        while (pending_message != NULL) {
            if (pending_message->req_id == recv_message.req_id) {
                pthread_mutex_lock(&pending_message->mutex);

                message_t * copied_message = copy_message(&recv_message);
                pending_message->message = *copied_message;

                free(copied_message);
                free(recv_adr);
                pending_message->ready = 1;

                pthread_cond_signal(&pending_message->cond);
                pthread_mutex_unlock(&pending_message->mutex);
                break;
            }
            pending_message = pending_message->next;
        }
        pthread_mutex_unlock(&message_list_mutex);

        // If the message was not found in the waiting list, it is a request, so it is stored in the pending request struct
        if (pending_message == NULL) {
            pthread_mutex_lock(&mla->pending_request->mutex);
            
            message_t * copied_message = copy_message(&recv_message);
            mla->pending_request->message = *copied_message;
            mla->pending_request->recv_adr = recv_adr;

            free(copied_message);
            mla->pending_request->ready = 1;

            pthread_cond_signal(&mla->pending_request->cond);
            pthread_mutex_unlock(&mla->pending_request->mutex);
        }

    }
    return NULL;
}
