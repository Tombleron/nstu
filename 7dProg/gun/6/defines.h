// Имя семафора, которым управляет клиент
#define SEM_CLIENT "/client"
// Имя семафора, которым управляет сервер
#define SEM_SERVER "/server"
// Имя области разделяемой памяти
#define SHM "/shared_memory"
// Права доступа к семафорам
#define SEM_FILE_MODE S_IRUSR | S_IWUSR
// Права доступа к разделяемой памяти
#define SHM_FILE_MODE S_IRUSR | S_IWUSR
// Флаги для открытия семафора сервера
#define SEM_SERVER_FLAGS O_RDWR | O_CREAT
// Флаги для открытия семафора клиента
#define SEM_CLIENT_FLAGS O_RDWR | O_CREAT
// Флаги для разделяемой памяти
#define SHM_FLAGS O_RDWR | O_CREAT

// Размер буфера
#define BUFFER_SIZE 256

typedef struct Message {
  char content[BUFFER_SIZE];
  char symbol;
} Message;
