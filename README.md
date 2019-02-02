#CS 4760 Assignment #1
This README has been written using markdown language.
 
#Usage

Within the project directory use make to compile the source files.
```bash
make
```
The program will create an executable file called **syscall**. To execute thexecutable, refer to the examples below.

```bash
./syscall  -h   #Will display all possible options and their descriptions
```

If the arguments are missing or partially missing. The program will automatically use the default input file name [**input.dat**] and output file name [**output.dat**].

#Clean

The makefile also includes a clean function which removes the executable, objectfiles, and only the default **output.dat** file if it exists.

```bash
make clean
```

#Version Log

Github was used as version control for this assignment. The link to this assignments version control is [Github](https://github.com/veebzz/OperatingSystems/commits/master).

#Existing Problems

* The program throws a huge error when the dynamic memory is released with the free function. Though it displays big error text, the program still finishes its job.
* The command line executable does not accept multiple option arguments such as:
```bash
./syscall -io <argument> <argument>

or

./syscall -oi <argument> <argument>
```

