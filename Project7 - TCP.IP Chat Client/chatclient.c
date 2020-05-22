/*******************************************************************************
 * Name        : chatclient.c
 * Author      : Siddhanth Patel
 * Partner     : Elijah Wendel
 * Date        : 01 May 2020
 * Description : Client that communicates with chatserver to form a chatroom.
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

bool starts_with(const char *str, const char *prefix) {
    /* TODO:
       Return true if the string starts with prefix, false otherwise.
       Note that prefix might be longer than the string itself.
     */
    if (strlen(prefix) > strlen(str)) {
      return false;
    }
    for (int i = 0; i < strlen(prefix); i++) {
      if (prefix[i] != str[i]) {
        return false;
      }
    }
    return true;
}


int handle_stdin() {
	int msg_status = get_string(outbuf, MAX_MSG_LEN + 1);
	if (msg_status == TOO_LONG) {
		printf("Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
	} else {
		if (send(client_socket, outbuf, strlen(outbuf), 0) < 0) {
	        fprintf(stderr, "Error: Failed to send message to server. %s.\n",
	                strerror(errno));
	    }
	    if (strcmp(outbuf, "bye") == 0) {
	    	printf("Goodbye.\n");
	    	return 1;
	    }
	}
	return 0;
}

int handle_client_socket() {
	int bytes_recvd;
	if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) == -1) {
        fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n",
                strerror(errno));
    }
    inbuf[bytes_recvd] = '\0';
    if (bytes_recvd == 0) {
    	fprintf(stderr, "\nConnection to server has been lost.\n");
    	return 1;
    } else if (strcmp(inbuf, "bye") == 0) {
    	printf("\nServer initiated shutdown.\n");
    	return 1;
    } else {
    	inbuf[bytes_recvd] = '\0';
    	printf("\n%s\n", inbuf);
    }
    return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <server ip> <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int bytes_recvd, ip_conversion, retval = EXIT_SUCCESS;
	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	memset(&server_addr, 0, addrlen);

	ip_conversion = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
	if (ip_conversion == 0) {
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
		return EXIT_FAILURE;
	}

	int port_number = 0;

	if (!parse_int(argv[2], &port_number, "port number")) {
		return EXIT_FAILURE;
	}

	if (port_number < 1024 || port_number > 65535) {
		fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
		return EXIT_FAILURE;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_number);

	printf("Username: ");
    fflush(stdout);
    int username_status;

    while ((username_status = get_string(username, MAX_NAME_LEN + 1)) != OK) {
		if (username_status == TOO_LONG) {
			printf("Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
		}
		printf("Username: ");
    }

    printf("Hello, %s. Let's try to connect to the server.\n", username);

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    	fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
    	return EXIT_FAILURE;
    	// retval = EXIT_FAILURE;
    	// goto EXIT;
	}

	if (connect(client_socket, (struct sockaddr *)&server_addr, addrlen) < 0) {
        fprintf(stderr, "Error: Failed to connect to server. %s.\n",
                strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN - 1, 0)) < 0) {
        fprintf(stderr, "Error: Failed to receive message from server. %s.\n",
                strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    if (bytes_recvd == 0) {
        fprintf(stderr, "All connections are busy. Try again later.\n");
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    inbuf[bytes_recvd] = '\0';
    printf("\n%s\n\n", inbuf);
    fflush(stdout);

    if (send(client_socket, username, strlen(username), 0) < 0) {
        fprintf(stderr, "Error: Failed to send username to server. %s.\n",
                strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }

    fd_set readIO;

    printf("[%s]: ", username);
    fflush(stdout);

    while (true) {
        FD_ZERO(&readIO);
    	FD_SET(client_socket, &readIO);
    	FD_SET(STDIN_FILENO, &readIO);

    	if ((select(client_socket + 1, &readIO, NULL, NULL, NULL) < 0) && errno != EINTR) {
    		fprintf(stderr, "Error: select() failed. %s.\n", strerror(errno));
    		retval = EXIT_FAILURE;
    		goto EXIT;
    	}

		int temp = 0;
    	if (FD_ISSET(client_socket, &readIO)) {
    		temp = handle_client_socket();
    	}
    	if (FD_ISSET(STDIN_FILENO, &readIO)) {
    		temp = handle_stdin();
    	}
    	if (temp == 1) {
    		retval = EXIT_SUCCESS;
    		goto EXIT;
    	}
    	if (temp == 2) {
    		retval = EXIT_FAILURE;
    		goto EXIT;
    	}

        printf("[%s]: ", username);
        fflush(stdout);
	}

EXIT:
    if (fcntl(client_socket, F_GETFD) >= 0) {
        close(client_socket);
    }
    return retval;
}
