Задача 1. Звуки природы
========================

## Подготовка

С C++ 20 тип нетиповых шаблонных параметров может быть классом. Раньше в шаблонах можно было передавать только интегральные типы, указатели, ссылки и enum'ы. То есть можно писать так:

```c++
struct Data {
    int a;
    char b;
};

template <Data input>
class StaticDataHolder {};

StaticDataHolder<{5, 'c'}> holder;
```

Конечно, есть [куча ограничений](https://en.cppreference.com/w/cpp/language/template_parameters#Non-type_template_parameter). Эта фича поддерживается в GCC с 9-й версии. Нам она пригодится.

Вспомните, как определяются [пользовательские операторы](https://en.cppreference.com/w/cpp/language/user_literal) для строковых литералов.

Вспомните, как работает `dynamic_cast`.

Вспомните, как использовать [variadic templates](https://en.cppreference.com/w/cpp/language/parameter_pack).

**Убедитесь, что вы используете GCC 10.**

## Задача

### Отображение полиморфных типов в объекты

Пусть у нас есть полиморфная иерархия классов, наследующихся от `Base`. Каждому классу в этой иерархии поставлено в соответствие некоторое значение типа `Target`. Получив ссылку `Base& object`, мы хотим найти значение, соответствующее тому типу, которым является `object`.

Предлагается создать обобщённый механизм, позволяющий задавать такие отображения.

Вам нужно реализовать классы `Mapping` и `ClassMapper`, а также статический метод `ClassMapper::map`. Ниже приведены списки шаблонных параметров и сигнатуры:

```c++
template <class From, auto target>
struct Mapping;

template <class Base, class Target, class... Mappings>
struct ClassMapper {
	static std::optional<Target> map(const Base& object);
};
```

При вызове `map(object)` маппер должен искать среди `Mappings...` первый `Mapping<From, target>`, такой что реальный тип `object` &mdash; это From или наследник `From`, и возвращать `target`. Если такой маппинг не найден, возвращается `std::nullopt`.

Код **не** должен компилироваться, если при инстанцировании `ClassMapper<Base, Target, Mappings...>::map` хотя бы один тип из пака `Mappings...` это не `Mapping<SomeClass, Target some_value>`, где `SomeClass` наследуется от `Base`. Скорее всего, у вас это получится естественным образом.

Гарантируется, что при инстанцировании маппинги передаются отсортированными в порядке наследования: наследники идут раньше суперклассов. В противном случае поведение не определено.

Если ничего не поняли, прочитайте следующий раздел, а потом пример.

### Строки в compile time

1. Реализуйте шаблонный класс `String` с одним нетиповым шаблонным параметром `size_t max_length`, конструктором от двух аргументов `const char* string, size_t length` и неявным оператором каста к `std::string_view`. Класс должен хранить первые `length` символов `string` и возвращать их при касте к `string_view`. Если `length > max_length`, поведение не определено.

2. Реализуйте оператор `""_cstr`, возвращающий String<256>, содержащий литерал. Если длина литерала больше 256, поведение не определено.

3. Сделайте так, чтобы `String` и `""_cstr` можно было использовать в compile time.
4. Сделайте так, чтобы объекты типа `String` можно было передавать как нетиповые шаблонные параметры.

```c++
static_assert(std::is_literal_type_v<String<256>>);
static_assert(std::is_same_v<String<256>, decltype("smth"_cstr)>);
static_assert("some text"_cstr == "some text"sv);
static_assert(String<128>{"some text", 4} == "some"sv);
```

Эти ассерты точно должны проходить. Но их недостаточно, чтобы класс можно было использовать в качестве типа нетиповых шаблонных параметров. См. ссылку выше.

## Пример

Представим, что у нас есть иерархия классов, представляющих животных. Нам приходит указатель на какое-то животное, и нужно понять, какой звук оно издаёт. Для этого я хочу написать следующий код.

```cpp
class Animal {
public:
    virtual ~Animal() = default;
};

class Cat : public Animal {};
class Cow : public Animal {};
class Dog : public Animal {};

using MyMapper = ClassMapper<
    Animal, String<256>,
    Mapping<Cat, "Meow"_cstr>,
    Mapping<Dog, "Bark"_cstr>,
    Mapping<Cow, "Moo"_cstr>
>;


std::shared_ptr<Animal> some_animal{new Dog()};
std::string_view dog_sound = *MyMapper::map(*some_animal);
ASSERT_EQ(dog_sound, "Bark");
```

Также посмотрите [тесты](https://github.com/Mrkol/mipt-metaprogramming-2021/blob/master/tests/task1/main.cpp).

## Формальности

**Дедлайн:** 04:00 10.10.2021.

**Баллы:** 3 уе.

`ClassMapper`, `Mapping`, `String`, `""_cstr` должны быть доступны в глобальном неймспейсе при подключении заголовочного файла `string_mapper.hpp`. Этот заголовочный файл должен находиться в папке `task1`, расположенной в свою очередь в корне репозитория.

Код пушьте в ветку `task1` и делайте pull request в `master`.

Используйте всю мощь стандартной библиотеки, включая type traits и библиотеку концептов, если потребуется.

## Предыстория

Я столкнулся с подобной задачей, когда было нужно преобразовывать C++ исключения в исключения Java и выбрасывать их через JNI. То есть имеется достаточно большая иерархия C++-исключений, исключение ловится по ссылке на объект базового класса, а дальше надо получить соответствующее имя Java-класса.

До C++ 20 такое можно было реализовать, используя [GNU расширение оператора ""](https://habr.com/ru/post/243581/).
