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

For example, the AE case with `delta = -1` (submodular) and `100` locations would be `./ae 100 0 0`
