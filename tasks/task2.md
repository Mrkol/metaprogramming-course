Задача 2. Бесконечная лень
========================

## Подготовка

На семинаре мы поигрались с тайплистами. Вспомним, что есть несколько способов их определить. В данной задаче мы разовьём "старый" способ при помощи современных технологий.

Вспомните, как работает инстанциация для классов: инстанциация объявления, инстанциация определения, инстанциация их членов.

Вспомните, что такое констрейнты и концепты, чем может являться концепт с философской точки зрения, как констрейнты взаимодействуют с специализациями и перегрузками.

Вспомните, что в дочерний класс можно вытаскивать различные сущности из родительского при помощи `using BaseClassName::EntityName;` (работает для функций, полей и *алиасов*).

Вспомните, что специализировать шаблоны классов и перегружать функции можно при помощи *констрейнтов*. Однако имейте ввиду, что возможность компиляторов видеть импликации между наборами констрейнтов пока что достаточно ограничена и иногда приходится повоевать, чтобы добиться желаемого результата.

**Убедитесь, что вы используете Clang 12+**

## Задача

### Работа с бесконечными списками типов

Рассмотрим следующий концепт.

```c++

template<class TL>
concept TypeSequence =
    requires {
        typename TL::Head;
        typename TL::Tail;
    };
```

Утверждается, что он описывает *бесконечные последовательности типов*. От `TL::Tail` также неявно требуется удовлетворять концепту `TypeSequence`, однако смоделировать это на C++20 скорее всего не возможно.

Но как последовательность типов может быть бесконечной, если памяти в компиляторе конечно?

Ответ такой же, как и в функциональных языках: ленивые вычисления. Компилятор лениво инстанцирует члены шаблонов классов, в том числе и алиасы вроде Tail, а значит они могут ссылаться даже на текущий шаблон но с другими аргументами.

Пример:
```c++
template<class T>
struct FunnyStarrySequence {
    using Head = T;
    using Tail = FunnyStarrySequence<T*>;
};
```
Эта инстанциации этого шаблона описывают бесконечные списки списки вида `(T, T*, T**, T***, T****, ...)`. Однако весь список в памяти компилятора не хранится, хранится лишь схема получения следующих элементов. Сколько раз возьмёте следующий элемент -- столько и будет вычислено.

Но с одними лишь бесконечными последовательностями типов работать неудобно, поэтому рассмотрим следующий код.

```c++

struct Nil {};

template<class TL>
concept Empty = std::derived_from<TL, Nil>;

template<class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;
```

Мы ввели концепт `Empty`, описывающий пустой список как произвольного наследника `Nil`, а также `TypeList`, описывающий *потенциально бесконечный список* как либо пустой, либо последовательность. Отметим, что в этом случае мы собрались нарушать неявно требования от `TL::Tail` являться последовательностью и заменяем его на требование являться списком типов. Более правильным подходом было бы определить `TypeList` независимо от TypeSequence и рассматривать их как независимые сущности. Однако выбранный подход позволяет компилятору догадаться, что любая последовательность также является списком, что может пригодиться на практике при перегрузке функций.

Итак, ваша задача -- реализовать набор интересных операций для работы с бесконечными списками, в процессе догадавшись как вообще реализовывать конкретные типы тайплистов.

### "Новые" тайплисты

Отныне их мы будем называть тайп-тюплами (type tuple), чтобы лучше их отличать от потенциально бесконечных листов. В этой задаче тюплы нам тоже понадобятся в некоторых местах, однако сильно заумных алгоритмов на них мы писать не будем. Напомним, что определяются они как
```c++
template<class... Ts>
struct TTuple {};
```
Не забудьте, что все различные тюплы можно объединить в одно семейство с помощью концепта `TypeTuple`. Однако определить его так же, как мы это делали на семинаре, не выйдет, так как clang 12 всё ещё не поддерживает лямбды внутри невычисляемых контекстов. Используйте традиционные трюки со специализациями, либо обходитесь обычными функциями вместо лямбд.

### Часть 1: операции

Следующие функции должны находится в неймспейсе `TypeLists`, как и объявления концептов `Empty`, `TypeSequence`, `TypeList`. Сущности `TTuple` и `TypeTuple` же положите в неймспейс `TypeTuples`.

 * `Cons<T, TL>` -- список, в начале которого стоит `T`, а дальше -- элементы списка `TL`.
 * `FromTuple<TT>`/`ToTuple<TL>` -- функции для конвертации между **конечными** списками и тюплами. Помогут вам при дебаге.
 * `Repeat<T>` -- бесконечный список из `T`.
 * `Take<N, TL>` -- первые `N` элементов потенциально бесконечного списка `TL`.
 * `Drop<N, TL>` -- всё кроме первых `N` элементов списка `TL`.
 * `Replicate<N, T>` -- список из `N` элементов равных `T`.
 * `Map<F, TL>` -- список из результатов применения `F` к элементам `TL`.
 * `Filter<P, TL>` -- список лишь тех элементов `TL`, что удовлетворяют `P<_>::Value`. Относительный порядок элементов не должен меняться.
 * `Iterate<F, T>` -- список, в котором каждый следующий элемент является результатом применения `F` к предыдущему, а первый -- `T`.
 * `Cycle<TL>` -- бесконечный список, в котором раз за разом повторяется конечный список `TL`.
 * `Inits<TL>` -- список всех префиксов `TL` в порядке возрастания длины.
 * `Tails<TL>` -- список всех суффиксов `TL` в порядке возрастания длины их дополнения до всего списка.
 * `Scanl<OP, T, TL>` -- последовательность, в которой первый элемент -- `T`, а каждый последующий получается путём применения `OP<_, _>::Type` к текущему и следующему элементу `TL`.
 * `Foldl<OP, T, TL>` -- тип, получаемый как `OP<... OP<OP<T, TL[0]>, TL[1]> ... >`. Если последовательность бесконечная, значение не определено.
 * `Zip2<L, R>` -- список пар из i-ых элементов списков `L` и `R` соответственно, идущих подряд.
 * `Zip<TL...>` -- список тюплов по одному элементу фиксированного номера из каждого списка.
 * `GroupBy<EQ,TL>` -- список из списков **подряд** идущих элементов `TL`,  "равных" **последовательно**, т.е. каждый следующий элемент должен быть равен текущему (это позволяет, например, искать возрастающие подпоследовательности). Равенство подразумевается в смысле `EQ<T, S>::Value == true`. Например, групбай букв в слове "Mississippi" по равенству -- `["M","i","ss","i","ss","i","pp","i"]`. Если все элементы *бесконечной* последовательности `TL` равны, поведение не определено (подумайте, что нам мешает его определить).

### Часть 2: числовые последовательности

Содержимое этого раздела поместите в глобальный неймспейс.

Введём новый класс:
```c++
template<auto V>
struct ValueTag{ static constexpr auto Value = V; };
```
С его помощью мы можем хранить значения внутри типов, что даёт нам возможность работать с бесконечными списками значений в компайл тайме. Вычислите следующие типы:

 * `Nats` -- натуральные числа `(0, 1, 2, 3, ...)`
 * `Fib` -- числа Фибоначчи `(0, 1, 1, 2, ...)`
 * `Primes` -- простые числа `(2, 3, 5, 7, ...)`

Также для удобства заведите шаблон для генерации списка значений из пака параметров:
```c++
template<class T, T... ts>
using VTuple = TTuple<ValueTag<ts>...>;
```

По возможности используйте уже реализованные функции.

## Примеры и тесты

В общем случае сравнение на равенство бесконечных последовательностей сводится к решению проблемы останова, поэтому для дебага действуйте аналогично [тестам](https://github.com/Mrkol/mipt-metaprogramming-2021/blob/master/tests/task2/main.cpp): отрезайте какой-то кусок бесконечного списка, конвертируйте в тюпл и сравнивайте их через `std::is_same`. Если внутри списка есть другие списки, обрезание и конвертацию необходимо делать рекурсивно при помощи `Map` и каррированной версии `Take`.

## Формальности

**Дедлайн:** 04:00 17.10.2021.

**Баллы:** 4 уе.

Все упомянутые сущности должны лежать в файле `type_lists.hpp` в соответствующих неймспейсах. Этот заголовочный файл должен находиться в папке `task2`, расположенной в свою очередь в корне репозитория.

Код пушьте в ветку `task2` и делайте pull request в `master`. Не забывайте ставить меня в ревьюверы.

Обязательное требование -- по-максимуму проставлять констрейнты шаблонным аргументам, и, если это целесообразно, выделять наборы констрейнтов в концепты.

## Предыстория

Помните, мы с вами смотрели справку по haskell, и, прокручивая функции для бесконечных списков, я говорил "это всё не то"? Оказывается, то! В результате нескольких дней экспериментов было выяснено, что бесконечные списки таки можно реализовать, и выглядит оно очень даже ничего. Конечно полезность данного "изобретения" под вопросом, однако для тренировки самое то.
