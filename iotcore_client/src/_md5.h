#pragma once

typedef struct
{
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];   
} MD5_CTX;

void _MD5Init(MD5_CTX *context);
void _MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputlen);
void _MD5Final(MD5_CTX *context, unsigned char digest[16]);