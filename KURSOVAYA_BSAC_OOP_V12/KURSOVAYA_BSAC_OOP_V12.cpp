#include <Windows.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <vector>

#define INVALID_IDX -1
const auto toys_vec_filename = "toys_vec.txt";
const auto users_filename = "users.txt";

std::string encrypt(std::string str, int key = 3) {
    for (auto& ch : str)
        ch += key;
    return str;
}

std::string decrypt(std::string str, int key = 3) {
    return encrypt(str, -key);
}

enum USER_STATUS {
    USER = 1,
    ADMIN
};

struct IData {
    virtual void print() const = 0;
    virtual void print_header() const = 0;
    virtual ~IData() = default;
};

struct Toy_t : virtual IData {
    static inline const auto fmt = "{: ^16}|{: ^16}|{: ^12}|{: ^13}|{: ^10}";
    std::string name, maker;
    int count, min_age;
    float price;

    Toy_t() = default;
    Toy_t(Toy_t&& other) : name{std::move(other.name)},
                           maker{std::move(other.maker)},
                           count{std::move(other.count)},
                           min_age{std::move(other.min_age)},
                           price{std::move(other.price)} {}

    Toy_t(const Toy_t& other) : name{other.name},
                                maker{other.maker},
                                count{other.count},
                                min_age{other.min_age},
                                price{other.price} {}

    Toy_t& operator=(const Toy_t& other) {
        name = other.name;
        maker = other.maker;
        count = other.count;
        min_age = other.min_age;
        price = other.price;
        return *this;
    }

    void print() const override {
        std::println(fmt, name, maker, count, min_age, price);
    }

    void print_header() const override {
        std::println(fmt, "Название", "Производитель", "Количество", "Мин. возраст", "Стоимость");
    }

    friend std::ostream& operator<<(std::ostream& ostr, const Toy_t& t) {
        ostr << t.name << ' ' << t.maker << ' ' << t.count << ' ' << t.min_age << ' ' << t.price;
        return ostr;
    }

    friend std::istream& operator>>(std::istream& istr, Toy_t& t) {
        istr >> t.name >> t.maker >> t.count >> t.min_age >> t.price;
        return istr;
    }
};

struct User_t : virtual IData {
    static inline const auto fmt = "{: ^16}|{: ^16}|{: ^6}";
    int role;
    std::string login, password;

    User_t() = default;
    User_t(User_t&& other) : role{std::move(other.role)},
                             login{std::move(other.login)},
                             password{std::move(other.password)} {}

    User_t(const User_t& other) : role{other.role},
                                  login{other.login},
                                  password{other.password} {}

    User_t& operator=(const User_t& other) {
        role = other.role;
        login = other.login;
        password = other.password;
        return *this;
    }

    void print() const override {
        std::println(fmt, login, password, role);
    }

    void print_header() const override {
        std::println(fmt, "Логин", "Пароль", "Роль");
    }

    friend std::ostream& operator<<(std::ostream& ostr, const User_t& u) {
        ostr << u.login << ' ' << u.password << ' ' << u.role;
        return ostr;
    }

    friend std::istream& operator>>(std::istream& istr, User_t& u) {
        istr >> u.login >> u.password >> u.role;
        return istr;
    }
};

static std::vector<Toy_t> toys_vec;
static std::vector<User_t> users_vec;

User_t* find_user_ptr(const std::string& login) {
    for (size_t i = 0; i < users_vec.size(); i++)
        if (users_vec[i].login == login)
            return &users_vec[i];
    return nullptr;
}

void save(const auto& vec, const std::string& filename) {
    std::ofstream file_obj(filename);
    for (const auto& obj : vec)
        file_obj << obj << '\n';
}

template <typename T>
auto load(const std::string& filename) {
    std::vector<T> return_value;
    if (std::ifstream file_obj(filename); !file_obj)
        std::println("Невозможно загрузить из {}", filename);
    else {
        std::string buf;
        T temp{};
        while (std::getline(file_obj, buf)) {
            std::stringstream sstr(buf);
            sstr >> temp;
            return_value.emplace_back(std::move(temp));
        }
    }
    return return_value;
}

static User_t* login() {
    User_t *found_entry = nullptr, *user_ptr = new User_t;
    std::println("Вход");
    std::print("Введите логин: ");
    std::cin >> user_ptr->login;
    if (found_entry = find_user_ptr(user_ptr->login); !found_entry) {
        std::println("Такого пользователя нет");
        delete user_ptr;
        return nullptr;
    }
    std::print("Введите пароль: ");
    std::cin >> user_ptr->password;
    if (decrypt(found_entry->password) != user_ptr->password) {
        std::println("Неверный пароль");
        delete user_ptr;
        return nullptr;
    }
    user_ptr->role = found_entry->role;
    return user_ptr;
}

static User_t* create_user() {
    User_t* new_user = new User_t;
    std::println("Создание пользователя");
    std::print("Введите логин: ");
    std::cin >> new_user->login;
    if (find_user_ptr(new_user->login) != nullptr) {
        std::println("Пользователь с таким логином уже существует");
        delete new_user;
        return nullptr;
    }
    std::print("Введите пароль: ");
    std::cin >> new_user->password;

    std::string repeat_passw;
    std::print("Повторите пароль: ");
    std::cin >> repeat_passw;
    if (new_user->password != repeat_passw) {
        std::println("Пароли не совпадают");
        delete new_user;
        return nullptr;
    }
    new_user->password = encrypt(new_user->password);
    std::print("Введите роль\n1) Пользователь\n2) Администратор\n");
    std::cin >> new_user->role;
    users_vec.emplace_back(*new_user);
    save(users_vec, users_filename);
    return new_user;
}

static User_t* auth() {
    if (users_vec.empty())
        return create_user();
    int choice;
    std::cout << "1) Войти\n2) Зарегистрироваться\n";
    (std::cin >> choice).get();
    return choice == 1 ? login() : create_user();
}

void create_toy() {
    std::println("Создание игрушки");
    Toy_t temp;
    std::print("Введите название: ");
    (std::cin >> temp.name).get();
    std::print("Введите производителя: ");
    (std::cin >> temp.maker).get();
    std::print("Введите минимальный рекомендуемый возраст: ");
    (std::cin >> temp.min_age).get();
    std::print("Введите количество: ");
    (std::cin >> temp.count).get();
    std::print("Введите стоимость: ");
    (std::cin >> temp.price).get();
    toys_vec.emplace_back(std::move(temp));
    std::println("Игрушка успешно создана!");
}

auto select_entry(const auto& vec) {
    int choice = INVALID_IDX;
    if (vec.empty())
        return choice;

    std::cout << std::string(4, ' ');
    vec.front().print_header();
    for (size_t i = 0; i < vec.size(); i++) {
        std::print("{: >3})", i + 1);
        vec[i].print();
    }
    std::print("Введите индекс: ");
    (std::cin >> choice).get();
    if (choice <= vec.size() && choice > 0)
        return choice - 1;
    std::println("Индекс вне массива");
    return INVALID_IDX;
}

void edit_toy() {
    const int selected_idx = select_entry(toys_vec);
    if (selected_idx == INVALID_IDX)
        return;
    auto& t = toys_vec[selected_idx];

    std::println(
        "Выберите то, что желаете изменить\n"
        "1) Название\n2) Производитель\n"
        "3) Минимальный возраст\n4) Количество\n5) Стоимость");
    int choice;
    (std::cin >> choice).get();
    std::print("Введите новое значение: ");
    switch (choice) {
        case 1:
            (std::cin >> t.name).get();
            break;
        case 2:
            (std::cin >> t.maker).get();
            break;
        case 3:
            (std::cin >> t.min_age).get();
            break;
        case 4:
            (std::cin >> t.count).get();
            break;
        case 5:
            (std::cin >> t.price).get();
            break;
        default:
            break;
    }
    save(toys_vec, toys_vec_filename);
}

void edit_user() {
    const int selected_idx = select_entry(users_vec);
    if (selected_idx == INVALID_IDX)
        return;
    auto& u = users_vec[selected_idx];
    int choice;
    std::println("Выберите то, что желаете изменить\n1) Логин\n2) Пароль\n3) Роль");
    (std::cin >> choice).get();
    std::print("Введите новое значение: ");
    switch (choice) {
        case 1:
            (std::cin >> u.login).get();
            break;
        case 2: {
            std::string buf;
            (std::cin >> buf).get();
            u.password = encrypt(buf);
            break;
        }
        case 3: {
            int temp;
            std::println("1) Пользователь\n2) Администратор\n");
            (std::cin >> temp).get();
            if (temp == 1 || temp == 2)
                u.role = temp;
            break;
        }
        default:
            break;
    }
    save(users_vec, users_filename);
}

void delete_entry(auto& vec) {
    const int selected_idx = select_entry(vec);
    if (selected_idx == INVALID_IDX)
        return;
    int answ;
    std::println("Вы действительно желаете удалить эту запись?\n1) Да\n2) Нет");
    (std::cin >> answ).get();
    if (answ != 1) {
        std::println("Действие отменено");
        return;
    }
    vec.erase(vec.begin() + selected_idx);
    std::println("Запись удалена");
}

void print_vector(const std::string& title, const auto& vec) {
    std::cout << title << ":\n";
    vec.front().print_header();
    for (const auto& elem : vec)
        elem.print();
}

void sort_toys() {
    if (toys_vec.empty()) {
        std::println("Список игрушек пуст");
        return;
    }
    std::println("Выберите поле для сортировки:");
    std::println("1) Название");
    std::println("2) Производитель");
    std::println("3) Количество");
    std::println("4) Минимальный возраст");
    std::println("5) Стоимость");
    std::println("0) Выход");

    int choice;
    (std::cin >> choice).get();
    switch (choice) {
        case 1:
            std::println("Отсортировано по названию");
            std::sort(toys_vec.begin(), toys_vec.end(),
                      [](const auto& l, const auto& r) { return l.name < r.name; });
            break;
        case 2:
            std::println("Отсортировано по производителю");
            std::sort(toys_vec.begin(), toys_vec.end(),
                      [](const auto& l, const auto& r) { return l.maker < r.maker; });
            break;
        case 3:
            std::println("Отсортировано по количеству");
            std::sort(toys_vec.begin(), toys_vec.end(),
                      [](const auto& l, const auto& r) { return l.count < r.count; });
            break;
        case 4:
            std::println("Отсортировано по минимальному возрасту");
            std::sort(toys_vec.begin(), toys_vec.end(),
                      [](const auto& l, const auto& r) { return l.min_age < r.min_age; });
            break;
        case 5:
            std::println("Отсортировано по стоимости");
            std::sort(toys_vec.begin(), toys_vec.end(),
                      [](const auto& l, const auto& r) { return l.price < r.price; });
            break;
        default:
            return;
    }
    toys_vec.front().print_header();
    for (const auto& toy : toys_vec)
        toy.print();
}

void search_by_name(const std::vector<Toy_t>& toys_vec) {
    std::vector<const Toy_t*> found;
    std::string search_req;
    std::println("Введите название игрушки для поиска: ");
    (std::cin >> search_req).get();

    for (auto& toy : toys_vec) {
        if (toy.name.contains(search_req))
            found.emplace_back(&toy);
    }

    if (found.empty()) {
        std::println("По вашему запросу ничего не найдено!");
        return;
    }
    std::println("Найденные игрушки:");
    found.front()->print_header();
    for (auto ptr : found)
        ptr->print();
}

void print_by_age(const std::vector<Toy_t>& toys_vec) {
    std::vector<const Toy_t*> found;
    int age;
    std::cout << "Введите минимальный рекомендуемый возраст: ";
    (std::cin >> age).get();
    for (const auto& t : toys_vec)
        if (t.min_age >= age)
            found.emplace_back(&t);

    if (found.empty()) {
        std::println("По вашему запросу ничего не найдено!");
        return;
    }
    std::println("Найденные игрушки:");
    found.front()->print_header();
    for (auto ptr : found)
        ptr->print();
}

struct User_menu {
    virtual ~User_menu() = default;
    virtual void show() {
        std::println("1) Посмотреть все игрушки");
        std::println("2) Искать игрушки по возрасту");
    }
    virtual void exec(int index) {
        system("cls");
        switch (index) {
            case 1:
                print_vector("Список игрушек", toys_vec);
                break;
            case 2:
                print_by_age(toys_vec);
                break;
            default:
                return;
        }
    }
};

struct Admin_menu : public virtual User_menu {
    void show() override {
        User_menu::show();
        std::println("3) Добавить игрушку");
        std::println("4) Удалить игрушку");
        std::println("5) Отредактировать игрушку");
        std::println("6) Посмотреть всех пользователей");
        std::println("7) Добавить пользователя");
        std::println("8) Удалить пользователя");
        std::println("9) Отредактировать пользователя");
        std::println("10) Сортировать игрушки");
    }
    void exec(int index) override {
        system("cls");
        switch (index) {
            case 1:
            case 2:
                User_menu::exec(index);
                break;
            case 3:
                create_toy();
                break;
            case 4:
                delete_entry(toys_vec);
                save(toys_vec, toys_vec_filename);
                break;
            case 5:
                edit_toy();
                break;
            case 6:
                print_vector("Список пользователей", users_vec);
                break;
            case 7: {
                delete create_user();
                break;
            }
            case 8:
                delete_entry(users_vec);
                save(users_vec, users_filename);
                break;
            case 9:
                edit_user();
                break;
            case 10:
                sort_toys();
                break;
            default:
                return;
        }
    }
};

void print_copyright() {
    std::println("{}\t{}", "******", "БГАС 2023");
    std::println("{}\t{}", __DATE__, __TIME__);
    std::print("\n\n\n");
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    toys_vec = load<Toy_t>(toys_vec_filename);
    users_vec = load<User_t>(users_filename);

    auto* user = auth();
    if (!user) {
        std::println("Ошибка при создании пользователя");
        return 0;
    }

    auto* menu = user->role == USER_STATUS::ADMIN ? new Admin_menu() : new User_menu();
    int choice;
    while (true) {
        system("cls");
        print_copyright();
        menu->show();
        std::println("0) Выход");
        std::print("Ввод: ");
        (std::cin >> choice).get();
        if (choice == 0)
            break;
        menu->exec(choice);
        system("pause");
    }
    save(users_vec, users_filename);
    save(toys_vec, toys_vec_filename);
    delete user;
    delete menu;
}