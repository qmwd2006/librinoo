/**
 * @file   buffer.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Feb  1 18:56:27 2017
 *
 * @brief  Contains functions to create a buffer (pointer & size)
 *
 *
 */

#include "rinoo/memory/module.h"

static rn_buffer_class_t default_class = {
	.inisize = RN_BUFFER_HELPER_INISIZE,
	.maxsize = RN_BUFFER_HELPER_MAXSIZE,
	.init = NULL,
	.growthsize = rn_buffer_helper_growthsize,
	.malloc = rn_buffer_helper_malloc,
	.realloc = rn_buffer_helper_realloc,
	.free = rn_buffer_helper_free,
};

static rn_buffer_class_t static_class = {
	.inisize = 0,
	.maxsize = 0,
	.init = NULL,
	.growthsize = NULL,
	.malloc = NULL,
	.realloc = NULL,
	.free = NULL,
};

/**
 * Creates a new buffer. It uses the buffer class for memory allocation.
 * If class is NULL, then default buffer class is used.
 *
 * @param class Buffer class to be used.
 *
 * @return Pointer to the created buffer.
 */
rn_buffer_t *rn_buffer_create(rn_buffer_class_t *class)
{
	rn_buffer_t *buffer;

	buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	if (class == NULL) {
		class = &default_class;
	}
	buffer->class = class;
	buffer->msize = class->inisize;
	if (class->init != NULL && class->init(buffer) != 0) {
		free(buffer);
		return NULL;
	}
	if (class->malloc != NULL) {
		buffer->ptr = class->malloc(buffer, class->inisize);
		if (buffer->ptr == NULL) {
			free(buffer);
			return NULL;
		}
	}
	return buffer;
}

/**
 * Initializes a static buffer.
 *
 * @param buffer Pointer to the buffer to init.
 * @param ptr Pointer to the static memory.
 * @param size Size of the static memory.
 */
void rn_buffer_static(rn_buffer_t *buffer, void *ptr, size_t size)
{
	buffer->ptr = ptr;
	buffer->size = size;
	buffer->msize = 0;
	buffer->class = &static_class;
}

/**
 * Initializes a buffer to use a specific memory segment.
 * This memory segment needs read & write access.
 * This memory segment won't be extended (reallocated).
 *
 * @param buffer Pointer to the buffer to init.
 * @param ptr Pointer to the memory segment to use.
 * @param msize Size of the memory segment.
 */
void rn_buffer_init(rn_buffer_t *buffer, void *ptr, size_t msize)
{
	buffer->ptr = ptr;
	buffer->size = 0;
	buffer->msize = msize;
	buffer->class = &static_class;
}

/**
 * Destroys a buffer.
 *
 * @param buffer Pointer to the buffer to destroy.
 *
 * @return 0 on success, or -1 if an error occurs.
 */
int rn_buffer_destroy(rn_buffer_t *buffer)
{
	if (buffer->ptr != NULL && buffer->class->free != NULL) {
		if (buffer->class->free(buffer) != 0) {
			return -1;
		}
		buffer->ptr = NULL;
	}
	free(buffer);
	return 0;
}

/**
 * Extends a buffer. It tries to set new size to (size * 2).
 *
 * @param buffer Pointer to the buffer to extend.
 * @param size New desired size.
 *
 * @return 0 if succeeds, -1 if an error occurs.
 */
int rn_buffer_extend(rn_buffer_t *buffer, size_t size)
{
	void *ptr;
	size_t msize;

	if (buffer->class->growthsize == NULL || buffer->class->realloc == NULL) {
		return -1;
	}
	msize = buffer->class->growthsize(buffer, size);
	if (msize < size) {
		return -1;
	}
	ptr = buffer->class->realloc(buffer, msize);
	if (ptr == NULL) {
		return -1;
	}
	buffer->ptr = ptr;
	buffer->msize = msize;
	return 0;
}

/**
 * It is vprintf-like function which tries to add data to the buffer
 * and it will try to extend this buffer if it is to small.
 * This function uses vsnprintf, you can find how to use the format
 * string with man vsnprintf.
 *
 * @param buffer Pointer to the buffer to add data to.
 * @param format Format string which defines subsequent arguments.
 * @param ap Vararg rn_list.
 *
 * @return Number of bytes printed if succeeds, else -1.
 */
int rn_buffer_vprint(rn_buffer_t *buffer, const char *format, va_list ap)
{
	int res;
	va_list ap2;

	va_copy(ap2, ap);
	while (((uint32_t) (res = vsnprintf(buffer->ptr + buffer->size,
				       buffer->msize - buffer->size,
				       format, ap2)) >= buffer->msize - buffer->size) &&
	       rn_buffer_extend(buffer, buffer->size + res + 1) == 0) {
		va_end(ap2);
		va_copy(ap2, ap);
	}
	va_end(ap2);
	buffer->size += res;
	return res;
}

/**
 * It is printf-like function which tries to add data to the buffer
 * and it will try to extend this buffer if it is to small.
 * This function uses rn_buffer_vprint.
 *
 * @param buffer Pointer to the buffer to add data to.
 * @param format Format string which defines subsequent arguments.
 *
 * @return Number of bytes printed if succeeds, else -1.
 */
int rn_buffer_print(rn_buffer_t *buffer, const char *format, ...)
{
	int res;
	va_list ap;

	va_start(ap, format);
	res = rn_buffer_vprint(buffer, format, ap);
	va_end(ap);
	return res;
}

/**
 * Adds data to a buffer. If the buffer is to small, this function
 * will try to extend it.
 *
 * @param buffer Buffer where the data will be added.
 * @param data Data to add to the buffer.
 * @param size Size of data.
 *
 * @return size if data is added to the buffer, else -1.
 */
int rn_buffer_add(rn_buffer_t *buffer, const char *data, size_t size)
{
	if (size + buffer->size > buffer->msize && rn_buffer_extend(buffer, size + buffer->size) < 0) {
		return -1;
	}
	memcpy(buffer->ptr + buffer->size, data, size);
	buffer->size += size;
	return size;
}

/**
 * Adds a string to a buffer. It actually calls rn_buffer_add with strlen
 * of str as size parameter.
 *
 * @param buffer Buffer where the string will be added
 * @param str String to add to the buffer
 *
 * @return Number of bytes added on success, or -1 if an error occurs
 */
int rn_buffer_addstr(rn_buffer_t *buffer, const char *str)
{
	return rn_buffer_add(buffer, str, strlen(str));
}

/**
 * Adds a null byte to the end of a buffer.
 *
 * @param buffer Buffer where the null byte will be added
 *
 * @return 0 on success, or -1 if an error occurs
 */
int rn_buffer_addnull(rn_buffer_t *buffer)
{
	if ((buffer->size == 0 || ((char *) buffer->ptr)[buffer->size - 1] != 0) && rn_buffer_add(buffer, "\0", 1) < 0) {
		return -1;
	}
	return 0;
}

/**
 * Erases beginning data in the buffer and moves the rest
 * to the beginning. This function does -not- reduce the buffer.
 *
 * @param buffer Buffer where data will be erased.
 * @param size Size to erase. If 0, the whole buffer is erased.
 *
 * @return 0 if data has been erased, else -1.
 */
int rn_buffer_erase(rn_buffer_t *buffer, size_t size)
{
	if (buffer->ptr == NULL) {
		return -1;
	}
	if (size == 0 || size >= buffer->size) {
		buffer->size = 0;
	} else {
		memmove(buffer->ptr, buffer->ptr + size, buffer->size - size);
		buffer->size -= size;
	}
	return 0;
}

/**
 * Duplicates a buffer.
 *
 * @param buffer Pointer to the buffer to duplicate.
 * @param class Pointer to the class buffer to use in the new buffer.
 *
 * @return A pointer to the new buffer, or NULL if an error occurs.
 */
rn_buffer_t *rn_buffer_dup_class(rn_buffer_t *buffer, rn_buffer_class_t *class)
{
	rn_buffer_t *newbuffer;

	if (class->malloc == NULL) {
		return NULL;
	}
	newbuffer = malloc(sizeof(*newbuffer));
	if (unlikely(newbuffer == NULL)) {
		return NULL;
	}
	*newbuffer = *buffer;
	if (newbuffer->msize == 0) {
		newbuffer->msize = buffer->size;
	}
	newbuffer->class = class;
	if (class->init != NULL && class->init(newbuffer) != 0) {
		free(newbuffer);
		return NULL;
	}
	newbuffer->ptr = class->malloc(newbuffer, newbuffer->msize);
	if (unlikely(newbuffer->ptr == NULL)) {
		free(newbuffer);
		return NULL;
	}
	memcpy(newbuffer->ptr, buffer->ptr, buffer->size);
	return newbuffer;
}

/**
 * Duplicates a buffer.
 *
 * @param buffer Pointer to the buffer to duplicate.
 *
 * @return A pointer to the new buffer, or NULL if an error occurs.
 */
rn_buffer_t *rn_buffer_dup(rn_buffer_t *buffer)
{
	return rn_buffer_dup_class(buffer, buffer->class);
}

/**
 * Compares two buffers
 *
 * @param buffer1 Pointer to a buffer.
 * @param buffer2 Pointer to a buffer.
 *
 * @return An integer less than, equal to, or greater than zero if buffer1 is found, respectively, to be less than, to match, or be greater than buffer2
 */
int rn_buffer_cmp(rn_buffer_t *buffer1, rn_buffer_t *buffer2)
{
	int ret;
	size_t min;

	min = (rn_buffer_size(buffer1) < rn_buffer_size(buffer2) ? rn_buffer_size(buffer1) : rn_buffer_size(buffer2));
	ret = memcmp(rn_buffer_ptr(buffer1), rn_buffer_ptr(buffer2), min);
	if (ret == 0) {
		ret = rn_buffer_size(buffer1) - rn_buffer_size(buffer2);
	}
	return ret;
}

/* Compares two buffers, ignoring the case of characters.
 *
 * @param buffer1 Pointer to a buffer.
 * @param buffer2 Pointer to a buffer.
 *
 * @return An integer less than, equal to, or greater than zero if buffer1 is found, respectively, to be less than, to match, or be greater than buffer2
 */
int rn_buffer_casecmp(rn_buffer_t *buffer1, rn_buffer_t *buffer2)
{
	char c1;
	char c2;
	size_t i;
	size_t min;

	min = (rn_buffer_size(buffer1) < rn_buffer_size(buffer2) ? rn_buffer_size(buffer1) : rn_buffer_size(buffer2));
	for (i = 0; i < min; i++) {
		c1 = ((const char *) rn_buffer_ptr(buffer1))[i];
		c2 = ((const char *) rn_buffer_ptr(buffer2))[i];
		switch (c1) {
			case 'a' ... 'z':
				if (c2 >= 'A' && c2 <= 'Z') {
					c2 += 'a' - 'A';
				}
				break;
			case 'A' ... 'Z':
				if (c2 >= 'a' && c2 <= 'z') {
					c1 += 'a' - 'A';
				}
				break;
		}
		if (c1 != c2) {
			return c1 - c2;
		}
	}
	return rn_buffer_size(buffer1) - rn_buffer_size(buffer2);
}

/**
 * Compares a buffer with a string.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than str
 */
int rn_buffer_strcmp(rn_buffer_t *buffer, const char *str)
{
	size_t i;
	char *ptr;

	ptr = rn_buffer_ptr(buffer);
	for (i = 0; i < rn_buffer_size(buffer) && str[i] != 0 && ptr[i] == str[i]; i++) {
	}
	if (i >= rn_buffer_size(buffer)) {
		return -str[i];
	}
	return ptr[i] - str[i];

}

/**
 * Compares a buffer to the first n bytes of a string.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 * @param len Maximum length of the string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than s2
 */
int rn_buffer_strncmp(rn_buffer_t *buffer, const char *str, size_t len)
{
	int ret;
	size_t min;

	min = (rn_buffer_size(buffer) < len ? rn_buffer_size(buffer) : len);
	ret = memcmp(rn_buffer_ptr(buffer), str, min);
	if (ret == 0 && rn_buffer_size(buffer) < len) {
		ret = rn_buffer_size(buffer) - len;
	}
	return ret;
}

/**
 * Compares a buffer with a string ignoring case.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than str
 */
int rn_buffer_strcasecmp(rn_buffer_t *buffer, const char *str)
{
	int ret;
	size_t min;
	size_t len;

	len = strlen(str);
	min = (rn_buffer_size(buffer) < len ? rn_buffer_size(buffer) : len);
	ret = strncasecmp(rn_buffer_ptr(buffer), str, min);
	if (ret == 0) {
		ret = rn_buffer_size(buffer) - len;
	}
	return ret;
}

/**
 * Compares a buffer to the first n bytes of a string ignoring case.
 *
 * @param buffer Pointer to a buffer.
 * @param str Pointer to a string.
 * @param len Maximum length of the string.
 *
 * @return An integer less than, equal to, or greater than zero if buffer is found, respectively, to be less than, to match, or be greater than s2
 */
int rn_buffer_strncasecmp(rn_buffer_t *buffer, const char *str, size_t len)
{
	int ret;
	size_t min;

	min = (rn_buffer_size(buffer) < len ? rn_buffer_size(buffer) : len);
	ret = strncasecmp(rn_buffer_ptr(buffer), str, min);
	if (ret == 0 && rn_buffer_size(buffer) < len) {
		ret = rn_buffer_size(buffer) - len;
	}
	return ret;
}

/**
 * Converts a buffer to a long int accordingly to strtol.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 * @param base Conversion base.
 *
 * @return Result of conversion.
 */
long int rn_buffer_tolong(rn_buffer_t *buffer, size_t *len, int base)
{
	long int result;
	char *endptr;
	rn_buffer_t *workbuf;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = rn_buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (rn_buffer_addnull(workbuf) == 0) {
		result = strtol(workbuf->ptr, &endptr, base);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		rn_buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer to an unsigned long int accordingly to strtoul.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 * @param base Conversion base.
 *
 * @return Result of conversion.
 */
unsigned long int rn_buffer_toulong(rn_buffer_t *buffer, size_t *len, int base)
{
	char *endptr;
	rn_buffer_t *workbuf;
	unsigned long int result;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = rn_buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (rn_buffer_addnull(workbuf) == 0) {
		result = strtoul(workbuf->ptr, &endptr, base);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		rn_buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer to a float accordingly to strtof.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 *
 * @return Result of conversion.
 */
float rn_buffer_tofloat(rn_buffer_t *buffer, size_t *len)
{
	float result;
	char *endptr;
	rn_buffer_t *workbuf;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = rn_buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (rn_buffer_addnull(workbuf) == 0) {
		result = strtof(workbuf->ptr, &endptr);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		rn_buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer to a double accordingly to strtod.
 *
 * @param buffer Pointer to a buffer to convert.
 * @param len If not NULL, it stores the buffer length processed for conversion.
 *
 * @return Result of conversion.
 */
double rn_buffer_todouble(rn_buffer_t *buffer, size_t *len)
{
	double result;
	char *endptr;
	rn_buffer_t *workbuf;

	workbuf = buffer;
	if (buffer->msize == 0) {
		/* Considering buffer->ptr has not been allocated */
		workbuf = rn_buffer_dup_class(buffer, &default_class);
		if (workbuf == NULL) {
			return 0;
		}
	}
	result = 0;
	endptr = workbuf->ptr;
	if (rn_buffer_addnull(workbuf) == 0) {
		result = strtod(workbuf->ptr, &endptr);
	}
	if (len != NULL) {
		*len = endptr - (char *) workbuf->ptr;
	}
	if (workbuf != buffer) {
		rn_buffer_destroy(workbuf);
	} else {
		/* Removing null byte */
		buffer->size--;
	}
	return result;
}

/**
 * Converts a buffer into a string.
 * It makes sure the buffer ends with \0 and returns the internal buffer pointer.
 * Buffer length changes, thus.
 *
 * @param buffer Pointer to the buffer to convert.
 *
 * @return A pointer to a string or NULL if an error occurs.
 */
char *rn_buffer_tostr(rn_buffer_t *buffer)
{
	if (rn_buffer_addnull(buffer) != 0) {
		return NULL;
	}
	return buffer->ptr;
}

/**
 * Encodes a buffer to base64.
 *
 * @param dst Pointer to the buffer to store the encoded string.
 * @param src Pointer to the buffer to encode.
 *
 * @return 0 on success, or -1 if an error occurs
 */
int rn_buffer_b64encode(rn_buffer_t *dst, rn_buffer_t *src)
{
	static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t i;
	char cur;
	int shift;
	unsigned int bits;

	bits = 0;
	shift = 0;
	for (i = 0; i < rn_buffer_size(src); i++)
	{
		bits = (bits << 8) + ((unsigned char *) rn_buffer_ptr(src))[i];
		shift += 8;
		do {
			cur = b64[(bits << 6 >> shift) & 0x3f];
			rn_buffer_add(dst, &cur, 1);
			shift -= 6;
		}
		while (shift > 6 || (i == rn_buffer_size(src) - 1 && shift > 0));
	}
	while (rn_buffer_size(dst) & 3) {
		rn_buffer_add(dst, "=", 1);
	}
	return 0;
}
