# scas

Assembler and linker for z80.

## Status

Nearly done. We'd like to get relative labels working so it can compile the
KnightOS kernel, and then there are a few more bugs that need to be sorted out.
Should be usable now.

## Compiling from Source

Compiling under UNIX and Cygwin environments:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    sudo make install

Compiling on windows is recommended with [MSYS2](https://msys2.github.io/),
but will probably work with MinGW as well:

    mkdir build
    cd build
    cmake -G 'MSYS Makefiles' -DCMAKE_BUILD_TYPE=Release ..
    make
    make install

Don't forget to run the MSYS terminal as admin, or install under 
MSYS2 binaries with `-DCMAKE_INSTALL_PREFIX=/mingw64`.

Now read `man scas` to learn how to use it.

## Help, Bugs, Feedback

If you need help with KnightOS, want to keep up with progress, chat with
developers, or ask any other questions about KnightOS, you can hang out in the
IRC channel: [#knightos on irc.freenode.net](http://webchat.freenode.net/?channels=knightos).
 
To report bugs, please create [a GitHub issue](https://github.com/KnightOS/KnightOS/issues/new) or contact us on IRC.
 
If you'd like to contribute to the project, please see the [contribution guidelines](http://www.knightos.org/contributing).
