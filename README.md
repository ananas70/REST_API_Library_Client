# Tema 4 - Client web. Comunicaţie cu REST API.
### Autoare: Stoica Ana-Florina, grupa 325 CB

## **Descriere** 
Acest proiect reprezintă o aplicație simplă de gestionare a unei biblioteci, cu funcționalități de înregistrare, autentificare, acces la bibliotecă, adăugare și ștergere de cărți. Scopul temei este de a scrie un client HTTP în C/C++ care să interacţioneze cu un REST API.

Pentru a rezolva tema, am implementat un client funcțional în C++ care acceptă comenzi de la tastatură (stdin) şi trimite, în funcţie de comandă, cereri către server. Scopul lui este de a funcţiona ca o interfaţă în linia de comandă (CLI) cu biblioteca virtuală.

## **Implementare**
Codul este scris în limbajul C++ și utilizează biblioteca standard și alte biblioteci pentru gestionarea conexiunilor de rețea și a cererilor HTTP. Am corelat fișierul sursă cu bibliotecile `helpers.h`, `buffers.h`, `requests.h` și `nlohmann/json.hpp` (pentru parsarea datelor JSON). 

Am optat pentru biblioteca `nlohmann` deoarece este compatibilă cu C++ și ușor de folosit. Astfel, am preferat să manipulez JSON într-un mod simplu, întrucât transformarea datelor în format JSON și invers se face într-un mod simplu.

Pentru început, am folosit funcțiile implementate în cadrul laboratorului 09 (`helpers.c`, `requests.c`, `buffers.c`), iar în `requests.h` am adăugat încă 3 funcții:
> * 2 variații ale funcțiilor `compute_post_request` și `compute_get_request`, și anume `compute_post_request_auth` și `compute_get_request_auth` care includ și logica necesară parsării token-ului JWT. 
> * Funcția `compute_delete_request_auth` care oferă funcționalitatea necesară pentru ștergerea unei cărți din bibliotecă.

Am implementat următoarele comenzi:

- **REGISTER**: Se verifică faptul ca utilizatorul nu este deja conectat cu credențialele date și se trimit datele către server. Se parsează răspunsul primit de la server.

- **LOGIN**: Se verifică faptul ca utilizatorul nu este deja conectat. După aceea, se formează un obiect JSON cu datele utilizatorului, se adaugă în conținutul trimis către server (`body_data`) și se trimite conținutul către server. Se parsează răspunsul primit de la server și se salvează cookie-urile aferente utilizatorului.

- **ENTER_LIBRARY**: După verificarea autentificării, se poate accesa biblioteca. Se formează și se salvează, în același timp, token-ul JWT al utilizatorului sesiunii curente. Acest token va fi folosit ulterior, când va trebui să dovedim că utilizatorul are acces la bibliotecă.

- **GET_BOOKS**: Se verifică accesul la bibliotecă și se accesează biblioteca, urmând a se afișa toate cărțile disponibile în format JSON.

- **GET_BOOK**: Pentru a vizualiza detalii despre o anumită carte, se parsează de la tastatură id-ul dorit, se formează ruta de acces către carte și se trimite cererea potrivită către server. Se afișează răspunsul primit de la server în format JSON.

- **ADD_BOOK**: Se verifică datele introduse de utilizator (să nu fi introdus câmpuri vide sau un Page Count invalid) și formează câmpul JSON corespunzător noii cărți. Se trimite un POST Request către server și, în cazul adăugării cu succes, se afișează un mesaj de succes.

- **DELETE_BOOK**: Se verifică accesul la bibliotecă și integritatea id-ului cărții vizate și se trimite un GET Request către server. Se afișează un mesaj corespunzător în caz de succes.

- **LOGOUT**: curpinde logica de deconectare a utilizatorului din sesiunea curentă. Se verifică faptul că utilizatorul este conectat și se trimite către server un GET Request. Se golesc câmpurile `user_cookies` și `user_token` ce conțin cookie-urile și token-ul JWT ale utilizatorului și se setează flag-ul `logged_in` ca fals.

- **EXIT**: Se închide conexiunea cu server-ul (prin închiderea socket-ului `server_sock`) și se iese din bucla infinită.