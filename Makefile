APXS2=/usr/sbin/apxs

all: mod_track.so

mod_track.so: mod_track.c
	$(APXS2) -c -Wc,-g mod_track.c

# RPM BUILDER
FILES=Makefile \
      mod_track.c \
          mod_track.spec \
          mod_track.conf \

DIRS=/usr/lib64/httpd/modules/ \
     /usr/bin/ \
         /etc/httpd/conf.d/ \

BUILDROOT=.

tarball: 
	tar -czv --exclude=.svn --dereference -f $(BUILDROOT)/mod_track.tgz $(FILES)

rpm: tarball
	rpmbuild -ta $(BUILDROOT)/mod_track.tgz

makedirs: $(INSTALL_ROOT)
	test -d $(INSTALL_ROOT) || echo "Invalid build root specified - $(INSTALL_ROOT)"
	for dir in $(DIRS); do mkdir -p $(INSTALL_ROOT)/$$dir; done

install: $(INSTALL_ROOT) makedirs
	cp -v .libs/mod_track.so $(INSTALL_ROOT)/usr/lib64/httpd/modules
	cp -v mod_track.conf $(INSTALL_ROOT)/etc/httpd/conf.d/

