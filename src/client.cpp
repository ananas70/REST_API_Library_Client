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
#define SERVER_ADDR "34.246.184.49"

#define URL_REGISTER "/api/v1/tema/auth/register"
#define URL_LOGIN "/api/v1/tema/auth/login"
#define URL_ACCESS "/api/v1/tema/library/access"
#define URL_BOOKS "/api/v1/tema/library/books"
#define URL_LOGOUT "/api/v1/tema/auth/logout"
#define CONTENT_TYPE "application/json"


using namespace std;
using json = nlohmann::json;


int main() {
    setvbuf (stdout, NULL, _IONBF, 0);

    int server_sock; // socket-ul pe care vom comunica cu server-ul
    string user_cookies; // vid la inceput
    string user_token;
    bool logged_in = false;

    string input;
    // clientul e activ cat timp nu primeste exit
    while(1) {
        server_sock = open_connection(SERVER_ADDR, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
        
        getline(cin, input);    // sau cin >> input; cin.ignore()

        if(input == "register") {
            // verificam si daca e deja logat (cookie == NULL?)
            if (!user_cookies.empty()) {
                //clientul e deja logat
                cout << "Client deja conectat" << endl;
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

            cout << json_str << endl;

            char* body_data[1];
            body_data[0] = (char *)json_str.c_str();


            string message = compute_post_request(SERVER_ADDR, URL_REGISTER, CONTENT_TYPE, body_data, 1, NULL, 0);
            
            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // verificam ce mesaj am primit
            
            // Întoarce eroare dacă username-ul este deja folosit de către cineva!

            if(response.find("error") != string::npos)
                cout << "Error: username-ul este deja folosit de către cineva!" << endl;
            else 
                cout << "Utilizator înregistrat cu succes!" << endl;
        }

        else if (input == "login") {

            if (!user_cookies.empty()) {
                //clientul e deja logat
                cout << "Client deja conectat" << endl;
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

            // verificam ce mesaj am primit
            /* 
                Răspuns: Întoarce cookie de sesiune.
                Erori tratate: Întoarce un mesaj de eroare dacă nu se potrivesc credenţialele!
            */
            

            if(response.find("error") != string::npos) {
                cout << "Error: nu se potrivesc credenţialele!" << endl;
                user_cookies.clear();
            }
            else {
                cout << "Utilizatorul a fost logat cu succes" << endl;

                string cookie_key = "Set-Cookie: ";
                size_t start_pos = response.find(cookie_key);

                // Mută pointerul la începutul valorii cookie-ului
                start_pos += cookie_key.length();

                // Găsește sfârșitul liniei (marcată de "\r\n")
                size_t end_pos = response.find("\r\n", start_pos);

                user_cookies = response.substr(start_pos, end_pos - start_pos);
                logged_in = true;
            }

        }
        
        else if (input == "enter_library") {
            if(!logged_in) {
                cout << "Nu sunteti autentificat" << endl;
                continue;
            }

            char* cookies[1];
            cookies[0] = (char *) user_cookies.c_str();
            string message = compute_get_request(SERVER_ADDR, URL_ACCESS, NULL, cookies, 1);

            send_to_server(server_sock, (char *) message.c_str());

            string response = receive_from_server(server_sock);

            // verificam ce mesaj am primit
            //Erori tratate: Întoarce un mesaj de eroare dacă nu se potrivesc credenţialele! 

            if(response.find("error") != string::npos) {
                cout << "Error: nu se potrivesc credenţialele!" << endl;
                user_cookies.clear();
            }
            else {
                cout << "Utilizatorul are acces la biblioteca" << endl;
                // formam token-ul JWT
                user_token.clear();
                user_token += "Authorization: Bearer ";
                string token_key = "token";
                size_t start_pos = response.find(token_key);
                start_pos += token_key.length() + 3; // +3 pentru a sări peste "token" și primele două caractere după el, de obicei `\":\"`
                size_t end_pos = response.find("\"", start_pos);

                user_token = response.substr(start_pos, end_pos - start_pos);
            }
        }

        else if (input == "get_books") {
            if(user_token.empty()) {    // sau il compari cu * ?
                cout << "Permission denied";
                continue;
            }

            char* cookies[1];
            cookies[0] = (char*) user_cookies.c_str();
            string message = compute_get_request_auth(SERVER_ADDR, URL_BOOKS, NULL, cookies, 1, (char*) user_token.c_str());

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // verificam ce mesaj am primit
            //Erori tratate: Întoarce un mesaj de eroare dacă nu se potrivesc credenţialele! 

            if(response.find("error") != string::npos) {
                cout << "Error: nu se potrivesc credenţialele!" << endl;
                user_cookies.clear();
            }

            else {
                
                if (response.find("[{") != string::npos && response.find("}]") != string::npos) {
                    unsigned first = response.find("[{");
                    unsigned last = response.find("}]");
                    string to_print = response.substr(first, last - first + 2);

                    json array_json = json::parse(to_print);

                    //* se afiseaza intr-un mod mai lizibil
                    for (int i = 0; i < static_cast<int>(array_json.size()); ++i) {
                    cout << "id: " << array_json[i]["id"] << "," << endl;
                    cout << "title: " << array_json[i]["title"] << endl;
                    }

                    // cout << response <<endl;

                } else {
                    cout << "Error: There are no books!" << endl;
                }
            }
        }

        else if (input == "get_book") {
            if(user_token.empty()) {
                cout << "Permission denied" << endl;
                continue;
            }

            cout << "id=";
            string id;
            getline(cin, id);
            // cout << "You're searching for " <<id<<endl;
            if(!all_of(id.begin(), id.end(), ::isdigit)) {
                cout << "Invalid book id" << endl;
                continue;
            }

            string url_given_book = URL_BOOKS;
            url_given_book = url_given_book + "/" + id;

            char* cookies[1];
            cookies[0] = (char*) user_cookies.c_str();
            string message = compute_get_request_auth(SERVER_ADDR,(char*) url_given_book.c_str(), NULL, cookies, 1, (char*) user_token.c_str());

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // verificam ce mesaj am primit
            //Erori tratate: Întoarce un mesaj de eroare dacă nu se potrivesc credenţialele! 

            if(response.find("error") != string::npos) {
                cout << "Error: get book" << endl;
                //cout << response << endl;
                user_cookies.clear();
            }

            else {
                //* daca nu exita setul de paranteze inseamna ca
                //* nu exista carti adaugate
                if (response.find("[{") != string::npos && response.find("}]") != string::npos) {
                    unsigned first = response.find("[{");
                    unsigned last = response.find("}]");

                    string to_print = response.substr(first, last - first + 2);

                    json array_json = json::parse(to_print);

                    //* se afiseaza informatiile cartii intr-un mod mai lizibil
                    for (int i = 0; i < static_cast<int>(array_json.size()); ++i) {
                    cout << "id: " << id << endl;
                    cout << "title: " << array_json[i]["title"] << endl;
                    cout << "author: " << array_json[i]["author"] << "," << endl;
                    cout << "publisher: " << array_json[i]["publisher"] << ","
                        << endl;
                    cout << "genre: " << array_json[i]["genre"] << "," << endl;
                    cout << "page_count: " << array_json[i]["page_count"] << endl;
                    }

                    // cout << response << endl;
                } else {
                cout << "There are no books!" << endl;
                }
            }
        }

        else if (input == "add_book") {
            if(user_token.empty()) {
                cout << "permission denied" << endl;
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

            //TODO : verifica le pe toate ca nu au spatii, sunt numere etc.

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

            // verificam ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "Error: add book" << endl;
                cout << response;
                user_cookies.clear();
            }

            else {
                cout << "Carte adaugata cu succes" << endl;
            }
        }

        else if (input == "delete_book") {
            if(user_token.empty()) {
                cout << "permission denied" << endl;
                continue;
            }
            
            string id;
            cout << "id="; 
            getline(cin, id);
            //verifica ca e numar

            string url_book = URL_BOOKS;
            url_book += "/" + id;

            char* tokens[1];
            tokens[0] = (char *)user_token.c_str();
            string message = compute_get_request(SERVER_ADDR, (char *) url_book.c_str(), NULL, tokens, 1);

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            // verificam ce mesaj am primit

            if(response.find("error") != string::npos) {
                cout << "Error: delete book" << endl;
                user_cookies.clear();
            }

            else {
                cout << "Carte stearsa" << endl;
            }
        }

        else if (input == "logout") {
            if(!logged_in) {
                cout << "Nici macar nu esti logat, fraiere" << endl;
                continue;
            }

            char* cookies[1];
            cookies[0] = (char *) user_cookies.c_str();

            string message = compute_get_request(SERVER_ADDR, URL_LOGOUT, NULL, cookies, 1);

            send_to_server(server_sock, (char *)message.c_str());

            string response = receive_from_server(server_sock);

            if(response.find("error") != string::npos) {
                cout << "Error: logout" << endl;
                user_cookies.clear();
            }

            else {
                cout << "Te ai deconectat" << endl;
            }

            user_token.clear();

        }

        else if (input == "exit") {
            close_connection(server_sock);
            break;
        }

        else {
            cout << "Unknown command" << endl;
        }
    }

}