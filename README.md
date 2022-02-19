# Redesigned Octo Computing Machine
## An implementation of the Pandos+ operating system

### Dependences:
1. [umps3](https://github.com/virtualsquare/umps3)
2. __cmake-format__ for formatting the code

### Usage:

Build kernel and create disk image with:
```
make all
```
or just simply:
```
make
```
_Note:_ Generated outputs file are stored in  __output__ dir.

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







