#include <cassert>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

#include "requester.hpp"

using namespace std;

class Controller : public Requester
{
    map<int, vector<int>> id2path; // для всех узлов

    bool is_id_known(int id)
    {
        return id2path.find(id) != id2path.end();
    }

    // Обозначения:
    // - request = path + command
    // - path = size + ids
    // - command = operation + arguments
    string send_command(int id, string command)
    {
        // Находим путь.
        auto it_path = id2path.find(id);
        assert(it_path != id2path.end());
        auto& path = it_path->second;
        assert(path.size() != 0);
        int id_child = path[0];

        // Формируем сообщение.
        string request = to_string(path.size());
        for (int id : path) {
            request += " " + to_string(id);
        }
        request += " " + command;

        // Находим сокет.
        auto& socket = get_socket(id_child);

        // Отправляем сообщение и возвращаем ответ.
        debug("send_command to " + to_string(id) + ": " + command);
        send_message(socket, request);
        return receive_message(socket);
    }

    void operation_create()
    {
        int id, id_parent;
        cin >> id >> id_parent;

        if (id2path.find(id) != id2path.end()) {
            cerr << "Error: Already exists" << endl;
            return;
        }

        // Копируем путь из родителя.
        auto& path = id2path[id] = vector<int>();
        if (id_parent != -1) {
            assert(id2path.find(id_parent) != id2path.end());
            auto& path_parent = id2path[id_parent];
            path.assign(path_parent.begin(), path_parent.end());
        }
        // Дополняем путь.
        path.push_back(id);

        if (id_parent == -1) {
            // Создаём узел сами.
            int pid = create_node(id);
            cout << "Ok: " << pid << '\n';
            return;
        }

        if (! is_id_known(id_parent)) {
            cerr << "Error: Parent not found" << endl;
            return;
        }

        // Делегируем команду родителю создаваемого узла.
        try {
            cout << send_command(id_parent, "create " + to_string(id)) << endl;
        } catch (...) {
            cerr << "Error: Parent is unavailable" << endl;
        }
    }

    void operation_exec()
    {
        // Вначале читаем команду полностью.
        int id;
        int n;
        cin >> id >> n;

        string args_string = to_string(n);
        for (int i = 0; i < n; i++) {
            int arg;
            cin >> arg;
            args_string += ' ' + to_string(arg);
        }

        // Затем обрабатываем.
        if (! is_id_known(id)) {
            cerr << "Error:" << id << ": Not found" << endl;
            return;
        }

        try {
            cout << send_command(id, "exec " + args_string) << endl;
        } catch (...) {
            cerr << "Error:" << id << ": Node is unavailable" << endl;
        }
    }

    bool is_node_available(int id)
    {
        string response;
        try {
            response = send_command(id, "exec 0");
        } catch (...) {
            return false;
        }

        return response.rfind("Ok:", 0) == 0; // начинается с этой строки
    }

    void operation_pingall()
    {
        vector<int> ids_unavailable;
        for (const auto& pair_id_path : id2path) {
            int id = pair_id_path.first;
            if (! is_node_available(id)) {
                ids_unavailable.push_back(id);
            }
        }

        cout << "Ok: ";
        if (ids_unavailable.empty()) {
            cout << -1;
        } else {
            for (size_t i = 0; i < ids_unavailable.size(); i++) {
                if (i > 0) {
                    cout << ';';
                }
                cout << ids_unavailable[i];
            }
        }
        cout << endl;
    }

public:
    Controller() : Requester(-1) {}

    void loop()
    {
        while (true) { //считывается команда
            debug("reading operation");
            string operation;
            if (! (cin >> operation)) {
                break;
            }

            if (operation == "create") {
                operation_create();
            } else if (operation == "exec") {
                operation_exec();
            } else if (operation == "pingall") {
                operation_pingall();
            } else {
                cerr << "Error: Incorrect operation" << endl;
            }
        }
        debug("exiting");
    }
};

int main()
{
    Controller().loop();
}
