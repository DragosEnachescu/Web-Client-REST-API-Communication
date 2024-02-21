#include "parson.h"
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#define COOKIE_IN_RESPONSE 277 /* in response, incepand cu pozitia 277 trebuie sa luam cookie-ul pt cand ne logam */
#define TOKEN_COOKIE_LENGHT 1000

/* functie care returneaza un char* pt a forma obiectul json pt datele de logare si autentificare */
char *serialization_register_login(char username[], char password[]) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    char *serialized_string = NULL;
    
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    serialized_string = json_serialize_to_string_pretty(root_value);

    return serialized_string;
}

/* functie care returneaza obiectul json cu datele despre cartea de la comanda add_book */
char *serialization_add_book(char *title, char *author, char *genre, char *page_count, char *publisher) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    char *serialized_string = NULL;

    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "page_count", page_count);
    json_object_set_string(root_object, "publisher", publisher);

    serialized_string = json_serialize_to_string_pretty(root_value);

    return serialized_string;
}

char *serialization_get_book(int id, const char *title, const char *author, const char *publisher, const char *genre, int page_count) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    char *serialized_string = NULL;

    json_object_set_number(root_object, "id", id);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_number(root_object, "page_count", page_count);

    serialized_string = json_serialize_to_string_pretty(root_value);

    return serialized_string;
}

char *receive_response_from_server(char command[], int sockfd, char *url, char **body_data, char **cookie_table, char **token_table) {
    char *response, *message;

    if (strncmp(command, "register", 8) == 0) {
        message = compute_post_request("34.254.242.81", url, "application/json", body_data, 1, NULL, 1, NULL, 0);

    } else if (strncmp(command, "login", 5) == 0) {
        message = compute_post_request("34.254.242.81", url, "application/json", body_data, 1, NULL, 1, NULL, 0);

    } else if (strncmp(command, "enter_library", 13) == 0) {
        message = compute_get_delete_request("get", "34.254.242.81", url, NULL, cookie_table, 1, NULL, 0);

    } else if (strncmp(command, "get_books", 9) == 0) {
        message = compute_get_delete_request("get", "34.254.242.81", url, NULL, NULL, 0, token_table, 1);

    } else if(strncmp(command, "get_book", 8) == 0) {
        message = compute_get_delete_request("get", "34.254.242.81", url, NULL, NULL, 0, token_table, 1);

    } else if (strncmp(command, "add_book", 8) == 0) {
        message = compute_post_request("34.254.242.81", url, "application/json", body_data, 1, NULL, 1, token_table, 1);

    } else if (strncmp(command, "delete_book", 11) == 0) {
        message = compute_get_delete_request("delete", "34.254.242.81", url, NULL, NULL, 0, token_table, 1);

    } else if (strncmp(command, "logout", 6) == 0) {
        message = compute_get_delete_request("get", "34.254.242.81", "/api/v1/tema/auth/logout", NULL, cookie_table, 1, NULL, 0);

    }

    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);

    return response;
}

int main(int argc, char *argv[]) {
    char *response;
    int sockfd;

    char *cookie = ""; /* se va retine cookie-ul pt cand ne dam comanda de login */
    char *token = ""; /* se va retine token-ul de la comanda enter_library */

    char **cookie_table = malloc(sizeof(char *));

    char **token_table = malloc(sizeof(char *));
    token_table[0] = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));
    strcpy(token_table[0], "");

    char *books = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));

    while (1) {
        sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);

        char command[20] = {0};
        scanf("%s", command);

        if (strncmp(command, "register", 8) == 0) {
            char username[20]; 
            char password[20];
            
            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            char **body_data = calloc(1, sizeof(char *));
            body_data[0] = calloc(1000, sizeof(char));

            body_data[0] = serialization_register_login(username, password);

            response = receive_response_from_server(command, sockfd, "/api/v1/tema/auth/register", body_data, NULL, NULL);

            if(strncmp(response, "HTTP/1.1 400", 12) == 0) {
                printf("The username %s is taken!\n", username);
            } else if (strncmp(response, "HTTP/1.1 201", 12) == 0) {
                printf("200 - OK - Utilizator înregistrat cu succes!\n");
            } else {
                printf("Error\n");
            }

        } else if (strncmp(command, "login", 5) == 0) {
            char username[20];
            char password[20];

            printf("username=");
            scanf("%s", username);

            printf("password=");
            scanf("%s", password);

            char **body_data = calloc(1, sizeof(char *));
            body_data[0] = serialization_register_login(username, password);

            response = receive_response_from_server(command, sockfd, "/api/v1/tema/auth/login", body_data, NULL, NULL);

            if (strncmp(response, "HTTP/1.1 200", 12) == 0) {
                printf("200 - OK - Bun venit!\n");

                char *set_cookie = response + COOKIE_IN_RESPONSE;
                cookie = strtok(set_cookie, ";");

            } else if (strncmp(response, "HTTP/1.1 400", 12) == 0) {
                printf("Credentials are not good!\n");
            } else if (strcmp(cookie, "") != 0) {
                printf("You are already logged in, please continue on this id, or logout and connect with another account");
            } else {
                printf("Error\n");
            }

        } else if (strncmp(command, "enter_library", 13) == 0) {
            cookie_table[0] = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));

            strcpy(cookie_table[0], cookie);

            response = receive_response_from_server(command, sockfd, "/api/v1/tema/library/access", NULL, cookie_table, NULL);

            if (strncmp(response, "HTTP/1.1 401", 12) == 0) {
                printf("You are not logged in!\n");
            } else if (strncmp(response, "HTTP/1.1 200", 12) == 0) {
                printf("OK\n");
                /* trebuie extras din raspunsul serverului token-ul */
                char *token_copy = response + 472;
                token = strtok(token_copy, "}");
                token[strlen(token) - 1] = '\0'; /* token-ul JWT */

            } else {
                printf("Error\n");
            }

        } else if (strncmp(command, "get_books", 9) == 0) {
            token_table[0] = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));

            strcpy(token_table[0], token);

            response = receive_response_from_server(command, sockfd, "/api/v1/tema/library/books", NULL, NULL, token_table);

            if (strncmp(response, "HTTP/1.1 500", 12) == 0) {
                printf("Error when decoding tokenn!\n");
            } else if (strncmp(response, "HTTP/1.1 200", 12) == 0) {
                if (strlen(response) == 460) {
                    /* in cazul in care nu au fost adaugate carti */
                    printf("No books to display!\n");

                } else {
                    printf("OK\n");
                    strcpy(books, response + 460);
                
                    JSON_Value *root_value = json_parse_string(books);

                    char *serialized_string = json_serialize_to_string_pretty(root_value);
                    printf("%s\n", serialized_string);

                }
            } else {
                printf("Error\n");
            }

        } else if(strncmp(command, "get_book", 8) == 0) {
            token_table[0] = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));

            strcpy(token_table[0], token);

            char *url = malloc(100 * sizeof(char));
            strcpy(url, "/api/v1/tema/library/books/");
            
            int id;
            printf("id=");
            scanf("%d", &id);

            char *id_copy = malloc(100 * sizeof(char));
            sprintf(id_copy, "%d", id); /* conversia numarului din int in char */

            strcat(url, id_copy);

            response = receive_response_from_server(command, sockfd, url, NULL, NULL, token_table);

            if (strncmp(response, "HTTP/1.1 404", 12) == 0) {
                printf("No book was found!\n");
            } else if (strncmp(response, "HTTP/1.1 200", 12) == 0) {
                strcpy(books, response + 460);

                JSON_Value* book_info = json_parse_string(books);

                JSON_Object* object = json_value_get_object(book_info);

                const char* title = json_object_get_string(object, "title");
                const char* author = json_object_get_string(object, "author");
                const char* publisher = json_object_get_string(object, "publisher");
                const char* genre = json_object_get_string(object, "genre");
                int page_count = json_object_get_number(object, "page_count");

                char *serialized_string = serialization_get_book(id, title, author, publisher, genre, page_count);

                JSON_Value *root_value = json_parse_string(serialized_string);

                char *output = json_serialize_to_string_pretty(root_value);
                printf("%s\n", output);

            } else if(strncmp(response, "HTTP/1.1 500", 12) == 0) {
                printf("Error when decoding tokenn!\n");
            } else {
                printf("Error\n");
            }

        } else if (strncmp(command, "add_book", 8) == 0) {
            char *author, *title, *genre, *publisher, *page_count;

            title = malloc(100 * sizeof(char));

            printf("title=");
            fgets(title, 100, stdin);
            fgets(title, 100, stdin);
            title[strlen(title) - 1] = '\0';

            author = malloc(100 * sizeof(char));

            printf("author=");
            fgets(author, 100, stdin);
            author[strlen(author) - 1] = '\0';

            genre = malloc(100 * sizeof(char));

            printf("genre=");
            fgets(genre, 100, stdin);
            genre[strlen(genre) - 1] = '\0';

            publisher = malloc(100 * sizeof(char));

            printf("publisher=");
            fgets(publisher, 100, stdin);
            publisher[strlen(publisher) - 1] = '\0';

            page_count = malloc(100 * sizeof(char));

            printf("page_count=");
            fgets(page_count, 100, stdin);
            page_count[strlen(page_count) - 1] = '\0';

            int OK = 0;
            if (strcmp(author, "") == 0 || strcmp(title, "") == 0 || strcmp(genre, "") == 0 || strcmp(publisher, "") == 0) {
                printf("You forgot to fill in some data about the book.\n");
                OK++;
            }

            int nr = 0;
            for (int i = 0; i < strlen(page_count); i++) {
                if (strchr("0123456789", page_count[i])) nr++;
            }

            if (nr != strlen(page_count)) {
                printf("The data about number of pages is not valid, please type only numeric characters!\n");
                OK++;
            }

            if (OK == 0) {
                char **body_data = calloc(1, sizeof(char *));
                body_data[0] = calloc(TOKEN_COOKIE_LENGHT, sizeof(char));

                body_data[0] = serialization_add_book(title, author, genre, page_count, publisher);

                token_table[0] = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));

                strcpy(token_table[0], token);

                response = receive_response_from_server(command, sockfd, "/api/v1/tema/library/books", body_data, NULL, token_table);

                if (strncmp(response, "HTTP/1.1 200", 12) == 0) {
                    printf("OK\n");
                } else {
                    printf("Error\n");
                }
            }

        } else if (strncmp(command, "delete_book", 11) == 0) {
            token_table[0] = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));

            strcpy(token_table[0], token);

            char *url = malloc(100 * sizeof(char));
            strcpy(url, "/api/v1/tema/library/books/");
            
            int id;
            printf("id=");
            scanf("%d", &id);

            char *id_copy = malloc(100 * sizeof(char));
            sprintf(id_copy, "%d", id); /* conversia numarului din int in char */

            strcat(url, id_copy);

            response = receive_response_from_server(command, sockfd, url, NULL, NULL, token_table);

            if (strncmp(response, "HTTP/1.1 500", 12) == 0) {
                printf("Error when decoding tokenn!\n");
            } else if (strncmp(response, "HTTP/1.1 200", 12) == 0) {
                printf("OK\n");
            } else {
                printf("Error\n");
            }

        } else if (strncmp(command, "logout", 6) == 0) {
            cookie_table[0] = malloc(TOKEN_COOKIE_LENGHT * sizeof(char));

            strcpy(cookie_table[0], cookie);

            response = receive_response_from_server(command, sockfd, "/api/v1/tema/auth/logout", NULL, cookie_table, NULL);

            if (strncmp(response, "HTTP/1.1 400", 12) == 0) {
                printf("You are not logged in!\n");
            } else if (strncmp(response, "HTTP/1.1 200", 12) == 0) {
                printf("OK\n");
                strcpy(cookie, "-1");
                
                /* aici este cazul cand imediat dupa comanda de login se da comanda de logout */
                if (strcmp(token, "") != 0) strcpy(token, "-1");
            } else {
                printf("Error\n");
            }

        } else if (strncmp(command, "exit", 4) == 0) {
            break;
        } else {
            printf("Invalid command. Please try another one!\n");
        }
    }

    close_connection(sockfd);
    return 0;
}