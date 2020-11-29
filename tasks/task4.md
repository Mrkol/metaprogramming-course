Задача 4. Annotation processing
========================

## Предыстория

Имея возможность интроспекции, можно обощить решения многих задач. Boost.PFR умеет генерировать для поддеживаемых структур специализацию `std::hash`, операторы ввода и вывода в поток и операторы сравнения. Другие варианты применения: бинарная сериализация, ORM.

Иногда может быть полезно сообщить функции, использующей интроспекцию, какую-то дополнительную информацию о конкретном поле. Например, указать название колонки в базе данных или исключить поле из рассмотрения в операторах сравнения. В экосистеме Java для передачи такой контекстной информации используются аннотации. Мы попробуем реализовать поддержку рефлекшна с аннотациями в C++.

В C++ есть два встроенных способа передать контекстную информацию: директивы препроцессора и аттрибуты. Ни тот, ни другой нам не подходят, так как их использование потребовало бы разработки внешних инструментов, выходящих за рамки языка. Вместо этого для аннотирования полей мы будем использовать другие поля:

```cpp
struct S {
    int x; // no annotation
    
    Annotate<Transient> _annot1;
    char y; // annotated with Transient
    
    Annotate<Transient> _annot2;
    Annotate<NoCompare> _annot3;
    float z; // annotated with Transient, NoCompare
    
    Annotate<Transient, NoCompare> _annot4;
    float w; // annotated with Transient, NoCompare
};
```

У аннотаций могут быть аргументы. Мы будем передавать их как типовые параметры шаблона:

```cpp
struct MySerializableStruct {
    int x;

    Annotate<Checksum<Crc32>> _annot1;
    uint32_t checksum; // annotated with Checksum with parameter Crc32
};
```

## Подготовка

Вспомните, как выполняются различные операции над списками типов. Самые базовые из них [реализованы в стандартной библиотеке](https://en.cppreference.com/w/cpp/header/tuple) для `std::tuple`.

Вспомните последние две пары про рефлекшн в C++.

Вспомните, как использовать [template template parameters](https://en.cppreference.com/w/cpp/language/template_parameters).

## Задача

Рассмотрим класс-шаблон `Annotate`:

```cpp
template <class...>
class Annotate {};
```

Будем считать, что все поля типа `Annotate` описывают аннотации к ближайшему следующему полю, имеющему тип, отличный от `Annotate`.

Реализуйте шаблон `Describe`:

```cpp
template <class T>
struct Describe {
    static constexpr size_t num_fields = /* number of fields */
    
    template <size_t I>
    using Field = /* field descriptor (see below) */;
};
```

`num_fields` &mdash; число полей структуры `T` без учёта полей типа `Annotate`.

Для каждого `I` от 0 до `num_fields - 1` `Field<I>` &mdash; тип, описывающий I-е поле в порядке объявления в структуре `T` без учёта полей-аннотаций, со следующими членами:

```cpp
struct /* unspecified (field descriptor) */ {
    using Type = /* type of field */;
    using Annotations = Annotate</* all annotations of the field */>;

    template <template <class...> class AnnotationTemplate>
    static constexpr bool has_annotation_template = /* see below */;
    
    template <class Annotation>
    static constexpr bool has_annotation_class = /* see below */;

    template <template <class...> class AnnotationTemplate>
    using FindAnnotation = /* see below */;
};
```

`has_annotation_template<AnnotationTemplate>` истинен, когда среди аннотаций поля есть `AnnotationTemplate` с произвольными аргументами шаблона.

`has_annotation_class<Annotation>` истинен, когда среди аннотаций поля есть `Annotation`.

`FindAnnotation<AnnotationTemplate>` &mdash; инстанс `AnnotationTemplate` с теми параметрами, с которыми он встречается в списке аннотаций поля. Если `AnnotationTemplate` встречается несколько раз, `FindAnnotation` &mdash; любой из подходящиз инстансов. Если среди аннотаций нет `AnnotationTemplate`, использование `FindAnnotation<AnnotationTemplate>` должно приводить к substitution failure.

Для определения количества и типов полей используйте любую подходящую технику. Реализацию можно подсмотреть в [презентации](https://docs.google.com/presentation/d/1DHkmbyjYLiU8-Qgp5NclhDNPcb5fViN29fEWZx7YFoI/edit?usp=sharing) или в [Boost.PFR](https://github.com/apolukhin/magic_get). (Только не копипастите из Boost.PFR, пожалуйста. Там как минимум code style не совместим с нашим.) Получать значения полей или ссылки на них в данной задаче не требуется.

### Ограничения на поддерживаемые структуры

В качестве шаблонного аргумента `Describe` могут использоваться только классы, удовлетворяющее слелующим условиям:

- не юнионы,
- нет пользовательских конструкторов и деструкторов,
- нет виртуальных методов,
- нет базовых классов,
- все поля публичны, не имеют значений по умолчанию и их типы
  - либо скалярны,
  - либо соответствуют этим требованиям.

В противном случае поведение не определено.

Если последнее поле структуры имеет тип `Annotate`, поведение не определено.

## Пример

```cpp
// some macro magic
// to avoid handmade names for annotation fields

#define mpg_concat(a,b)  a##b
#define mpg_annotation_label(a) mpg_concat(_annotion_, a)
#define mpg_annotate(...) Annotate<__VA_ARGS__> mpg_annotation_label(__COUNTER__);

// mpg_annotate(A, B, C) expands to
// `Annotate<A, B, C> some_ugly_unique_name`

struct Transient;
struct Crc32;
struct NoIo;
struct NoCompare;

template <class Algorithm>
struct Checksum;

struct S {
    int x; // no annotation

    mpg_annotate(Transient, NoIo)
    char y; // annotated with Transient

    mpg_annotate(Transient)
    mpg_annotate(NoCompare)
    float z; // annotated with Transient, NoCompare

    mpg_annotate(NoIo, Checksum<Crc32>)
    uint64_t w; // annotated with Transient, NoCompare
};


static_assert(std::is_same_v<Describe<S>::Field<0>::Type, int>);
static_assert(std::is_same_v<Describe<S>::Field<0>::Annotations, Annotate<>>);


static_assert(Describe<S>::Field<1>::has_annotation_class<NoCompare> == false);
static_assert(Describe<S>::Field<1>::has_annotation_template<Checksum> == false);
static_assert(Describe<S>::Field<1>::has_annotation_class<NoIo> == true);

static_assert(std::is_same_v<Describe<S>::Field<2>::Annotations, Annotate<Transient, NoCompare>>);

static_assert(Describe<S>::Field<3>::has_annotation_template<Checksum> == true);
static_assert(std::is_same_v<Describe<S>::Field<3>::FindAnnotation<Checksum>, Checksum<Crc32>>);
```

## Формальности

**Дедлайн:** 09:00 14.12.2020.

**Баллы:** 3 уе.

Классы `Annotate`, `Descriptor` должны быть доступны в глобальном неймспейсе при подключении заголовочного файла `reflect.hpp`. Этот заголовочный файл должен находиться в папке `task4`, расположенной в корне репозитория.

Код пушьте в ветку `task4` и делайте pull request в `master`.

Используйте всю мощь стандартной библиотеки.
