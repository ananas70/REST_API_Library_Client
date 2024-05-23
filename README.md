# Tema 4 - Tema 4. Client web. Comunicaţie cu REST API.
### Stoica Ana-Florina, grupa 325 CB

## Descriere 
Acest proiect reprezintă o aplicație simplă de gestionare a unei biblioteci, cu funcționalități de înregistrare, autentificare, acces la bibliotecă, adăugare și ștergere de cărți.

## Implementare
Codul este scris în limbajul C++ și utilizează biblioteca standard și alte biblioteci pentru gestionarea conexiunilor de rețea și a cererilor HTTP. Am corelat fișierul sursă cu bibliotecile `helpers.h`, `requests.h` și `nlohmann/json.hpp` (pentru parsarea datelor JSON). Am ales să optăm pentru biblioteca `nlohmann` deoarece are o interfață simplă și face lucrul cu JSON în C++ ușor și accesibil.

Pentru început, am folosit bibliotecile implementate în cadrul laboratorului 09 (`helpers.h`, `requests.h`, `buffers.h`), iar în `requests.h` am adăugat 2 variații ale funcțiilor `compute_post_request` și `compute_get_request`, și anume `compute_post_request_auth` și `compute_get_request_auth` care includ și logica necesară parsării token-ului JWT.


- REGISTER: Se verifică faptul ca utilizatorul nu este deja conectat cu credențialele date și se trimit datele către server. Se parsează răspunsul primit de la server.

- LOGIN: Se verifică faptul ca utilizatorul nu este deja conectat cu credențialele date și se trimit datele către server. Se parsează răspunsul primit de la server și se salvează cookie-urile aferente utilizatorului.

- ENTER_LIBRARY: După verificarea autentificării, se poate accesa biblioteca. Se formează și se salvează, în același timp, token-ul JWT specific utilizatorului sesiunii curente.

- GET_BOOKS: Se verifică logarea anterioară a utilizatorului și se accesează biblioteca, urmând a se afișa toate cărțile disponibile în format JSON.

- GET_BOOK: Pentru a vizualiza detalii despre o anumită carte, se parsează de la tastatură id-ul dorit și se trimite cererea potrivită către server.

- ADD_BOOK: Se formează câmpul JSON aferent noii cărți și se trimite un POST Request către server.

- DELETE_BOOK: Se verifică id-ul cărții vizate și se trimite un GET Request către server.

- LOGOUT: curpinde logica de delogare a utilizatorului din sesiunea curentă.

- EXIT: Pentru a închide aplicația, este introdusă comanda exit. Se închide conexiunea cu server-ul și programul principal.