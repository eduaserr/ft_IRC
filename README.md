1 Paso
Handle Signals
Parseo de argumentos

2 Paso
Iniciar server
Server. cpp,hpp

Socket(familia de protocolo,
	two-way connection based bytestream,
	default protocolo). Retorna un fd con el numero del proceso a procesar.

setsockopt(). activar `SO_REUSEADDR` para poder volver a iniciar el servidor después de cerrarlo sin esperar


estructura `sockaddr_in`
	sockaddr_in address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(_port);


bind(fd del socket,
	ptr a estructura adecuada para el protocolo elegido,
	sizeof(addr_struct)). Enlaza el socket al puerto.




3 Paso
server.run()
	- acceptClient(). aceptar cleintes
	- handleClient(). procesar lineas completas
	- if (_clients.find(fd) == _clients.end()) { continue ; }
	- flushClientOutput()

3.1
AcceptClient()
	accept().









3.2
handleCleint()

3.3
flushClientOutput








.hpp (objetos) a crear

Server.hpp
_name: nombre del canal, por ejemplo #general.
_topic: tema actual del canal.
_members: descriptores de los clientes miembros.
_operators: descriptores de los operadores.
_invited: descriptores de clientes invitados.
_inviteOnly: modo +i.
_topicOperatorsOnly: modo +t.
_hasKey: indica si el canal tiene clave.
_key: clave del canal, modo +k.
_hasUserLimit: indica si existe límite de usuarios.
_userLimit: máximo de miembros, modo +l.


Client.hpp
Channel.hpp

Comandos.hpp -> 1 .hpp para cada comando?