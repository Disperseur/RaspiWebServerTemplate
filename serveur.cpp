#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <netinet/in.h>
#include <unistd.h>

// Variables simulant les capteurs
int sensor_value = 42;
bool led_state = false;

// Fonction pour lire un fichier (HTML ou CSS)
std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Erreur : Impossible d'ouvrir " << filename << std::endl;
        return "<h1>Erreur 404 : Fichier introuvable</h1>";
    }
    
    std::ostringstream content;
    content << file.rdbuf();  // Lire tout le fichier
    return content.str();
}

void handle_request(int client_socket) {
    char buffer[1024] = {0};
    int bytes_read = read(client_socket, buffer, 1024);
    if (bytes_read < 0) {
        std::cerr << "Erreur lors de la lecture de la requête." << std::endl;
        return;
    }

    std::string request(buffer);
    std::cout << "Requête reçue :\n" << request << std::endl;

    std::string filename = "page1.html"; // Par défaut, on sert page1.html
    std::string content_type = "text/html"; // Type de fichier

    if (request.find("GET /page2.html") != std::string::npos) {
        filename = "page2.html";
    } else if (request.find("GET /main.css") != std::string::npos) {
        filename = "main.css";
        content_type = "text/css";
    } else if (request.find("GET /toggle_led") != std::string::npos) {
        led_state = !led_state;
        std::cout << "LED basculée : " << (led_state ? "Allumée" : "Éteinte") << std::endl;
        filename = "page2.html"; // Redirige vers page2
    }

    std::string content = read_file(filename);

    // Remplacement des valeurs dynamiques (si c'est un fichier HTML)
    if (content_type == "text/html") {
        size_t pos;
        while ((pos = content.find("{{sensor_value}}")) != std::string::npos) {
            content.replace(pos, std::string("{{sensor_value}}").length(), std::to_string(sensor_value));
        }
        while ((pos = content.find("{{led_state}}")) != std::string::npos) {
            content.replace(pos, std::string("{{led_state}}").length(), led_state ? "Allumée" : "Éteinte");
        }
    }


    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type + "\r\n\r\n" + content;
    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Échec de la création du socket." << std::endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Échec du bind." << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Échec de l'écoute." << std::endl;
        return -1;
    }

    std::cout << "Serveur démarré sur le port 80..." << std::endl;

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            std::cerr << "Erreur lors de l'acceptation de la connexion." << std::endl;
            continue;
        }
        std::cout << "Nouvelle connexion acceptée." << std::endl;
        handle_request(new_socket);
    }

    return 0;
}
