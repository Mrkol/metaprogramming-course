Метапрограммирование на C++
============================

[Презентации](https://drive.google.com/drive/folders/1JUSjHeZMyicojhJ7ebN3aStnU7VxqZEq?usp=sharing)

[Задания](https://github.com/Mrkol/mipt-metaprogramming-2021/tree/master/tasks)

[Code style](https://github.com/Mrkol/mipt-metaprogramming-2021/blob/master/codestyle.md)

## Темы (WIP)
Темы идут в произвольном порядке и не мапятся 1 к 1 на занятия. Более того, скорее всего осветить все из них за один семестр не удастся. Программа продумана не до конца и будет уточняться по ходу дела.

1. Templates. Templateable entities, kinds, metafunctions
2. Template debugging. Dependent false assertion, tracers, archetypes
3. Name resolution, two-phase lookup, dependent names 
4. Type deduction. Template arguments, auto, deduction guides
5. Perfect forwarding. Unversal references, argument forwarding, result forwarding, decltype(auto)
6. Friends, Barton–Nackman trick
7. Concepts, requirements
8. Stateful metaprogramming with friends
9. Reflection tricks. std::source_location, enum reflection, struct reflection
10. Type lists
11. Traits and policies
12. Policy based design, Mixins, EBCO
13. Polymorphism. VTables, templates, CRTP, type erasure
14. Multimethods
15. Named template arguments trick
16. Hierarchy generation
17. Tuples and variants
18. OO patterns revisited: factory, abstract factory, prototype, visitor, command, observer, strategy...
19. Niebloids
20. Tag invoke
21. Ranges

## Полезные ссылки

[refactoring.guru](http://refactoring.guru/) &mdash; сайт про паттерны и рефакторинг

[Concepts and constraints](https://en.cppreference.com/w/cpp/language/constraints) &mdash; cppreference

[Концепты: упрощаем реализацию классов std utility](https://youtu.be/udTEfwCkmaw) &mdash; доклад Андрея Давыдова

[Метапрограммирование, щадящее компилятор](https://www.youtube.com/watch?v=udTEfwCkmaw) &mdash; доклад Андрея Давыдова

Building Range Adaptors ([part 1](https://www.youtube.com/watch?v=YWayW5ePpkY), [part 2](https://www.youtube.com/watch?v=g-F280_AQp8)) &mdash; Chris Di Bella, CppCon

[Move only folly::Function](https://youtu.be/SToaMS3jNH0) &mdash; Sven Over, CppCon

[Inplace std::function alternative for audio processing](https://youtu.be/VY83afAJUIg) &mdash; Tom Poole, CppCon

[C++14 Reflections Without Macros, Markup nor External Tooling](https://youtu.be/abdeAew3gmQ) &mdash; Антон Полухин, CppCon

[How to Hack C++ with Templates and Friends](https://www.worldcadaccess.com/blog/2020/05/how-to-hack-c-with-templates-and-friends.html)

[C++ vtables](https://shaharmike.com/cpp/vtable-part1/) &mdash; таблицы виртуальных функций в деталях

[Customization Point Design in C++11 and Beyond](http://ericniebler.com/2014/10/21/customization-point-design-in-c11-and-beyond/) &mdash; ниблоиды, CPO используемые в `std::ranges`

[tag_invoke: A general pattern for supporting
customisable functions (P1895)](http://open-std.org/JTC1/SC22/WG21/docs/papers/2019/p1895r0.pdf) &mdash; новый ультимативный механизм для CPO 

[function2](https://github.com/Naios/function2) &mdash; улучшеный `std::function`, пример реализации `vtable` руками

[libunifex](https://github.com/facebookexperimental/libunifex) &mdash; новый фреймворк для асинхронности в плюсах. Содержит примеры и на `vtable`, и на тайплисты, и на `tag_invoke`, и вообще на всё на свете

## Полезные книжки

Andrei Alexandrescu, "Modern C++ Design: Generic Programming and Design Patterns Applied" &mdash; слегка устаревшая классика

David Vandevoorde, Nicolai M. Josuttis, Douglas Gregor, "C++ Templates: The Complete Guide (2nd ed.)" &mdash; всё, что нужно знать о шаблонах в C++17
