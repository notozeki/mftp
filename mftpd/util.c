#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

// リトルエンディアンなら0
// ビッグエンディアンなら1を返す
int byte_order()
{
	unsigned int data = 0xFF;
	unsigned char* p = (unsigned char*)&data;
	if ( p[0] == 0xFF ) return 0;
	else return 1;
}

unsigned int order_flip(unsigned int data)
{
	unsigned int ret = 0;
	unsigned char* p = (unsigned char*)&data;
	unsigned char* q = (unsigned char*)&ret;

	int i;
	for ( i = 0; i < sizeof(unsigned int); i++ ) {
		q[i] = p[sizeof(unsigned int)-1 - i];
	}
	return ret;
}


int countchr(char* s, char c)
{
	int count;
	for ( count = 0; *s; s++ ) {
		if ( *s == c ) count++;
	}
	return count;
}

int string_split(char* str, char del, int* countp, char*** vectorp)
{
	char** vec;
	int count_max, i, len;
	char* s;
	char* p;

	if ( str == NULL ) return -1;
	count_max = countchr(str, del) + 1;
	vec = malloc(sizeof(char*)*(count_max + 1));
	if ( vec == NULL ) return -1;

	for ( i = 0; i < count_max; i++ ) {
		while ( *str == del ) str++;
		if ( *str == '\0' ) break;
		for ( p = str; *p != del && *p != '\0'; p++ ) continue;
		len = p - str;
		s = malloc(len + 1);
		if ( s == NULL ) {
			int j;
			for ( j = 0; j < i; j++ ) {
				free(vec[j]);
				vec[j] = 0;
			}
			return -1;
		}
		memcpy(s, str, len);
		s[len] = 0;
		vec[i] = s;
		str = p;
	}
	vec[i] = NULL;
	*countp = i;
	*vectorp = vec;
	return i;
}

void free_string_vector(int count, char** vectorp)
{
	int i;
	for ( i = 0; i < count; i++ ) {
		if ( vectorp[i] == NULL ) break;
		free(vectorp[i]);
	}
	free(vectorp);
}
