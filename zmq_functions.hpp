#include <iostream>
#include <zmq.hpp>

using namespace std;

const int PORT_BASE = 10000;

void send_message(zmq::socket_t& socket, const string& msg) //отправить сообщение
{
    zmq::message_t message(msg.size());
    memcpy(message.data(), msg.c_str(), msg.size()); //Функция memcpy копирует msg.size() байтов первого блока памяти, на который ссылается указатель msg.c_str(), 
	//во второй блок памяти, на который ссылается указатель message.data().
    socket.send(message); //Возвращает значение true, если сообщение успешно отправлено, значение false, если это не так
}

string receive_message(zmq::socket_t& socket) //получить сообщение
{
    zmq::message_t message;
    if (! socket.recv(&message)) { //если сокет не получил сообщение
        throw runtime_error("socket.recv returned false"); //выкинем ошибку
    }
    string received_msg(static_cast<char*>(message.data()), message.size()); //данные в сообщении, размер сообщения
    return received_msg;
}

string id2address(int id) //новый адрес (127.0.0.1 - адрес интернет-протокола loop-back)
                          //Использование адреса 127.0.0.1 позволяет устанавливать соединение и передавать информацию 
						  //для программ-серверов, работающих на том же компьютере, что и программа-клиент, независимо от конфигурации аппаратных сетевых средств компьютер
{
    return "tcp://127.0.0.1:" + to_string(PORT_BASE + id);
}

void connect(zmq::socket_t& socket, int id) //инициирует соединение на сокете 
{
    socket.connect(id2address(id));
}

void disconnect(zmq::socket_t& socket, int id) //обрывает соединение на сокете 
{
    socket.disconnect(id2address(id));
}

void bind(zmq::socket_t& socket, int id) //create an endpoint for accepting connections and bind it to the socket referenced by the socket argument.
{
    socket.bind(id2address(id));
}

void unbind(zmq::socket_t& socket, int id)
{
    socket.unbind(id2address(id));
}
