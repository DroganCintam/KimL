### The lexical and syntactic analysers

The file `simpl.l` defines the lexer. The file `simpl.yy` defines the parser.

To translate these files into C++ files, use bison and Flex.

#### On Linux
```
bison -l -osimpl.parser.cpp simpl.yy
flex -L -osimpl.lexer.cpp simpl.l
mv *.cpp ../src
mv *.h ../include/simpl
```

#### On Windows
```
bison -l -osimpl.parser.cpp simpl.yy
flex -L -osimpl.lexer.cpp simpl.l
move *.cpp ../src
move *.h ../include/simpl
```
