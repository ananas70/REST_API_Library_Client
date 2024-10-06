# Web Client. Communication with REST API

## **Description**
This project represents a simple library management application, featuring functionalities for registration, authentication, library access, and adding and deleting books. The goal of the assignment is to write an HTTP client in C/C++ that interacts with a REST API.

To accomplish this assignment, I implemented a functional client in C++ that accepts commands from the keyboard (stdin) and sends requests to the server based on the commands. Its purpose is to function as a command-line interface (CLI) for the virtual library.

## **Implementation**
The code is written in C++ and utilizes the standard library along with other libraries for managing network connections and HTTP requests. I correlated the source file with the libraries `helpers.h`, `buffers.h`, `requests.h`, and `nlohmann/json.hpp` (for parsing JSON data).

I opted for the `nlohmann` library as it is compatible with C++ and easy to use. This allows me to manipulate JSON in a straightforward manner, as transforming data to and from JSON format is simple.

Initially, I used the functions implemented in lab 09 (`helpers.c`, `requests.c`, `buffers.c`), and in `requests.h`, I added three additional functions:
> * Two variations of the functions `compute_post_request` and `compute_get_request`, namely `compute_post_request_auth` and `compute_get_request_auth`, which include the logic needed for parsing the JWT token.
> * The function `compute_delete_request_auth`, which provides the necessary functionality for deleting a book from the library.

I implemented the following commands:

- **REGISTER**: It checks if the user is already logged in with the provided credentials and sends the data to the server. The response from the server is parsed.

- **LOGIN**: It checks if the user is already logged in. Then, a JSON object is formed with the user's data, added to the content sent to the server (`body_data`), and the content is sent to the server. The response from the server is parsed, and the corresponding cookies are saved for the user.

- **ENTER_LIBRARY**: After authentication verification, the library can be accessed. The JWT token of the current session user is formed and saved at the same time. This token will be used later when proving that the user has access to the library.

- **GET_BOOKS**: It verifies access to the library and accesses the library, displaying all available books in JSON format.

- **GET_BOOK**: To view details about a specific book, the desired ID is parsed from the keyboard, the access route to the book is formed, and the appropriate request is sent to the server. The response from the server is displayed in JSON format.

- **ADD_BOOK**: It checks the data entered by the user (to ensure no empty fields or invalid Page Count were entered) and forms the corresponding JSON field for the new book. A POST request is sent to the server, and in the case of a successful addition, a success message is displayed.

- **DELETE_BOOK**: It checks access to the library and the integrity of the targeted book ID and sends a GET request to the server. A corresponding message is displayed in case of success.

- **LOGOUT**: This includes the logic for logging the user out of the current session. It checks if the user is logged in and sends a GET request to the server. The `user_cookies` and `user_token` fields, which contain the cookies and JWT token of the user, are cleared, and the `logged_in` flag is set to false.

- **EXIT**: It closes the connection to the server (by closing the `server_sock` socket) and exits the infinite loop.
