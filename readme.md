# Mastering C


## Tasks

* Allocate and fill struct with string members.
* Allocate and fill struct with array member.
* Serialize struct with string, enum, and array members.


## Requirements

* `libjansson-devel` (`libjansson-dev` in Debian) library package.


## Building and running tests

```
gcc structs.c -o structs.test && ./structs.test
gcc structs-json.c -o structs-json.test -ljansson && ./structs-json.test
```


## Development

The `clang-format` tool is used for formatting with the "Google" style option.
* https://clang.llvm.org/docs/ClangFormat.html
* https://clang.llvm.org/docs/ClangFormatStyleOptions.html

```
clang-format -style=Google -i *.c *.h
```
