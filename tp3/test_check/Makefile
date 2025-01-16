clean:
	rm -rf libelf tests_build

install: clean
	mkdir tests_build || true
	cd tests_build && cmake .. && make

alltests: clean install
	python3 runall.py