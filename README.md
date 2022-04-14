# Really Optimistic Computing Machine
## An implementation of the Pandos+ operating system

### Dependences:
1. [umps3](https://github.com/virtualsquare/umps3)
2. __cmake-format__ to format code
3. __doxygen__ to generate documentation

### Usage:

Build kernel, create disk image and documentation with:
```
make all
```
or just simply:
```
make
```
_Note:_ Generated outputs file are stored in  __output__ dir.
_Note:_ Generated documentation file are stored in  __doc__ dir.

You must then create a machine in the umps3 emulator, using generated files in 
your configuration.

### Other commands:

Build kernel with:
```
make kernel
```

Clean the project with:
```
make clean
```

Format code with:
```
make format
```

Generate documentation with:
```
make docs 
```








