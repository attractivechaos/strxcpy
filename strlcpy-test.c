#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// generate a long string; buf[] must be at least len+1 long
void gen_hex_str(char *buf, long len)
{
	const char *hex = "0123456789ABCDEF";
	long i;
	for (i = 0; i < len; ++i)
		buf[i] = hex[i&0xf];
	buf[i] = 0;
}

// my preferred strncpy() replacement; returns the number of chars written, including NULL
size_t strxcpy(char *dst, const char *src, size_t size)
{
	size_t i = 0;
	if (size > 0) {
		for (i = 0; i < size - 1 && src[i]; ++i)
			dst[i] = src[i];
		dst[i] = 0;
		return i + 1;
	} else return 0;
}

// from https://github.com/git/git/blob/master/compat/strlcpy.c
size_t gitstrlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}

// test strncpy-like prototype
void test_strncpy(char *dst, const char *src, long len, long step, char *(*cpy_func)(char *, const char *, size_t))
{
	long k, i;
	for (i = k = 0; i < len; i += step + step) {
		long n = i + step < len? step : len - i;
		cpy_func(&dst[k], &src[i], n);
		k += n;
	}
	dst[k] = 0;
}

// test strlcpy-like prototype
void test_strlcpy(char *dst, const char *src, long len, long step, size_t (*cpy_func)(char *, const char *, size_t))
{
	long k, i;
	for (i = k = 0; i < len; i += step + step) {
		long n = i + step < len? step : len - i;
		cpy_func(&dst[k], &src[i], n + 1);
		k += n;
	}
}

// test snprintf
void test_snprintf(char *dst, const char *src, long len, long step)
{
	long k, i;
	for (i = k = 0; i < len; i += step + step) {
		long n = i + step < len? step : len - i;
		snprintf(&dst[k], n + 1, "%*s", (int)n, &src[i]);
		k += n;
	}
}

// test snprintf
void test_memcpy(char *dst, const char *src, long len, long step)
{
	long k, i;
	for (i = k = 0; i < len; i += step + step) {
		long n = i + step < len? step : len - i;
		memcpy(&dst[k], &src[i], n);
		k += n;
	}
}

int main(int argc, char *argv[])
{
	long len = 3000000, step = 11;
	char *dst, *src;
	clock_t t;

	if (argc > 1) len = atol(argv[1]);
	if (argc > 2) step = atol(argv[2]);
	src = (char*)malloc(len + 1);
	dst = (char*)malloc(len + 1);
	gen_hex_str(src, len);

	t = clock();
	test_strncpy(dst, src, len, step, strncpy);
	printf("strncpy from libc: %.3f sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);

	t = clock();
	test_strlcpy(dst, src, len, step, strxcpy);
	printf("strxcpy by me: %.3f sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);

	t = clock();
	test_memcpy(dst, src, len, step);
	printf("memcpy from libc: %.3f sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);

#ifdef __MACH__ // Linux doesn't have strlcpy()
	t = clock();
	test_strlcpy(dst, src, len, step, strlcpy);
	printf("strlcpy from libc: %.3f sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);
#endif

	t = clock();
	test_strlcpy(dst, src, len, step, gitstrlcpy);
	printf("gitstrlcpy from git: %.3f sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);

	t = clock();
	test_snprintf(dst, src, len, step);
	printf("snprintf from libc: %.3f sec\n", (double)(clock() - t) / CLOCKS_PER_SEC);

	free(dst);
	free(src);
	return 0;
}
