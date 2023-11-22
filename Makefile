all:
	@echo "Issue 'make doc', 'make test' or 'make c'"

doc:
	texi2pdf --batch m.tex >/dev/null || true

test:
	./tester.sh

test2:
	./test2.sh

c:
	$(CC) -Wall -Wextra decoder.c -o decoder
	cd lib/c && make

clean:
	rm -f *.toc *.pdf *.out *.log *.aux *.idx *.enc
	cd lib/c && make clean

archive: clean
	RELEASE=$$(cat version); \
	/bin/echo "  [TAR] ~/meaningful-$$RELEASE.tar.gz"; \
	cd .. && tar czf ~/meaningful-src-$$RELEASE.tar.gz --exclude=\.git meaningful/
