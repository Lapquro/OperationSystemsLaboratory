<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=00c8ff&height=150&section=header&text=Operating%20Systems%20Labs&fontSize=45&fontAlignY=35&animation=twinkling" width="100%"/>

<p align="center">
  <img src="https://img.shields.io/badge/C23-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white" alt="C23" />
  <img src="https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white" alt="CMake" />
  <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux" />
  <img src="https://img.shields.io/badge/POSIX-Ready-success?style=for-the-badge" alt="POSIX" />
</p>

</div>

---

## 💡 О репозитории

Лабораторные работы по курсу **«Операционные системы»** (МАИ, каф. 806, группа М8О-214БВ-25). Каждая работа лежит в своей папке со своим `CMakeLists.txt`, `README.md` и исходниками.

## 📚 Содержание

| Работа | Тема | Ключевые вызовы | Папка |
| :---: | :--- | :--- | :---: |
| **ЛР №1** | Межпроцессное взаимодействие, вариант 5: родитель и потомок, общение через каналы, фильтр составных чисел | `pipe`, `fork`, `dup2`, `execve`, `waitpid` | [`LAB1`](LAB1) |
| **ЛР №2** | Многопоточность, вариант 5: чётно-нечётная сортировка Бетчера, исследование ускорения и эффективности | `pthread_create`, `pthread_join`, `pthread_barrier_*`, `clock_gettime` | [`LAB2`](LAB2) |

## 🗂 Структура

```text
.
├── LAB1/
│   ├── parent.c
│   ├── child.c
│   ├── CMakeLists.txt
│   └── README.md
├── LAB2/
│   ├── main.c
│   ├── test.c
│   ├── CMakeLists.txt
│   ├── Report_Lab2.docx
│   └── README.md
├── .gitignore
└── README.md
```

## 🛠 Быстрый старт

Требования: `CMake` ≥ 4.2, компилятор C с поддержкой C23, Linux/WSL (POSIX).

```bash
cd LAB1            # или LAB2
mkdir cmake-build-debug && cd cmake-build-debug
cmake .. && make
```

Подробности запуска и примеры — в README каждой лабы.

<br>
<div align="center">
  <i>Разработано в рамках курса Операционных Систем 💻</i>
</div>
