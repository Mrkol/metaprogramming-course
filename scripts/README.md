Автоматическая сборка
======================

```sh
test.sh repo task # build and run ordinary tests
test.sh -n repo task # build nocompile tests
test.sh -i folder repo task # build tests in specific subfolder of solutions directory

all.sh task # build and run tests for all repositories from students.txt
all.sh task -n # build nocompile tests for all repositories from students.txt
```

