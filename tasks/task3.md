Задача 3. Очень умный неуказатель
========================

## Подготовка

Освежите в памяти [использование концептов и ограничений](https://en.cppreference.com/w/cpp/language/constraints) и [объектные концепты стандартной библиотеки](https://en.cppreference.com/w/cpp/concepts).

Вспомните пары про [умные указатели](https://youtu.be/UFYCImRpsDw) и [стирание типов](https://youtu.be/XdMH2yF3qOE).

## Задача

Реализуйте шаблонный класс, оборачивающий произвольный объект и логирующий обращения к нему.

```c++
template <class T>
class Spy {
public:
    explicit Spy(T);

    T& operator *();
    const T& operator *() const;

    /* unspecified */ operator ->();

    /*
     * if needed (see below):
     *   default constructor
     *   copy and move construction
     *   copy and move assignment
     *   comparisons
     *   destructor
    */

    template <std::invocable<unsigned int> Logger> requires /* see below */
    void setLogger(Logger&& logger);
};
```

Оборачиваемый объект необходимо хранить по значению. На паре справедливо заметили, что называть такие сущности указателями неправильно, поэтому мы будем называть их умными неуказателями.

Если `s` &mdash; значение типа `Spy<T>`,  выражение `s->member` должно приводить к обращению к нестатическому члену `member` оборачиваемого объекта.

#### Логгеры

Метод `setLogger` устанавливает логгер. После вычисления каждого выражения, содержащего обращения к оборачиваемому объекту через `operator ->`, должен вызываться логгер, если он установлен. В качестве аргумента логгер принимает количество обращений к объекту при вычислении выражения.

Обращения через `operator *` не логгируются.

Если оборачиваемый тип `T` не копируемый, то `Spy<T>` должен поддерживать move-only логгеры.

Если в одном выражении происходит обращение к оборачиваемому объекту и изменяется логгер, поведение не определено.

Использовать `std::function` в этом задании запрещено.

Поддержка small buffer optimization для логгеров: +1 у.е.

#### Сохранение концептов

Будем говорить, что умный неуказатель `W` сохраняет концепт `C`, если для любого типа `T`

1) `T` удовлетворяет `C` &rArr; `W<T>` удовлетворяет `C`,
2) `T` моделирует `C` &rArr; `W<T>` моделирует `C`.

Ваш `Spy` должен сохранять основные объектные концепты: `std::movable`, `std::copyable`, `std::semiregular`, `std::regular`. Для этого можно накладывать дополнительные ограничения на шаблонный аргумент метода `setLogger`.

Операторы сравнения должны сравнивать оборачиваемые объекты, игнорируя логгер. При копировании (перемещении) должны копироваться (перемещаться) и логгер, и оборачиваемый объект.

## Пример

```c++
struct Holder {
    int x = 0;
    bool isPositive() const {
        return x > 0;
    }
};

Spy s{Holder{}};
static_assert(std::semiregular<decltype(s)>);

s.setLogger([](auto n) { std::cout << n << std::endl; });

s->isPositive() && s->x--; // prints 1
s->x++ + s->x++; // prints 2
s->isPositive() && s->x--; // prints 2

s.setLogger([dummy = std::unique_ptr<int>()](auto n) {}); // compilation error

// -----------------------------------

struct MoveOnly {
    MoveOnly() = default;

    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator =(MoveOnly&&) = default;

    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator =(const MoveOnly&) = delete;

    ~MoveOnly() noexcept = default;
};

Spy t{MoveOnly{}};

s.setLogger([dummy = std::unique_ptr<int>()](auto n) {}); // ok

static_assert(std::movable<decltype(t)>);
static_assert(!std::copyable<decltype(t)>);
```

## Вопросы для размышлений

1. С какими проблемами мы бы столкнулись, если бы захотели добавить константную версию оператора `->`?

2. Как бы вы модифицировали класс `Spy`, чтобы поддержать обращения к оборачиваемому объекту из нескольких потоков?

## Формальности

**Дедлайн:** 09:00 30.11.2020.

**Баллы:** 4 (+1) уе.

Класс `Spy` должен быть доступен в глобальном неймспейсе при подключении заголовочного файла `spy.hpp`. Этот заголовочный файл должен находиться в папке `task3`, расположенной в корне репозитория.

Код пушьте в ветку `task3` и делайте pull request в `master`.

Используйте всю мощь стандартной библиотеки, кроме `std::function`.

Отвечать на вопросы для размышлений необязательно.