## TL;DR

```sh
git clone https://github.com/attractivechaos/strxcpy
cd strxcpy && make
./strlcpy-test
```
The output on my Mac laptop (strlcpy is much slower):
```txt
strncpy from libc: 0.003 sec
strxcpy by me: 0.002 sec
memcpy from libc: 0.001 sec
strlcpy from libc: 6.481 sec
gitstrlcpy from git: 6.274 sec
snprintf from libc: 6.433 sec
```

## The longer story

### strcpy and strncpy

In C, we often use the [strcpy][strcpy] function to copy strings. If the source
string is longer than the target buffer, we will have a buffer overflow, a bug.
The [strncpy][strncpy] function solves this problem by requiring the buffer
size. However, if the source string is longer than the buffer, strncpy does not
put the NULL character at the end of the target buffer. If we use strlen or
strcpy later on the target string, another buffer overflow.

### strlcpy

[Miller & Raadt][strlcpy-article] proposed [strlcpy][strlcpy] as a solution.
This new function returns the length of the source string and always adds NULL
to the target string as long as the buffer size is not zero. Many C programmers
recommend strlcpy over strncpy. The [strlcpy manpage][strlcpy] regards strlcpy
as a replacement of strncpy. The Git project even [bans strncpy][strncpy-ban].

I have never used strlcpy before as it doesn't conform to the C99 or POSIX
standards. When I took a more serious look today, I actually think it is no
better than strncpy. In [Git's implementation][git-strlcpy] of strlcpy, I
immediately noticed the strlen call. This leads to two problems below. First,
the source string `src` has to be NULL terminated. If it is not, a buffer
overflow. strncpy doesn't have this restriction. Second, while the time
complexity of strncpy is `O(min(bufSize,srcLen))`, the time complexity of
strlcpy is `O(srcLen)`. When we want to copy the first few characters in a long
source string, strlcpy can be much slower than strncpy.

### My preferred solution

We can mix the behavior of strncpy and strlcpy: the function `strxcpy` below
appends NULL to the target string but doesn't inspect more than `size-1`
characters in the source string. Its time complexity is the same as strncpy.
```c
/* my preferred string copy; returns the number of chars written, including NULL */
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
```

Not surprisingly, this function has its own flaws. Here, `size` is the buffer
size. The maximum number of characters to copy equals `size-1`, not `size`.
This is really the fault of strlcpy. I more like the [strncat][strncat]
behavior, which I will not detail here, but it may lead to an off-by-1 buffer
overflow if misused. There is not a universally good API.

### Benchmark

In this benchmark, I have a long string of 2 million characters and I am
copying many of its 11-char substrings to another buffer. The last substring
may be shorter than 11 chars. If we naively use strlcpy, we will get bad
performance because strlcpy always checks the whole string length and ends up
with a quadratic algorithm. This echoes the [Hacker News thread][sscanf] on 
quadratic-time parsing with sscanf.

This is a contrived example to highlight the issue with strlcpy. Experienced
programmers would probably use memcpy to avoid the trap. Nonetheless,
experienced programmers wouldn't fall into the strcpy/strncpy trap, either.
The discussion here is really about which method is less prone to misuses.
The strxcpy function above has one fewer pitfall and is probably the most
robust among the options so far. At the very least, people shouldn't take
strlcpy as a replacement of strncpy even if the [strlcpy manpage][strlcpy] says
so.

[strlcpy]: https://linux.die.net/man/3/strlcpy
[strncpy]: https://linux.die.net/man/3/strncpy
[strcpy]: https://linux.die.net/man/3/strcpy
[strncat]: https://linux.die.net/man/3/strncat
[strlcpy-article]: https://www.sudo.ws/todd/papers/strlcpy.html
[strncpy-ban]: https://github.com/git/git/commit/e488b7aba743d23b830d239dcc33d9ca0745a9ad
[git-strlcpy]: https://github.com/git/git/blob/master/compat/strlcpy.c
[sscanf]: https://news.ycombinator.com/item?id=26302744
