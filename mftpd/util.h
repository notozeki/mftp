#ifndef UTIL_H
#define UTIL_H

#define LITTLEENDIAN (!byte_order())
#define BIGENDIAN (byte_order())
int byte_order();
unsigned int order_flip(unsigned int data);

int string_split(char* str, char del, int* countp, char*** vectorp);
void free_string_vector(int count, char** vectorp);

#endif // UTIL_H
