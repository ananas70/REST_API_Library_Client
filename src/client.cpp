#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "helpers.h"
#include "requests.h"

#include "../nlohmann/json.hpp"

#define SERVER_PORT 8080
// #define SERVER_ADDR "34.246.184.49"
char SERVER_ADDR[] = "34.246.184.49";


char URL_REGISTER[] = "/api/v1/tema/auth/register";
char URL_LOGIN [] = "/api/v1/tema/auth/login";
char URL_ACCESS [] = "/api/v1/tema/library/access";
char URL_BOOKS [] ="/api/v1/tema/library/books";
char URL_LOGOUT[] = "/api/v1/tema/auth/logout";
char CONTENT_TYPE[] = "application/json";


using namespace std;
using json = nlohmann::json;

bool is_number(const string& str) {
    return !str.empty() && all_of(str.begin(), str.end(), ::isdigit);
}


int main() {
    setvbuf (stdout, NULL, _IONBF, 0);

    int server_sock; // socket-ul pe care vom comunica cu server-ul
    string user_cookies;
    string user_token;
    bool logged_in = false;

    string input;
    // Clientul e activ cât timp nu primește exit
    while(1) {
        server_sock = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
        
        getline(cin, input);

        if(input == "register") {
            if (!user_cookies.empty()) {
                // Clientul e deja logat
                cout << "EROARE: Clientul este deja conectat" << endl;
                continue;
            }

            string username, password;
            cout << "username=";
            getline(cin, username);
            cout << "password=";
            getline(cin, password);

            json user_data = {
                {"username", username},
                {"password", password}
            };
            string json_str = user_data.dump();

            char* body_data[1];
            body_data[0] = (char *)json_str.c_str();

            string message = compute_post_request(SERVER_ADDR, URL_REGISTER, CONTENT_TYPE, body_data, 1, NULL, 0);
            
            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // Verificăm ce mesaj am primit
            
            if(response.find("error") != string::npos)
                cout << "EROARE: username-ul este deja folosit de către cineva!" << endl;
            else 
                cout << "Utilizator înregistrat cu succes!" << endl;
        }

        else if (input == "login") {

            if (!user_cookies.empty()) {
                // Clientul e deja logat
                cout << "EROARE: Clientul este deja conectat" << endl;
                continue;
            }

            string username, password;
            cout << "username=";
            getline(cin, username);
            cout << "password=";
            getline(cin, password);

            json user_data = {
                {"username", username},
                {"password", password}
            };
            string json_str = user_data.dump();

            char* body_data[1];
            body_data[0] = (char *)json_str.c_str();

            string message = compute_post_request(SERVER_ADDR, URL_LOGIN, CONTENT_TYPE, body_data, 1, nullptr, 0);
            
            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // Verificăm ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "EROARE: nu se potrivesc credenţialele!" << endl;
                user_cookies.clear();
            }
            else {
                cout << "Utilizatorul a fost logat cu succes" << endl;

                string cookie_key = "Set-Cookie: ";
                size_t start_pos = response.find(cookie_key);

                start_pos += cookie_key.length();

                size_t end_pos = response.find("\r\n", start_pos);

                user_cookies.append(response.substr(start_pos, end_pos - start_pos));
                logged_in = true;
            }

        }
        
        else if (input == "enter_library") {
            if(!logged_in) {
                cout << "EROARE: Nu sunteți autentificat" << endl;
                continue;
            }

            char* cookies[1];
            cookies[0] = (char *) user_cookies.c_str();
            string message = compute_get_request(SERVER_ADDR, URL_ACCESS, NULL, cookies, 1);

            send_to_server(server_sock, (char *) message.c_str());

            string response = receive_from_server(server_sock);

            // Verificăm ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "EROARE: nu se potrivesc credenţialele!" << endl;
                user_cookies.clear();
            }
            else {
                cout << "Success: Utilizatorul are acces la biblioteca" << endl;
                // formăm token-ul JWT
                user_token.clear();
                user_token += "Authorization: Bearer ";
                string token_key = "token";
                size_t start_pos = response.find(token_key);
                start_pos += token_key.length() + 3; // +3 pentru a sări peste "token" și primele două caractere după el
                size_t end_pos = response.find("\"", start_pos);

                user_token = response.substr(start_pos, end_pos - start_pos);
            }
        }

        else if (input == "get_books") {
            if(user_token.empty()) {
                cout << "EROARE: Cerere repsinsă" << endl;
                continue;
            }

            char* cookies[1];
            cookies[0] = (char*) user_cookies.c_str();
            string message = compute_get_request_auth(SERVER_ADDR, URL_BOOKS, NULL, cookies, 1, (char*) user_token.c_str());

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // Verificăm ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "EROARE: cerere respinsă pentru get_books" << endl;
                user_cookies.clear();
            }

            else {
                cout << response <<endl;
            }
        }

        else if (input == "get_book") {
            if(user_token.empty()) {
                cout << "EROARE: Cerere respinsă" << endl;
                continue;
            }

            cout << "id=";
            string id;
            getline(cin, id);

            if(!is_number(id)) {
                cout << "EROARE: ID invalid" << endl;
                continue;
            }

            string url_given_book = URL_BOOKS;
            url_given_book = url_given_book + "/" + id;

            char* cookies[1];
            cookies[0] = (char*) user_cookies.c_str();
            string message = compute_get_request_auth(SERVER_ADDR,(char*) url_given_book.c_str(), NULL, cookies, 1, (char*) user_token.c_str());

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // Verificăm ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "EROARE: cerere respinsă pentru get_book" << endl;
                user_cookies.clear();
            }

            else {
                cout << response << endl;
            }
        }

        else if (input == "add_book") {
            if(user_token.empty()) {
                cout << "EROARE: Cerere respinsă" << endl;
                continue;
            }
            
            string title, author, genre, page_count, publisher;
            cout << "title=";      
            getline(cin, title);
            cout << "author=";     
            getline(cin, author);
            cout << "genre=";      
            getline(cin, genre);
            cout << "page_count="; 
            getline(cin, page_count);
            cout << "publisher=";  
            getline(cin, publisher);

            if(!is_number(page_count)) {
                cout << "EROARE: Tip de date incorect pentru numarul de pagini";
                continue;
            }

            char* body_data[1];
            char* cookies[1];
            json book_data = {
                {"title", title},
                {"author", author},
                {"genre", genre},
                {"page_count", page_count},
                {"publisher", publisher},
            };
            
            string json_str = book_data.dump();

            body_data[0] = (char *)json_str.c_str();

            cookies[0] = (char*) user_cookies.c_str();

            string message = compute_post_request_auth(SERVER_ADDR, URL_BOOKS, CONTENT_TYPE, body_data, 1, cookies, 1, (char*) user_token.c_str());

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // Verificăm ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "EROARE: add book" << endl;
                cout << response;
                user_cookies.clear();
            }

            else {
                cout << "Carte adăugata cu succes" << endl;
            }
        }

        else if (input == "delete_book") {
            if(user_token.empty()) {
                cout << "EROARE: Cerere respinsă" << endl;
                continue;
            }
            
            string id;
            cout << "id="; 
            getline(cin, id);
            
            if(!is_number(id)) {
                cout << "EROARE: ID invalid" << endl;
                continue;
            }

            string url_book = URL_BOOKS;
            url_book += "/" + id;

            char* cookies[1];
            cookies[0] = (char*) user_cookies.c_str();
            string message = compute_get_request_auth(SERVER_ADDR, URL_BOOKS, NULL, cookies, 1, (char*) user_token.c_str());


            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // Verificăm ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "EROARE: cerere respinsă pentru delete_book" << endl;
                user_cookies.clear();
            }

            else {
                cout << "Cartea cu id "<< std::stod(id) <<" a fost stearsa cu succes!" << endl;
            }
        }

        else if (input == "logout") {
            if(!logged_in) {
                cout << "EROARE: Nu sunteți logat" << endl;
                continue;
            }

            char* cookies[1];
            cookies[0] = (char *) user_cookies.c_str();

            string message = compute_get_request(SERVER_ADDR, URL_LOGOUT, NULL, cookies, 1);

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            if(response.find("error") != string::npos) {
                cout << "EROARE: cerere respinsă pentru logout" << endl;
                user_cookies.clear();
            }

            else {
                cout << "Utilizatorul s-a delogat cu succes!" << endl;
            }

            user_token.clear();

        }

        else if (input == "exit") {
            cout << "Inchidere program" << endl;
            close_connection(server_sock);
            break;
        }

        else {
            cout << "EROARE: comandă necunoscută" << endl;
        }
    }

    return 0;
}