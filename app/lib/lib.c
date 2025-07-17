/*
 * Copyright (C) 2013, 2014 F4OS Authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
 


//#include <kernel/fault.h>

void *memchr(const void *ptr, int value, size_t num) {
    const unsigned char *p = ptr;

    while (num--) {
        if (*p == value) {
            return (void *) p;
        }
        p++;
    }

    return NULL;
}

int memcmp(const void *ptr1, const void *ptr2, int num) {
    const unsigned char *p1 = ptr1;
    const unsigned char *p2 = ptr2;

    while (num--) {
        if (*p1 != *p2) {
            if (*p1 > *p2) {
                return 1;
            }
            else {
                return -1;
            }
        }

        p1++;
        p2++;
    }

    return 0;
}

 
void memset(void *p, uint8_t value, uint32_t size) {
    uint8_t *end = (uint8_t *) ((uintptr_t) p + size);

    while ( (uint8_t*) p < end ) {
        *((uint8_t*)p) = value;
        p++;
    }
}


void    *memcpy(
      void      *s, /* Destination address          */
      const void    *ct,    /* source address           */
      int       n   /* number of bytes to copy      */
    )
{
    register int i;
    char *dst = (char *)s;
    char *src = (char *)ct;

    for (i = 0; i < n; i++)
    {
        *dst++ = *src++;
    }
    return s;
}

// Overlap-safe memcpy
void memmove(void *dst, const void *src, size_t n) {
    const char *s = src;
    char *d = dst;
    if ((uintptr_t)s < (uintptr_t)d)
        while(n--) d[n] = s[n];
    else
        while(n--) *d++ = *s++;
}

char *strchr(const char *s, int c) {
    while (*s != c && *s) {
        s++;
    }

    if (*s == c) {
        return (char *) s;
    }

    return NULL;
}

int strlen(const char *s) {
    int len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

 

void strreverse(char *s) {
    char *begin = s;

    if (!s || !*s) {
        return;
    }

    while (*++s);
    s--;

    while (s > begin) {
        char temp = *begin;
        *begin++ = *s;
        *s-- = temp;
    }
}

int strcmp(const char *s, const char *p) {
    while (*s == *p && *s != '\0' && *p != '\0') {
        s++;
        p++;
    }

    if (*s == *p) {
        return 0;
    }
    else if (*s > *p) {
        return 1;
    }
    else {
        return -1;
    }
}

int strncmp(const char *s, const char *p, int n) {
    while (*s == *p && *s != '\0' && *p != '\0' && n) {
        s++;
        p++;
        n--;
    }

    if (!n || *s == *p) {
        return 0;
    }
    else if (*s > *p) {
        return 1;
    }
    else {
        return -1;
    }
}

char *strncpy(char *destination, const char *source, int num) {
    char *ret = destination;

    while (*source && num-- > 0) {
        *destination++ = *source++;
    }

    while (num-- > 0) {
        *destination++ = '\0';
    }

    return ret;
}

// Find if a character is in a list
int chrnlst(char c, const char *l) {
    for (int i = 0; l[i] != '\0'; i++) {
        if (c == l[i])
            return 1;
    }
    return 0;
}


char    *strncat(
      char      *s1,        /* first string         */
      const char    *s2,        /* second string        */
      int       n       /* length to concatenate    */
    )
{
    char *os1;

    os1 = s1;
    while (*s1++)
        ;
    --s1;
    while ((*s1++ = *s2++))
        if (--n < 0)
        {
            *--s1 = '\0';
            break;
        }
    return (os1);
}

char    *strstr(
      const char    *cs,        /* String to search     */
      const char    *ct     /* Substring to locate      */
    )
{
    char *cq;
    char *cr;

    for (; *cs != '\0'; cs++)
    {
        if (*cs == *ct)
        {
            cq = (char *)cs;
            cr = (char *)ct;
            while ((*cq != '\0') && (*cr != '\0'))
            {
                if (*cq != *cr)
                {
                    break;
                }
                cq++;
                cr++;
            }
            if ('\0' == *cr)
            {
                return (char *)cs;
            }
        }
    }
    return 0;
}


int strcpy(
      char      *tar,       /* target string        */
      const char        *src        /* source string        */
    )
{
    while ( (*tar++ = *src++) != '\0') {
        ;
    }
    return 0;
}

int strspn (const char *s1,const char *s2)
{
  const char *s = s1;
  const char *c;

  while (*s1)
    {
      for (c = s2; *c; c++)
    {
      if (*s1 == *c)
        break;
    }
      if (*c == '\0')
    break;
      s1++;
    }

  return s1 - s;
}

char * strcat(char *dest, const char *src)
{
    int offset;

    offset = strlen(dest);
    strcpy(dest+offset, src);
    return dest;
}

int
strcspn(s1, s2)
    const char *s1;
    register const char *s2;
{
    register const char *p, *spanp;
    register char c, sc;

    /*
     * Stop as soon as we find any character from s2.  Note that there
     * must be a NUL in s2; it suffices to stop when we find that, too.
     */
    for (p = s1;;) {
        c = *p++;
        spanp = s2;
        do {
            if ((sc = *spanp++) == c)
                return (p - 1 - s1);
        } while (sc != 0);
    }
    /* NOTREACHED */
}

int strnlen(
      const char    *s,     /* string           */
       int  len     /* max length           */
        )
{
    int n;

    n = 0;
    while (*s++ && n < len)
        n++;

    return (n);
}

char * strtok(char *s,const char* delim)
{
     char *spanp;
     int c, sc;
    char *tok;
    static char *last;


    if (s == NULL && (s = last) == NULL)
        return (NULL);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {       /* no non-delimiter characters */
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = (char *)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}


char    *strrchr(
      const char    *s,     /* String to search     */
      int       c       /* Character to locate      */
    )
{
    char *r = 0;

    for (; *s != '\0'; s++)
    {
        if (*s == (const char)c)
        {
            r = (char *)s;
        }
    }

    if ((const char)c == *s)
    {
        return (char *)s;
    }

    return r;
}



long    labs(
      long      arg
    )
{
    if (arg < 0)
        arg = -arg;
    return (arg);
}

int abs(int n) {
    return n > 0 ? n : -n;
}

char *strndup(const char *str, int n) {
    int len = strnlen(str, n);

    char *dup = malloc(len+1);  // Room for NULL
    if (!dup) {
        return NULL;
    }

    strncpy(dup, str, len);
    dup[len] = '\0';

    return dup;
}

char *strdup(const char *str) {
    return strndup(str, strlen(str));
}

long    atol(
          char      *p
        )
{
    long n;
    int f;

    n = 0;
    f = 0;
    for (;; p++)
    {
        switch (*p)
        {
        case ' ':
        case '\t':
            continue;
        case '-':
            f++;
        case '+':
            p++;
        }
        break;
    }
    while (*p >= '0' && *p <= '9')
    {
        n = n * 10 + *p++ - '0';
    }
    return (f ? -n : n);
}


int atoi(char buf[]) {
    int result = 0;
    int negate = 0;
    int i = 0;

    // ignore leading whitespace
    while (chrnlst(buf[i]," \t\r\n")) {
        i++;
    }

    // check for negative
    if (buf[i] == '-') {
        negate = 1;
        i++;
    }
    else if (buf[i] == '+') {
        i++;
    }

    // parse decimal number
    while (buf[i] >= '0' && buf[i] <= '9') {
        result *= 10;
        result += (buf[i] - '0');
        i++;
    }
    return result * (negate ? -1 : 1);
}

/* Safely takes absolute value of an int,
 * properly handling INT_MIN */
static uint32_t int_abs(int n) {
    if (n > 0) {
        return n;
    }
    else {
        return UINT_MAX - (uint32_t) n + 1;
    }
}

char *itoa(int number, char *buf, uint32_t len, uint32_t base) {
    int i = 0;

    if (!len) {
        return NULL;
    }

    if (base < 2 || base > 36) {
        return NULL;
    }

    if (number < 0) {
        buf[i++] = '-';
        len--;
    }

    number = int_abs(number);

    return uitoa(number, &buf[i], len, base);
}

/* Converts number to string, base 2 to 36.
 * Returns NULL on invalid base or insufficient space */
char *uitoa(uint32_t number, char *buf, uint32_t len, uint32_t base) {
    char lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXY";
    int i = 0;

    if (!len) {
        return NULL;
    }

    if (base < 2 || base > 36) {
        return NULL;
    }

    do {
        buf[i++] = lookup[number % base];
    } while (--len && (number /= base) > 0);

    if (!len) {
        return NULL;
    }

    buf[i] = '\0';

    strreverse(buf);

    return buf;
}

#if 0
/* Based on http://stackoverflow.com/a/2303798/10817 by Sophy Pal */
void ftoa(float num, float tolerance, char buf[], uint32_t n) {
    int m;
    int digit;

    if (isnan(num)) {
        buf[0%n] = 'n';
        buf[1%n] = 'a';
        buf[2%n] = 'n';
        buf[3%n] = '\0';
        return;
    }
    if (isinf(num)) {
        if (ispos(num)) {
            buf[0%n] = 'i';
            buf[1%n] = 'n';
            buf[2%n] = 'f';
            buf[3%n] = '\0';
            return;
        }
        else {
            buf[0%n] = '-';
            buf[1%n] = 'i';
            buf[2%n] = 'n';
            buf[3%n] = 'f';
            buf[4%n] = '\0';
            return;
        }
    }

    if (num < 0.0f) {
        *(buf++) = '-';
        n--;
        num = -num;
    }

    m = logarithm(num, 1);

    if (m < 0) {
        *(buf++) = '0';
        *(buf++) = '.';
    }

    while ((num > 0 + tolerance || m >= 0) && n > 1) {
        float weight = powf(10.0f, m);
        digit = floorf(num / weight);
        num -= (digit*weight);
        *(buf++)= '0' + digit;
        n--;
        if (m == 0 && n > 1) {
            *(buf++) = '.';
            n--;
        }
        m--;
    }

    /* Print zero after decimal at end */
    if (*(buf-1) == '.' && n > 1) {
        *(buf++) = '0';
        n--;
    }

    *(buf) = '\0';
}
#endif


void    bzero(
      void      *p,
      int       len
    )
{
    int n;
    char *pch = (char *)p;

    if ((n = len) <= 0)
    {
        return;
    }
    do
    {
        *pch++ = 0;
    }
    while (--n);
}
