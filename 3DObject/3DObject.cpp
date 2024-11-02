#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include <memory>

const double eps = 1e-6;
const double pi = std::acos(-1);

struct Point3D { double x{}, y{}, z{}; };   //точка в 3-х мерном пространстве

std::ostream& operator<<(std::ostream& os, const Point3D& p)
{   // точка в поток
    return os << "P {" << p.x << ", " << p.y << ", " << p.z << "}";
}

struct Vector3D { double x{}, y{}, z{}; };  //вектор в 3-х мерном пространстве

std::ostream& operator<<(std::ostream& os, const Vector3D& v)
{   // вектор в поток
    return os << ", V {" << v.x << ", " << v.y << ", " << v.z << "}";
}

class Figure3D //базовый класс
{
protected:
    Point3D c{};
    Figure3D() = default;
    Figure3D(const Point3D& center) : c{ center } {}

public:
    virtual ~Figure3D() {}
    virtual Point3D curve_point(double t) const = 0;    //точка кривой по параметру t
    virtual Vector3D curve_vector(double t) const = 0;  //вектор кривой по параметру t
};

class Circle3D : public Figure3D //окружность в плоскости XY
{
    double radi{};

public:
    Circle3D() = default;
    Circle3D(const Point3D& center, double radius)
        : Figure3D(center), radi{ radius } {}

    virtual ~Circle3D() {}
    double radius() const { return radi; }  //сортировка и суммирование

    virtual Point3D curve_point(double t) const override
    {
        return Point3D
        {
            radi * std::cos(t) + c.x,
            radi * std::sin(t) + c.y,
            c.z
        };
    }
    virtual Vector3D curve_vector(double t) const override
    {
        return Vector3D
        {
            -1 * radi * std::sin(t),
            radi * std::cos(t)
        };
    }
};

class Ellipse3D : public Figure3D    //эллипс в плоскости XY
{
    double radi_x{};
    double radi_y{};

public:
    Ellipse3D() = default;
    Ellipse3D(const Point3D& center, double radius_x, double radius_y)
        : Figure3D(center), radi_x{ radius_x }, radi_y{ radius_y } {}

    virtual ~Ellipse3D() {}

    virtual Point3D curve_point(double t) const override
    {
        return Point3D
        {
            radi_x * std::cos(t) + c.x,
            radi_y * std::sin(t) + c.y,
            c.z
        };
    }
    virtual Vector3D curve_vector(double t) const override
    {
        return Vector3D
        {
            -1 * radi_x * std::sin(t),
            radi_y * std::cos(t)
        };
    }
};

class Helix3D : public Figure3D  //спираль с осью, параллельной оси Z
{
    double radi{};
    double step{};

public:
    Helix3D() = default;
    Helix3D(const Point3D& center, double radius, double step_z)
        : Figure3D(center), radi{ radius }, step{ step_z } {}

    virtual ~Helix3D() {}

    virtual Point3D curve_point(double t) const override
    {
        return Point3D
        {
            radi * t * std::cos(t) + c.x,
            radi * t * std::sin(t) + c.y,
            step * t + c.z
        };
    }
    virtual Vector3D curve_vector(double t) const override
    {
        return Vector3D
        {
            radi * (std::cos(t) - t * std::sin(t)),
            radi * (std::sin(t) + t * std::cos(t)),
            step
        };
    }
};

using PFigure = std::shared_ptr<Figure3D>; //умный указатель на базовый класс

class Figure_generator
{
    enum figure_t { circle_t, ellipse_t, helix_t }; //типы генерируемых объектов

    static std::random_device rd;

    std::uniform_real_distribution<double> udd{ -1, 1 };    //генератор случайных чисел
    std::uniform_int_distribution<> gen{circle_t, helix_t };   // генератор случайных типов объектов

public:

    void set_diapason(double min, double max)   // если нужно изменить диапазон
    {
        if ((std::abs(max) + std::abs(min)) < 2 * eps)  // если значения диапазона одновременно слишко близки к нулю,
            return; // ничего не меняем
        udd = decltype(udd){ min, max }; // меняем параметры ГСВЧ
    }

    PFigure circle()
    {   // метод генерирует и размещает в динамической памяти объект класса Circle3D
        // и возвращает умный указатель на базовый класс
        double radius{};
        while (radius < eps)
            radius = std::abs(udd(rd));

        return std::make_shared<Circle3D>(Point3D{ udd(rd), udd(rd) }, radius);
    }

    PFigure ellipse()
    {   // метод генерирует и размещает в динамической памяти объект класса Ellipse3D
        // и возвращает умный указатель на базовый класс
        double radius_x{};
        while (radius_x < eps)
            radius_x = std::abs(udd(rd));

        double radius_y{};
        while (radius_y < eps)
            radius_y = std::abs(udd(rd));

        return std::make_shared<Ellipse3D>(Point3D{ udd(rd), udd(rd) }, radius_x, radius_y);
    }

    PFigure helix()
    {   //метод генерирует и размещает в динамической памяти объект класса Helix3D
        //возвращает умный указатель на базовый класс
        double radius{};
        while (radius < eps)
            radius = std::abs(udd(rd));

        double step{};
        while (std::abs(step) < eps)
            step = udd(rd);

        return std::make_shared<Helix3D>(Point3D{ udd(rd), udd(rd), udd(rd) }, radius, step);
    }
    PFigure random_figure()
    {   // метод генерирует случайную фигуру из доступного набора
        switch (gen(rd))
        {
        case circle_t:
            return circle();
        case ellipse_t:
            return ellipse();
        case helix_t:
            return helix();
        }
    }
};

std::random_device Figure_generator::rd;

int main()
{
    constexpr size_t sz = 10;   //размер первого массива

    std::vector<PFigure> figure_vec;  //массив указателей на объекты базового класса

    {
        Figure_generator s_gen;  //генератор
        s_gen.set_diapason(-10, 10);    //инициализируем диапазон
        for (size_t i{}; i < sz; ++i)
            figure_vec.emplace_back(s_gen.random_figure());   //заполняем массив
    }

    using PCircle = std::shared_ptr<Circle3D>;  //псевдоним типа - умный указатель на Circle3D

    std::vector<PCircle> circle_vector;    //массив указателей на Circle3D из первого контейнера

    double radius_sum{};    //сумма радиусов Circle3D

    for (auto& ps : figure_vec)
    {
        const std::type_info& ti{ typeid(*ps) };

        std::cout << ti.name() << ' '
            << ps->curve_point(pi / 4) << ' '
            << ps->curve_vector(pi / 4) << std::endl;   

        if (ti.hash_code() == typeid(Circle3D).hash_code()) // если текущий объект - указатель на Circle3D
        {
            circle_vector.emplace_back(std::dynamic_pointer_cast<Circle3D>(ps));   // кастим его к указателю на Circle3D
            radius_sum += circle_vector.back()->radius();  // плюсуем радиус
        }
        std::cout << std::endl;
    }
    std::sort(circle_vector.begin(), circle_vector.end(), [](const auto& lh, const auto& rh)  // сортируем по радиусу
        {
            return lh->radius() < rh->radius();
        });

    std::cout << "Sorted radiuses: \n";
    for (auto& pc : circle_vector) //Вывод отсортированных радиусов
        std::cout << pc->radius() << std::endl;
    std::cout << "sum = " << radius_sum << std::endl;   //Выводим сумму радиусов
}