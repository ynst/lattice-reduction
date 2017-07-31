# lattice-reduction
**The program runs in Linux/Unix environments (includes Macbooks)**

## How to compile the `ae` executable
Open the Terminal and clone the repository by using the following commands:

```shell
git clone https://github.com/ynst/lattice-reduction.git 
cd lattice-reduction
```

then compile the project using the command 
```shell
make
```

## Using `ae`

The program takes three inputs: the number of locations, a 0-2 number showing which algorithm is being run and a binary number that indicates whether the function is supermodular or submodular.

```shell
./ae [number of firms] [0:AE / 1:EAE / 2:brute force] [0/1]
```

`0` represents supermodular and `1` represents submodular.

For example, the AE case with `delta = -1` (submodular) and `100` locations would be `./ae 100 0 1`

## Changing the profit function

A profit function should be a C++ file that includes a function named `profit` that takes in a vector. The size of the vector should equal the number of facilities. The function should return a double number that represents the profit associated with the vector. 

Once you have this file that includes the profit function, place your file in the project folder and add its filename to the Makefile by changing the following line of the Makefile. 

```shell
SOURCES = utils.cpp jia.cpp ae.cpp test.cpp 
```

You should simply add the name of your file to this line and remove jia.cpp, which is the file that includes the current profit function.

If you have any questions, please do not hesitate to message/email me.
