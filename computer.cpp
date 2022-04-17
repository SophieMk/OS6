#include <cassert>
#include <map>
#include <sstream>
#include <unistd.h>

#include "requester.hpp"

using namespace std;

class Computer : public Requester
{
    zmq::socket_t socket_parent; //create a socket 

public:
    Computer(int id_self) :
        Requester(id_self),
        socket_parent(context, ZMQ_REP) // A socket of type ZMQ_REQ is used by a client to send requests to and receive replies from a service.
    {
        // Например, если наш идентификатор 11, то мы знаем, что создавшим
        // нас узлом приготовлен сокет на порту 10011, и мы подключаемся к нему.
        connect(socket_parent, id_self); //инициирует соединение на сокете 
    }

    void loop()
    {
        while (true) {
            string request_string; //переменная для запросов
            try {
                request_string = receive_message(socket_parent); //если получаем сообшение от сокета
            } catch (...) {
                continue; //продолжаем
            }
            debug("request_string: " + request_string);

            istringstream request_stream(request_string); //istringstream copies the string that you give it

            size_t len_path; //объявляем длину пути
            request_stream >> len_path; //считываем длину пути
            vector<int> path(len_path); //объявляем вектор из элементов пути
            int i_self = -1; //задаём i_self по умолчанию
            for (size_t i = 0; i < len_path; i++) {
                request_stream >> path[i]; //считываем путь
                if (path[i] == id_self) { //если путь - id
                    i_self = i; //длина пути i
                }
            }
            if (i_self == -1) { //если путь неверный
                send_message(socket_parent, "Error: Incorrect path"); //отправляем сообщение об ошибке
                continue;
            }
            debug("i_self: " + to_string(i_self));

            if (i_self < (int) len_path - 1) { // не последний
                // Передаём запрос дальше.
                auto& socket_child = get_socket(path[i_self + 1]); //получаем сокет с путём большим на единицу
                send_message(socket_child, request_string);  //отправляем сообщение
                auto response = receive_message(socket_child); //получаем ответ

                // Пересылаем ответ родителю.
                send_message(socket_parent, response);

                continue;
            }

            // Обрабатываем сами.
            string operation;
            request_stream >> operation; //считываем операцию

            if (operation == "create") { //если операция "создать"
                int id;
                request_stream >> id; //считываем id
                int pid = create_node(id); //создаём узел
                send_message(socket_parent, "Ok: " + to_string(pid)); //отправляем сообщение родителю
            } else if (operation == "exec") { //если операция "exec"
                int n;
                request_stream >> n; //считываем количество элементов
                int sum = 0;
                int x;
                for (int i = 0; i < n; ++i) {
                    request_stream >> x; //считываем элементы
                    sum += x; //суммируем
                }
                send_message( //отправляем сообщение
                    socket_parent,
                    "Ok:" + to_string(id_self) + ": " + to_string(sum)
                );
            }
        }
    }
};

int main(int argc, char* argv[])
{
    if (argc != 2) {
        return 1;
    }
    int id_self = atoi(argv[1]);

    Computer(id_self).loop();
}
