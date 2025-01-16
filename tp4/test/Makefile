clean:
	sudo rm -rf libelf tests_build

installcheckelf:
	python3 installcheckelf.py lcheck_pic || sudo apt-get install check
	python3 installcheckelf.py lelf || (git clone https://github.com/WolfgangSt/libelf.git -q && cd libelf && sudo ./configure && make -s && sudo make install -s)

install: installcheckelf
	python3 prep_tests.py
	mkdir tests_build || true
	temp=$$(cd tests_build && cmake .. && make)
	python3 clean_tests.py

singletest: clean install
	./tests_build/check_tests $(FOO)

alltests: clean install
	unzip floppy.zip
	python3 runall.py
