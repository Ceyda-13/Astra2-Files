default: libs game db
	@echo "--------------------------------------"
	@echo "Full Build Done"
	@echo "--------------------------------------"

libs: libgame liblua libsql libthecore libl
	@echo "--------------------------------------"
	@echo "Libs Build Done"
	@echo "--------------------------------------"

libgame: .
	@echo "--------------------------------------"
	@echo "libgame Build Start"
	$(MAKE) -C libsrc/$@/ clean
	$(MAKE) -C libsrc/$@/
	@echo "libgame Build Done"
	@echo "--------------------------------------"

liblua: .
	@echo "--------------------------------------"
	@echo "liblua Build Start"
	$(MAKE) -C libsrc/$@/ clean
	$(MAKE) -C libsrc/$@/
	@echo "liblua Build Done"
	@echo "--------------------------------------"

libsql: .
	@echo "--------------------------------------"
	@echo "libsql Build Start"
	$(MAKE) -C libsrc/$@/ clean
	$(MAKE) -C libsrc/$@/
	@echo "libsql Build Done"
	@echo "--------------------------------------"

libthecore: .
	@echo "--------------------------------------"
	@echo "libthecore Build Start"
	$(MAKE) -C libsrc/$@/ clean
	$(MAKE) -C libsrc/$@/
	@echo "libthecore Build Done"
	@echo "--------------------------------------"

libl: .
	@echo "--------------------------------------"
	@echo "libl Build Start"
	$(MAKE) -C libsrc/$@/ clean
	$(MAKE) -C libsrc/$@/
	@echo "libl Build Done"
	@echo "--------------------------------------"

game: .
	$(MAKE) -C $@/ clean
	$(MAKE) -C $@/
	$(MAKE) -C $@/ symlink

db: .
	$(MAKE) -C $@/ clean
	$(MAKE) -C $@/
	$(MAKE) -C $@/ symlink

