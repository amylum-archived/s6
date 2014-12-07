it: choose compile compilefordynlib load makelib makedynlib


compile: make-compile conf-cc stupid path-include warn-auto.sh
	./make-compile > compile && chmod 755 compile

compilefordynlib: make-compilefordynlib conf-cc stupid path-include warn-auto.sh
	./make-compilefordynlib > compilefordynlib && chmod 755 compilefordynlib

load: make-load conf-ld conf-stripbins stupid path-library path-library.so warn-auto.sh
	./make-load > load && chmod 755 load

makelib: conf-striplibs make-makelib warn-auto.sh systype
	./make-makelib > makelib && chmod 755 makelib

makedynlib: make-makedynlib warn-auto.sh
	./make-makedynlib > makedynlib && chmod 755 makedynlib
