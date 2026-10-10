#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>

#define PORT 3000
#define BUFFER_SIZE 8192
#define MAX_CONNECTIONS 100
#define MAX_HISTORY 1000

typedef struct {
    char user_input[2048];\n    char ai_response[2048];
} Message;

typedef struct {
    Message messages[MAX_HISTORY];
    int count;
    pthread_mutex_t lock;
} Conversation;

Conversation global_memory;

typedef struct {
    int socket;
    struct sockaddr_in address;
} ClientConnection;

void init_memory() {
    global_memory.count = 0;
    pthread_mutex_init(&global_memory.lock, NULL);
}

void add_to_memory(const char *user_input, const char *ai_response) {
    pthread_mutex_lock(&global_memory.lock);
    if (global_memory.count < MAX_HISTORY) {
        strncpy(global_memory.messages[global_memory.count].user_input, user_input, 2047);
        strncpy(global_memory.messages[global_memory.count].ai_response, ai_response, 2047);
        global_memory.count++;
    }
    pthread_mutex_unlock(&global_memory.lock);
}

char *get_memory_context() {
    static char context[4096];
    pthread_mutex_lock(&global_memory.lock);
    
    strcpy(context, "");
    int start = (global_memory.count > 5) ? global_memory.count - 5 : 0;
    
    for (int i = start; i < global_memory.count; i++) {
        strcat(context, "User: ");
        strcat(context, global_memory.messages[i].user_input);
        strcat(context, " | AI: ");
        strcat(context, global_memory.messages[i].ai_response);
        strcat(context, " || ");
    }
    
    pthread_mutex_unlock(&global_memory.lock);
    return context;
}

void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
}

const char *generate_response(const char *user_input) {
    static char response[2048];
    char lower_input[1024];
    strcpy(lower_input, user_input);
    to_lowercase(lower_input);

    char *memory = get_memory_context();

    if (strstr(lower_input, "seu nome") || strstr(lower_input, "quem é você")) {
        snprintf(response, sizeof(response), 
            "Sou o Olympus, uma IA conversacional criada para ser seu assistente inteligente. Tenho memória das nossas conversas e posso ajudar com ideias, redação, estudos, produtividade e muito mais.");
    } else if (strstr(lower_input, "estudo") || strstr(lower_input, "aprender") || strstr(lower_input, "plano")) {
        snprintf(response, sizeof(response), 
            "Para aprender eficientemente: 1) Defina objetivos claros, 2) Divida em blocos de 45-50 minutos, 3) Faça pausas de 10-15 minutos, 4) Revise regularmente, 5) Pratique ativamente. A consistência é mais importante que a intensidade.");
    } else if (strstr(lower_input, "email") || strstr(lower_input, "carta") || strstr(lower_input, "mensagem")) {
        snprintf(response, sizeof(response), 
            "Escrever um email eficaz: use um assunto claro e direto, comece com uma saudação apropriada, vá direto ao ponto, seja conciso, inclua um chamado à ação e revise antes de enviar. Evite jargão desnecessário.");
    } else if (strstr(lower_input, "resum") || strstr(lower_input, "condensa")) {
        snprintf(response, sizeof(response), 
            "Um bom resumo responde: O quê está sendo apresentado? Por quê é importante? Qual é a conclusão? Mantenha apenas as ideias principais, use suas próprias palavras e organize logicamente.");
    } else if (strstr(lower_input, "negócio") || strstr(lower_input, "startup") || strstr(lower_input, "empreend")) {
        snprintf(response, sizeof(response), 
            "Estratégia de negócio: identifique seu público, defina uma proposta de valor clara e diferenciada, crie um plano de execução realista, meça resultados constantemente e ajuste conforme necessário.");
    } else if (strstr(lower_input, "criativo") || strstr(lower_input, "ideia") || strstr(lower_input, "brainstorm")) {
        snprintf(response, sizeof(response), 
            "Ideias criativas surgem quando você combina: um problema bem definido, conhecimento profundo do seu público e pensamento divergente. Às vezes, olhar para fora da sua área inspira soluções inovadoras.");
    } else if (strstr(lower_input, "motiva") || strstr(lower_input, "desânimo") || strstr(lower_input, "cansaço")) {
        snprintf(response, sizeof(response), 
            "Quando a motivação cai: conecte-se ao seu \"porquê\", divida objetivos em passos menores, celebre pequenas vitórias, tire pausas regulares e lembre-se que nem sempre é preciso estar em pico. Progressão constante é melhor que perfeição.");
    } else if (strstr(lower_input, "tecnologia") || strstr(lower_input, "programa") || strstr(lower_input, "código")) {
        snprintf(response, sizeof(response), 
            "Na tecnologia: aprenda o fundamento antes de frameworks, pratique resolvendo problemas reais, leia código de outros, colabore com comunidades e mantenha-se atualizado. Constância no aprendizado é essencial.");
    } else if (strstr(lower_input, "relação") || strstr(lower_input, "amizade") || strstr(lower_input, "comunicação")) {
        snprintf(response, sizeof(response), 
            "Boas relações se constroem na comunicação honesta, empatia genuína e consistência. Ouça atentamente, seja autêntico, reconheça pontos de vista diferentes e invista tempo naquilo que importa. Qualidade vence quantidade.");
    } else if (strstr(lower_input, "saúde") || strstr(lower_input, "exercício") || strstr(lower_input, "sono")) {
        snprintf(response, sizeof(response), 
            "Para saúde integral: mantenha sono regular (7-9 horas), exercite-se 3-4x por semana, alimente-se consciente, hidrate-se bem e gerencie estresse com meditação ou respiraçao. Pequenos hábitos com consistência transformam tudo.");
    } else if (strstr(lower_input, "qual foi") || strstr(lower_input, "você falou") || strstr(lower_input, "lembra")) {
        snprintf(response, sizeof(response), 
            "Tenho memória! Estou analisando nossas últimas conversas... Se você quer que eu relembre algo específico, pode repetir ou descrever melhor o assunto. Estou aqui para continuar de onde paramos.");
    } else {
        snprintf(response, sizeof(response), 
            "Essa é uma pergunta interessante! Posso ajudar criando um plano de ação, resumindo conceitos, gerando ideias, escrevendo textos ou aprofundando em qualquer assunto. O que você gostaria de explorar?");
    }

    return response;
}

void escape_json_string(const char *input, char *output, size_t max_len) {
    size_t j = 0;
    for (size_t i = 0; input[i] && j < max_len - 1; i++) {
        switch (input[i]) {
            case '"':
                if (j + 1 < max_len) { output[j++] = '\\'; output[j++] = '"'; }
                break;
            case '\\':
                if (j + 1 < max_len) { output[j++] = '\\'; output[j++] = '\\'; }
                break;
            case '\n':
                if (j + 1 < max_len) { output[j++] = '\\'; output[j++] = 'n'; }
                break;
            case '\r':
                if (j + 1 < max_len) { output[j++] = '\\'; output[j++] = 'r'; }
                break;
            default:
                output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

void send_http_response(int client_socket, const char *json_response) {
    char http_response[16384];
    snprintf(http_response, sizeof(http_response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Connection: close\r\n"
        "Content-Length: %lu\r\n"
        "\r\n"
        "%s",
        strlen(json_response), json_response);
    
    send(client->socket, http_response, strlen(http_response), 0);
}

void send_http_file(int client_socket, const char *filename, const char *content_type) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        char error_response[] = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
        send(client_socket, error_response, strlen(error_response), 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_type, file_size);
    
    send(client_socket, header, strlen(header), 0);

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes, 0);
    }

    fclose(file);
}

void extract_json_value(const char *json, const char *key, char *value, size_t max_len) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);
    
    const char *start = strstr(json, search_key);
    if (!start) {
        value[0] = '\0';
        return;
    }

    start += strlen(search_key);
    size_t i = 0;
    
    while (start[i] != '"' && i < max_len - 1) {
        if (start[i] == '\\' && start[i+1] == '"') {
            value[i++] = '"';
            start++;
        } else {
            value[i] = start[i];
            i++;
        }
    }
    value[i] = '\0';
}

void handle_client(void *arg) {
    ClientConnection *client = (ClientConnection *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    bytes_received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("recv failed");
        close(client->socket);
        free(client);
        pthread_exit(NULL);
    }

    buffer[bytes_received] = '\0';
    printf("Requisição recebida:\n%s\n", buffer);

    if (strstr(buffer, "GET / ") || strstr(buffer, "GET /index.html")) {
        send_http_file(client->socket, "index.html", "text/html; charset=utf-8");
    } else if (strstr(buffer, "GET /styles.css")) {
        send_http_file(client->socket, "styles.css", "text/css");
    } else if (strstr(buffer, "GET /app.js")) {
        send_http_file(client->socket, "app.js", "application/javascript");
    } else if (strstr(buffer, "POST /api/chat")) {
        char *body_start = strstr(buffer, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            
            char user_input[2048] = {0};
            extract_json_value(body_start, "message", user_input, sizeof(user_input));

            if (strlen(user_input) > 0) {
                const char *ai_response = generate_response(user_input);
                add_to_memory(user_input, ai_response);
                
                char escaped_response[4096];
                escape_json_string(ai_response, escaped_response, sizeof(escaped_response));
                
                char json_response[8192];
                snprintf(json_response, sizeof(json_response),
                    "{\"response\": \"%s\", \"timestamp\": %ld}",
                    escaped_response, time(NULL));
                
                send_http_response(client->socket, json_response);
            } else {
                char error[] = "{\"error\": \"Campo 'message' vazio\"}";
                send_http_response(client->socket, error);
            }
        }
    } else if (strstr(buffer, "GET /api/memory")) {
        char *memory = get_memory_context();
        char escaped_memory[8192];
        escape_json_string(memory, escaped_memory, sizeof(escaped_memory));
        
        char json_response[16384];
        snprintf(json_response, sizeof(json_response),
            "{\"memory\": \"%s\", \"message_count\": %d}",
            escaped_memory, global_memory.count);
        
        send_http_response(client->socket, json_response);
    } else if (strstr(buffer, "OPTIONS /api/chat")) {
        char http_response[] = 
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
        send(client->socket, http_response, strlen(http_response), 0);
    } else if (strstr(buffer, "GET /api/health")) {
        char json_response[] = "{\"status\": \"ok\", \"name\": \"Olympus AI\", \"version\": \"1.0\"}";
        send_http_response(client->socket, json_response);
    } else {
        char error[] = "{\"error\": \"Rota não encontrada\"}";
        send_http_response(client->socket, error);
    }

    close(client->socket);
    free(client);
    pthread_exit(NULL);
}

int main() {
    int server_socket;
    struct sockaddr_in server_address;
    ClientConnection *client;
    pthread_t thread_id;

    init_memory();

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_socket, MAX_CONNECTIONS) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("🚀 Servidor Olympus AI rodando em http://localhost:%d\n", PORT);
    printf("Aguardando conexões...\n\n");

    while (1) {
        client = malloc(sizeof(ClientConnection));
        socklen_t address_len = sizeof(client->address);

        client->socket = accept(server_socket, (struct sockaddr *)&client->address, &address_len);
        if (client->socket < 0) {
            perror("Accept failed");
            free(client);
            continue;
        }

        printf("📌 Nova conexão de %s:%d\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port));

        if (pthread_create(&thread_id, NULL, (void *(*)(void *))handle_client, client) != 0) {
            perror("Thread creation failed");
            close(client->socket);
            free(client);
        } else {
            pthread_detach(thread_id);
        }
    }

    close(server_socket);
    return 0;
}
