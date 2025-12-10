.PHONY: all

all: kubsh           # Компиляция из исходников (цель по умолчанию)

kubsh: src/*.cpp
	g++ -o kubsh src/*.cpp -std=c++17 -Wall

run: kubsh           # Запуск kubsh
	./kubsh

deb:                 # Сборка deb-пакета
	dpkg-buildpackage -us -uc -b

clean:              # Очистка
	rm -f kubsh *.deb
