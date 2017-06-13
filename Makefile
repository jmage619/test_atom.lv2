all: test_atom.so test_atom_ui.so

test_atom.so: test_atom.c uris.h
	gcc -Wall -fPIC -shared -o test_atom.so test_atom.c

test_atom_ui.so: test_atom_ui.c lv2_external_ui.h uris.h
	gcc -Wall -fPIC -shared -o test_atom_ui.so test_atom_ui.c

clean:
	rm -f *.so
