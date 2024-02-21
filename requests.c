#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_delete_request(char *type, char *host, char *url, char *query_params,
                          char **cookies, int cookies_count,
                          char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (strncmp(type, "get", 3) == 0) {
        if (query_params != NULL) {
            sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
        } else {
            sprintf(line, "GET %s HTTP/1.1", url);
        }
    }

    else if (strncmp(type, "delete", 6) == 0) {
        if (query_params != NULL) {
            sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
        } else {
            sprintf(line, "DELETE %s HTTP/1.1", url);
        }
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        strcat(message, "Cookie: ");

        for (int i = 0; i < cookies_count; i++) {
            strcat(message, cookies[i]);

            if (i != cookies_count - 1) {
                strcat(message, "; ");
            }
        }

        strcat(message, "\r\n");
    }

    if (tokens != NULL) {
        strcat(message, "Authorization: Bearer ");

        for (int i = 0; i < tokens_count; i++) {
            strcat(message, tokens[i]);

            if (i != tokens_count - 1) {
                strcat(message, ";");
            }
        }

        strcat(message, "\r\n");
    }

    // Step 4: add final new line
    strcat(message, "\r\n");
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type,
                           char **body_data, int body_data_fields_count,
                           char **cookies, int cookies_count,
                           char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);


    int body_data_size = 0;
    for (int i = 0; i < body_data_fields_count; i++) {
        body_data_size += strlen(body_data[i]);
        strcat(body_data_buffer, body_data[i]);
        
        if (i < body_data_fields_count - 1) {
            strcat(body_data_buffer, "&");
            body_data_size += 1;
        }
    }

    sprintf(line, "Content-Length: %d", body_data_size);
    compute_message(message, line);

    if (cookies != NULL) {
        strcat(message, "Cookie: ");

        for (int i = 0; i < cookies_count; i++) {
            strcat(message, cookies[i]);

            if (i != cookies_count - 1) {
                strcat(message, "; ");
            }
        }

        strcat(message, "\r\n");
    }

    if (tokens != NULL) {
        strcat(message, "Authorization: Bearer ");

        for (int i = 0; i < tokens_count; i++) {
            strcat(message, tokens[i]);

            if (i != tokens_count - 1) {
                strcat(message, ";");
            }
        }

        strcat(message, "\r\n");
    }

    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}
