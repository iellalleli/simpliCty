# TODO
- Comments (need to fix. The text after or between their syntax isn't accepted)

- Constants (need to identify integers, floats, strings, chars. Ex. 10, 10.23, "hello" (ignores " and takes the value inside them), 'a') DONE but need to handle cases for "hello"" where there is a missing right parenthesis

- Clean up


# HOW TO RUN
- Compile program using 
```
gcc simpliCty/lexers.c simpliCty/main.c -o main.exe
```
- Run program with the sample files using .cty extension 
```
./main.exe samples/new.cty
```
- Output will display in terminal and logged at output/symbol_table.txt