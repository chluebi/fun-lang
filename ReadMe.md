Toy compiler of a functional-ish language with no heap allocation.
In a proper clang environment, run this to run the POC:

```bash
bash build.sh && ./build/compiler test.lang test.ll && clang -Wno-unused-command-line-argument -Woverride-module test.ll -o test && ./test
```