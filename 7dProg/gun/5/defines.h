// Имя очереди сообщений от клиента серверу
#define CLIENT_MQ "/server_mq"
// Имя очереди сообщений от сервера клиенту
#define SERVER_MQ "/client_mq"
// Права доступа к очереди сообщений
#define FILE_MODE S_IRUSR | S_IWUSR

// Размер буфера
#define BUFFER_SIZE 256

typedef struct Message {
  char content[BUFFER_SIZE];
  char symbol;
} Message;
