
#ifdef DEBUG
#define DBG(a, ...)                                                            \
  { printf("[DEBUG]: " a "\n", ##__VA_ARGS__); }
#else
#define DBG(a, ...)                                                            \
  {}
#endif

#define INFO(a, ...)                                                           \
  { printf("[INFO]: " a "\n", ##__VA_ARGS__); }

#define ERROR(a, ...)                                                          \
  { printf("[ERROR]: " a "\n", ##__VA_ARGS__); }
