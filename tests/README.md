Тесты
======

## Сборка и запуск

```sh
git clone https://github.com/raid-7/mipt-metaprogramming-2020.git
cd mipt-metaprogramming-2020/tests
mkdir build
cd build

cmake -DREPOSITORY_PATH=/path/to/your/solutions -DTASK=1 ..
cmake --build .
ctest --verbose
```

## Nocompile тесты

К некоторым задачам также могут быть тесты, которые не должны компилироваться. Чтобы попытаться их "собрать", добавьте опцию `-DNOCOMPILE=ON`. Обычные тесты при этом собираться не будут.

```sh
cmake -DREPOSITORY_PATH=/path/to/your/solutions -DTASK=1 -DNOCOMPILE=ON ..
cmake --build .
```

Если nocompile тестов для задачи нет, опция будет проигнорирована.

