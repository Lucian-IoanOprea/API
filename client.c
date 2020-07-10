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
#include "parson.h"

#define SERVER "3.8.116.10"
#define n_size 121
#define n_id 80

int main(int argc, char *argv[])
{
    int sockfd;
    char *cookie = NULL;
    char *token = NULL;
    while(1){
     char *buffer;
     buffer = calloc(n_size,sizeof(char));
     fgets(buffer,n_size,stdin);
     if(strncmp(buffer,"register",8) == 0){
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        char *myserial = NULL;
        char username[80];
        char password[80];
        printf("Username = ");
        scanf("%s",username);
        json_object_set_string(root_object, "username",username);
        printf("Password = ");
        scanf("%s",password);
        json_object_set_string(root_object, "password",password);
        myserial = json_serialize_to_string_pretty(root_value);
        sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Socket error!");
            exit(-1);
         }
         char *message;
         char *content_type ="application/json";
         message = compute_post_request(SERVER, "/api/v1/tema/auth/register", content_type, myserial, NULL, 0,NULL);
         printf("%s",message);
         send_to_server(sockfd,message);
         char *response;
         response = receive_from_server(sockfd);
        printf("%s\n", response);
        json_free_serialized_string(myserial);
        json_value_free(root_value);
        close(sockfd);
        free(message);
        free(response);
         memset(buffer,0,n_size);
         memset(username,0,80);
         memset(password,0,80);
     }
     if(strncmp(buffer,"login",5) == 0){
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        char *myserial = NULL;
        char username[80];
        char password[80];
        printf("Username = ");
        scanf("%s",username);
        json_object_set_string(root_object, "username",username);
        printf("Password = ");
        scanf("%s",password);
        json_object_set_string(root_object, "password",password);
        myserial = json_serialize_to_string_pretty(root_value);
        sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Socket error!");
            exit(-1);
         }
         char *message;
         char *content_type ="application/json";
         message = compute_post_request(SERVER, "/api/v1/tema/auth/login", content_type, myserial, NULL, 0,NULL);
         send_to_server(sockfd,message);
         char *response;
         response = receive_from_server(sockfd);
         printf("%s\n", response);
          char *error = basic_extract_json_response(response);
            if (error == NULL) {
                fprintf(stdout, "Utilizatorul %s s-a logat cu succes!\n", username);
                char *start = strstr(response, "Set-Cookie:");
                char *end = strchr(start, ';');
                cookie = (char *) calloc((end - start + 13), sizeof(char));
                memcpy(cookie, start + 12, end - (start + 12));
                fprintf(stdout, "Cookie: %s\n", cookie);
            } else {
                fprintf(stderr, "Credentiale gresite!\n");
            }
         json_free_serialized_string(myserial);
         json_value_free(root_value);
         close(sockfd);
         free(message);
         free(response);
         memset(buffer,0,n_size);
         memset(username,0,80);
         memset(password,0,80);
     }
     else if(strncmp(buffer,"enter_library",13) == 0){
          if(cookie == NULL){
             fprintf(stderr,"Eroare!Nu sunteti logat!");
             continue;
         }
        sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Socket error!");
            exit(-1);
         }
         char *message;
         char *cookies[1] = {cookie};
         message = compute_get_request(SERVER, "/api/v1/tema/library/access", NULL,cookies,1,NULL);
         send_to_server(sockfd,message);
         char *response;
         response = receive_from_server(sockfd);
        char *tokken = basic_extract_json_response(response);
        char *end = strchr(tokken + 10, '}');
        token = (char *)calloc(end - (tokken + 10) + 1, sizeof(char));
        memcpy(token, tokken + 10, end - 1 - (tokken + 10));
         printf("%s\n", response);
         close(sockfd);
        memset(buffer,0,sizeof(buffer));
         free(message);
         free(response);
     }
     else if(strncmp(buffer,"get_books",9) == 0){
         if(token == NULL || cookie == NULL){
             fprintf(stderr,"Eroare!Nu sunteti logat sau nu aveti acces la biblioteca!");
             continue;
         }
        sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Socket error!");
            exit(-1);
         }
         char *cookies[1] ={cookie};
         char *message;
         char *response;
         message = compute_get_request(SERVER, "/api/v1/tema/library/books", NULL, cookies, 1, token);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("%s\n",response);
        close(sockfd);
        memset(buffer,0,sizeof(buffer));
        free(response);
        free(message); 
     }
     else if(strncmp(buffer,"get_book",8) == 0){
          if(token == NULL || cookie == NULL){
             fprintf(stderr,"Eroare!Nu sunteti logat sau nu aveti acces la biblioteca!");
             continue;
         }
        sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
          if (sockfd < 0) {
            perror("Socket error!");
            exit(-1);
         }
         char *id;
         id = calloc(n_id,sizeof(char));
         printf("id = ");
         scanf("%s",id);
         char *cookies[1] = {cookie};
         char *message, *response;
         char address[140];
         sprintf(address, "/api/v1/tema/library/books/%d", atoi(id));
         message = compute_get_request(SERVER, address, NULL, cookies, 1, token);
         send_to_server(sockfd, message);
         response = receive_from_server(sockfd);
         printf("%s",response);
         close(sockfd);
         free(response);
         free(id);
         free(message);
         memset(buffer,0,sizeof(char));
         memset(address,0,140);
     }
     else if(strncmp(buffer,"add_book",8) == 0 ){
        if(token == NULL || cookie == NULL){
             fprintf(stderr,"Eroare!Nu sunteti logat sau nu aveti acces la biblioteca!");\
             continue;
         }
         sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
          if (sockfd < 0) {
            perror("Socket error!");
            exit(-1);
         }
        char title[80], author[80], genre[80], publisher[80];
        int page;
        printf("Titlu = "),scanf("%[^\n]s", title);
        printf("Author ="),scanf(" %[^\n]s", author);
        printf("Genre ="),scanf(" %[^\n]s", genre);
        printf("Publisher ="),scanf(" %[^\n]s", publisher);
        printf("Page_count ="), scanf("%d",&page);
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        char *serialized_string = NULL;
        char *message;
        char *response;
        json_object_set_string(root_object, "title", title);
        json_object_set_string(root_object, "author", author);
        json_object_set_string(root_object, "genre", genre);
        json_object_set_string(root_object, "publisher", publisher);
        json_object_set_number(root_object, "page_count", page);
        serialized_string = json_serialize_to_string_pretty(root_value);
            char *content_type = "application/json";
            char *cookies[1] = {cookie};
            message = compute_post_request(SERVER, "/api/v1/tema/library/books", content_type, serialized_string, cookies, 1, token);
            send_to_server(sockfd, message);
            json_value_free(root_value);
            json_free_serialized_string(serialized_string);
            response = receive_from_server(sockfd);
            printf("%s\n", response);
            close_connection(sockfd);
            free(message);
            free(response);
            memset(buffer,0,sizeof(buffer));
     }
     else if(strncmp(buffer,"delete_book",11) == 0){
          if(token == NULL || cookie == NULL){
             fprintf(stderr,"Eroare!Nu sunteti logat sau nu aveti acces la biblioteca!");
         }
         int id;
         printf("Id = ");
         scanf("%d",&id);
          sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
          if (sockfd < 0) {
            perror("Socket error!");
            exit(-1);
         }
        char *cookies[1] = {cookie};
        char address[140];
        char *message, *response;
        sprintf(address, "/api/v1/tema/library/books/%d", id);
        message = compute_delete_request(SERVER, address, NULL, cookies, 1,token);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        printf("%s\n", response);
        close_connection(sockfd);
        free(message);
        free(response);
        memset(buffer,0,sizeof(buffer));
        memset(address,0,140);
        
     }
     else if(strncmp(buffer,"logout",6)== 0){
           sockfd = open_connection(SERVER, 8080, AF_INET, SOCK_STREAM, 0);
            if (cookie == NULL) {
                fprintf(stderr, "Eroare! Nu sunteti logat!\n");
                free(buffer);
                continue;
            }
            char *message, *response;
            char *cookies[1] = {cookie};
            message = compute_get_request(SERVER, "/api/v1/tema/auth/logout", NULL, cookies, 1, token);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            printf("%s",response);
            close_connection(sockfd);
            if (cookie != NULL) {
                free(cookie);
                cookie = NULL;
            }
            memset(buffer,0,sizeof(buffer));
             if (token != NULL) {
                free(token);
                token = NULL;
            }
     }
     else if(strncmp(buffer,"exit",4) == 0){
        return 0;
     }
    }   
}