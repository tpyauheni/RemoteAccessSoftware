#pragma once
#ifndef RAC_SOFTWARE_BASE64
#define RAC_SOFTWARE_BASE64 1

char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);
unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length);
void _base64_init();
void _base64_destroy();

#endif // RAC_SOFTWARE_BASE64
