strlcpy-test:strlcpy-test.c
	$(CC) -O2 -o $@ $<

clean:
	rm -f strlcpy-test a.out
