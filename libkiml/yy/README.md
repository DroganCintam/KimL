### The lexical and syntactic analysers

The file `kiml.l` defines the lexer. The file `kiml.yy` defines the parser.

To translate these files into C++ files, use bison and Flex.

#### On Linux
```
bison -l -okiml.parser.cpp kiml.yy
flex -L -okiml.lexer.cpp kiml.l
mv *.cpp ../src
mv *.h ../include/kiml
```

#### On Windows
```
bison -l -okiml.parser.cpp kiml.yy
flex -L -okiml.lexer.cpp kiml.l
move *.cpp ../src
move *.h ../include/kiml
```
