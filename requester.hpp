#include <cassert>
#include <iostream>
#include <map>
#include <unistd.h>

#include "zmq_functions.hpp"

const bool IS_DEBUG = false;

using namespace std;

class Requester //класс управляющий|вычислительный узел
{
protected: // доступ только подклассам, но не пользователям класса
    int id_self;
    zmq::context_t context; // передаётся один и тот же во все сокеты
    map<int, zmq::socket_t> id2socket; // только для детей

    Requester(int id_self) :
        id_self(id_self),
        context(1) // один IO-thread (стандартно для внешних соединений)
    {}

    void make_socket(int id) //создание сокета
    {
        debug("make_socket(" + to_string(id) + ")");

        // Создаём сокет.
        assert(id2socket.find(id) == id2socket.end());
        id2socket.insert(
            make_pair(
                id,
                move(zmq::socket_t(context, ZMQ_REQ)) // for REQuests
            )
        );

        // Настраиваем сокет.
        auto& socket = id2socket.find(id)->second;
        socket.setsockopt(ZMQ_SNDTIMEO, 1000); // timeout = 1000 ms
        bind(socket, id); // слушаем порт 10000 + id
    }

    zmq::socket_t& get_socket(int id)  //получение сокета
    {
        debug("get_socket(" + to_string(id) + ")");

        auto it_socket = id2socket.find(id);
        assert(it_socket != id2socket.end());
        return it_socket->second;
    }

    int create_node(int id) //создаём узел
    {
        debug("create_node(" + to_string(id) + ")");

        pid_t pid = fork(); //создаём дочерний процесс
        if (pid < 0) {
            perror("Can't create new process");
            return -1;
        }
        if (pid == 0) {
            execl(
                "./computer",
                "./computer", to_string(id).c_str(), NULL // argv
            );
            perror("Can't execute new process");
            return -1;
        }

        make_socket(id);
        debug("create_node(" + to_string(id) + "): done");
        return pid;
    }

    void debug(string line) {
        if (IS_DEBUG) {
            cerr << "DEBUG (node " << id_self << "): " << line << endl;
        }
    }
};
