#ifndef _UNIRAIL_CAN_H
	#define _UNIRAIL_CAN_H
	
	/**
	 * @brief Sends a CAN message.
	 * @param can_socket The socket file descriptor
	 * @param can_id The CAN ID
	 * @param can_dlc The data length code
	 * @param data The data to send (8 bytes)
	 * @return 0 if the message was sent successfully
	 * @return 1 if an error occured
	 */
	int send_can_data(int can_socket, int can_id, int can_dlc, int data[8]);

	/**
	 * @brief Initializes a CAN connection
	 * @return The socket file descriptor
	 * @return -1 if an error occured
	 */
	int init_can_socket();
#endif