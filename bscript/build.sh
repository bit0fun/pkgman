#/bin/bash


# This is a generic package build script 

#Package Name
PKGNAME=

#Version Number
VERSION=0.0.0

#Package Archive
PKGAR=$PKGNAME-$VERSION.tar.gz

#Package hash
PKGHASH=$PKGAR.sha512

#Web location
URL_DOMAIN=
URL_AR=$URL_DOMAIN/$PKGAR
URL_HASH=$URL_DOMAIN/$PKGHASH


SRCDIR= /usr/src
ARDIR= /usr/src/ar
BUILDDIR=/usr/src/build
SCRIPTDIR=/usr/src/script
INSTALLDIR= /usr


SRC= $SRCDIR/$PKGNAME
ARCHIVE= $ARDIR/$PKGAR
BUILD= $BUILDDIR/$PKGNAME-$VERSION

# Download package source files
download(){
	if [ $(id -u) -ne 0 ]; then
		echo "This must be run as root or package maintainer"
	fi	

	# Download archive file
	curl -L -f -o $ARCHIVE $URL_AR 

	# Download hash file
	curl -L -f -o /tmp/$PKGHASH $URL_HASH

}

# Setup package for installing 
setup(){

	if [ $(id -u) -ne 0 ]; then
		echo "This must be run as root or package maintainer"
	fi	

	# Check that archive is present
	if [ ! -f $ARCHIVE ]; then
		#archive not found, something is wrong
		echo "Could not find $PKGAR, exiting"
		exit -1
	fi

	# Make directory for source file
	mkdir -p $SRC

	# Check hash of archive to ensure integrity
	if [ sha512sum -c --status $ARDIR/$PKGHASH ]; then
		echo "Invalid hash for package: $PKGNAME, $PKGHASH"
		exit -1
	fi

	# Extract archive from archive directory, put into source directory
	tar -xf $ARDIR/$PKGAR -C $SRCDIR/$PKGNAME


}

# Build package. Assuming traditional build
build(){
	if [ $(id -u) -ne 0 ]; then
		echo "This must be run as root or package maintainer"
	fi	

	mkdir -p $BUILD

	cd $BUILD

	$SRC/configure --prefix $INSTALLDIR

	# Make in parallel, leave 1 core over to make sure some resource 
	# constrained systems don't freak out
	make -j $(nproc --ignore=1)


}

# Install package
install(){
	if [ $(id -u) -ne 0 ]; then
		echo "This must be run as root or package maintainer"
	fi	
	cd $BUILD
	make install

	# Signal that install has completed
	touch /tmp/$PKGNAME
	echo "1" > /tmp/$PKGNAME
}


