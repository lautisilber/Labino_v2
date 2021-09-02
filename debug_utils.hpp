#ifdef DEBUG
    #define PRINT(...) Serial.printf(__VA_ARGS__)
#else
    #define PRINT(...)
#endif
