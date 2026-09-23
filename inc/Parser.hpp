//IrcMessage
//Representa una línea IRC ya separada por el parser.
#include <string>
#include <vector>

struct IrcMessage
{
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    std::string trailing;
    bool hasTrailing;
};


/*PRIVMSG #general :Hola a todos

command  = "PRIVMSG"
params   = ["#general"]
trailing = "Hola a todos"

IrcParser
Responsable únicamente de convertir texto IRC en IrcMessage.

No debe conocer clientes, canales ni permisos.

Debe:

reconocer \r\n;
conservar fragmentos incompletos;
aceptar varias líneas en un mismo buffer;
conservar los espacios del parámetro trailing;
normalizar el comando a mayúsculas.*/