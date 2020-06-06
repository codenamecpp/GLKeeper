CPUS := $(shell nproc)

all: build_debug
	true

build_debug: premake
	.build/premake5 gmake --cc=clang
	make -C .build config=debug_x86_64 -j$(CPUS)
	test -d bin || mkdir bin
	cp .build/bin/x86_64/Debug/glkeeper bin/glkeeper-debug

build_release: premake
	.build/premake5 gmake --cc=clang
	make -C .build config=release_x86_64 -j$(CPUS)
	test -d bin || mkdir bin
	cp .build/bin/x86_64/Release/glkeeper bin/glkeeper-release

clean:
	.build/premake5 gmake --cc=clang
	make -C third_party/Box2D/Build clean
	make -C .build clean

run:
	./bin/glkeeper-debug -gamedir ../games/dk2

builddir: 
	test -d .build || mkdir .build

premake: builddir
	test -e .build/premake5 || (cd .build && \
	wget https://github.com/premake/premake-core/releases/download/v5.0.0-alpha14/premake-5.0.0-alpha14-linux.tar.gz && \
	tar xzf premake-5.0.0-alpha14-linux.tar.gz && \
	rm premake-5.0.0-alpha14-linux.tar.gz)
