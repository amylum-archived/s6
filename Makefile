it all: .done

install: .done .installed

.done:
	@test -d package && package/compile && : > .done

.installed:
	@test -d package && package/export && : > .installed

clean:
	@test -d package && rm -rf compile .done .installed

distclean: clean
	@test -d package && rm -rf command include library library.so sysdeps
