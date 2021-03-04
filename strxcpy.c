#include <stddef.h> /* for size_t */

/* my preferred strncpy() replacement; returns the number of chars copied */
size_t strxcpy(char *dst, const char *src, size_t size)
{
	size_t i = 0;
	if (size > 0) {
		for (i = 0; i < size - 1 && src[i]; ++i)
			dst[i] = src[i];
		dst[i] = 0;
	}
	return i;
}
