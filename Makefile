all:
	texi2pdf --batch m.tex >/dev/null || true

test:
	./tester.sh

clean:
	rm -f *.toc *.pdf *.out *.log *.aux *.idx *.enc

archive: clean
	RELEASE=$$(cat version); \
	/bin/echo "  [TAR] ~/meaningful-$$RELEASE.tar.gz"; \
	cd .. && tar czf ~/meaningful-src-$$RELEASE.tar.gz meaningful/
