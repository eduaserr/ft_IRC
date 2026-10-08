# Plan de construcción de `ft_irc`

## Objetivo

Construir un servidor IRC en C++98, partiendo de un núcleo TCP no bloqueante, continuando con el parser y el estado de usuarios/canales, y terminando con los comandos obligatorios y las pruebas de integración.

El subject oficial es la autoridad. El repositorio local y la guía de GitHub se utilizan como referencias de arquitectura, no como código para copiar.

## Requisitos obligatorios

- C++98.
- Servidor IRC, no cliente IRC.
- Comunicación TCP/IP.
- Varios clientes simultáneos.
- Sin `fork()`.
- Todas las operaciones de I/O no bloqueantes.
- Una sola llamada a `poll()`.
- Sin comunicación servidor-servidor.
- Autenticación mediante password.
- Nickname y username.
- JOIN y mensajes privados.
- Comandos `KICK`, `INVITE`, `TOPIC` y `MODE`.
- Modos `i`, `t`, `k`, `o` y `l`.

Como referencias de protocolo se deben consultar RFC 1459 y RFC 2812-2813. Para la interacción visual se recomienda HexChat. Para pruebas deterministas se puede usar `nc` o un cliente TCP pequeño.

## Fase 0: preparación

1. Confirmar compilador, Linux y C++98.
2. Acordar ramas, formato de commits y responsable de integración.
3. Elegir cliente IRC de referencia.
4. Crear una matriz con cada comando, sus parámetros, precondiciones, estado que modifica, receptores y respuestas numéricas.
5. Definir desde el inicio:
   - Terminador `\r\n`.
   - Límite máximo de línea.
   - Tratamiento de fragmentos TCP.
   - Parámetros trailing.
   - Política de desconexión.
6. Mantener como comprobaciones obligatorias:
   - `make`
   - `make clean`
   - `make fclean`
   - `-Wall -Wextra -Werror -std=c++98`

## Fase 1: ejecutable y servidor TCP

1. Validar en `main.cpp` exactamente `<port> <password>`.
2. Validar el rango del puerto y errores de entrada.
3. Implementar en `Server`:
   - `socket()`.
   - `setsockopt(SO_REUSEADDR)`.
   - `fcntl(O_NONBLOCK)`.
   - `bind()`.
   - `listen()`.
4. Crear una única colección de `pollfd`.
5. Registrar el socket servidor con `POLLIN`.
6. Aceptar clientes hasta que `accept()` devuelva `EAGAIN` o `EWOULDBLOCK`.
7. Registrar cada cliente por descriptor.
8. Cerrar correctamente socket, `pollfd`, buffers y objeto `Client`.
9. Gestionar `SIGINT` y `SIGTERM` sin escribir en sockets desde el handler.

### Comprobación

- El servidor arranca.
- Se conectan dos o más clientes.
- El proceso no se bloquea sin tráfico.
- `Ctrl-C` libera recursos.
- El puerto puede reutilizarse después de cerrar el servidor.

## Fase 2: loop no bloqueante y buffers

1. Ejecutar un único `poll()` por iteración.
2. Procesar eventos de lectura y escritura.
3. Añadir a cada cliente:
   - Buffer de entrada.
   - Buffer de salida.
4. Tener en cuenta que `recv()` puede devolver:
   - Un fragmento de línea.
   - Varias líneas.
   - Cero bytes por desconexión.
5. Tener en cuenta que `send()` puede enviar solo una parte del buffer.
6. Activar `POLLOUT` únicamente cuando exista salida pendiente.
7. Mantener `POLLIN` activo junto con `POLLOUT` cuando sea necesario.
8. Gestionar `POLLHUP`, `POLLERR`, `POLLNVAL` y `EPIPE`.
9. Evitar invalidar iteradores cuando un comando desconecta a un cliente mientras se recorre `pollfds`.

### Comprobación

- Enviar una orden dividida en varios `send()`.
- Enviar varias órdenes dentro de un único paquete.
- Generar una salida grande.
- Desconectar un cliente abruptamente.

## Fase 3: parser IRC

1. Crear una estructura de mensaje IRC con:
   - Prefijo opcional.
   - Comando.
   - Parámetros.
   - Parámetro trailing.
2. No usar un parser que destruya los espacios del trailing.
3. Aceptar correctamente `\r\n`.
4. Ignorar líneas vacías según la política definida.
5. Rechazar o gestionar líneas que superen el límite establecido.
6. Normalizar los comandos a mayúsculas.
7. Centralizar el formato de:
   - Prefijo del servidor.
   - Prefijo `:nick!user@host`.
   - Respuestas numéricas.
   - Terminador `\r\n`.
8. Crear un dispatcher que reciba un mensaje ya parseado.
9. Implementar `CAP LS` y `PING/PONG` si el cliente elegido los envía durante la conexión.
10. Definir los errores más utilizados: `001`, `401`, `403`, `431`, `433`, `442`, `451`, `461`, `462`, `464`, `471`, `472`, `473`, `475` y `482`.

### Comprobación

- Comandos en mayúsculas y minúsculas.
- Trailing con espacios.
- `PRIVMSG` vacío.
- Comandos sin parámetros.
- Comandos desconocidos.
- Mensajes sin terminar y fragmentados.

## Fase 4: estado y registro

### `Client`

Debe almacenar como mínimo:

- Descriptor.
- Host o información necesaria para el prefijo.
- Nickname.
- Username.
- Realname.
- Estado de password.
- Estado de registro.
- Buffer de entrada.
- Buffer de salida.

### `Server`

Debe ser responsable de:

- Password global.
- Clientes por descriptor.
- Búsqueda de clientes por nickname.
- Canales por nombre.
- Servicios de envío.
- Creación y eliminación de canales.

### `Channel`

Debe almacenar:

- Nombre.
- Topic.
- Miembros.
- Operadores.
- Invitados.
- Password del canal.
- Límite de usuarios.
- Modos `i` y `t`.

Debe encapsular membership, operadores, invitaciones, eliminación de miembros y broadcast.

### Registro

1. Implementar `PASS`.
2. Implementar `NICK`.
3. Implementar `USER`.
4. Validar nickname único.
5. Permitir que los comandos lleguen en el orden real usado por el cliente.
6. Emitir `001` una sola vez cuando password, nickname y usuario sean válidos.
7. Restringir los comandos que requieren registro.

### Comprobación

- PASS, NICK y USER en distintos órdenes.
- Password incorrecta.
- Nickname duplicado.
- Cambio de nickname.
- Bienvenida única.

## Fase 5: canales y comandos básicos

### JOIN

1. Validar nombre del canal.
2. Crear el canal según la política acordada.
3. Aplicar invite-only, password y límite.
4. Convertir al primer miembro en operador.
5. Emitir el mensaje JOIN a los receptores correctos.
6. Responder topic y lista de usuarios.

### PRIVMSG

1. Aceptar como destino un nickname o un canal.
2. Validar que el usuario tenga permisos para escribir en el canal.
3. Enviar al usuario o a los miembros correspondientes.
4. Gestionar `401`, `404`, `442` y `461`.
5. Mantener el mensaje completo después de `:`.

### PART y QUIT

Aunque no sean el foco principal, son necesarios para limpiar correctamente membresías y probar desconexiones.

1. Notificar la salida.
2. Eliminar al cliente del canal.
3. Retirar sus privilegios e invitaciones.
4. Reasignar operador cuando la política lo requiera.
5. Eliminar canales vacíos si corresponde.

### WHO y NAMES

Implementarlos si el cliente de referencia los necesita para mostrar los miembros del canal.

### Comprobación

- Dos clientes en el mismo canal.
- Mensaje privado.
- Mensaje a canal.
- Usuario que no pertenece al canal.
- PART.
- QUIT.
- Cierre del socket.

## Fase 6: comandos obligatorios de canal

### MODE

Implementar individualmente:

- `+i` / `-i`: canal solo por invitación.
- `+t` / `-t`: solo operadores cambian el topic.
- `+k` / `-k`: password del canal.
- `+o` / `-o`: operador del canal.
- `+l` / `-l`: límite de usuarios.

Validar:

- Argumentos necesarios.
- Operador del canal.
- Existencia del usuario objetivo.
- Existencia del canal.
- Cambio de varios modos.
- Respuesta y broadcast correctos.

### TOPIC

1. Permitir consultar el topic.
2. Validar pertenencia.
3. Respetar el modo `+t`.
4. Notificar el cambio al canal.

### INVITE

1. Validar canal y usuario.
2. Validar permisos del invitador.
3. Guardar la invitación.
4. Enviar respuesta `341`.
5. Enviar el mensaje INVITE al usuario invitado.

### KICK

1. Validar que el ejecutor sea operador.
2. Validar que el objetivo pertenezca al canal.
3. Aceptar motivo opcional.
4. Notificar al canal.
5. Retirar al usuario, sus privilegios y su invitación.

Los comandos no deben cerrar directamente sockets ni manipular `pollfd`. Toda salida debe pasar por el buffer no bloqueante del servidor.

## Fase 7: integración

1. Congelar las interfaces de `Server`, `Client`, `Channel`, parser y `Command`.
2. Integrar en este orden:
   - Transporte.
   - Parser y registro.
   - Estado de canales.
   - Comandos.
3. Integrar ramas frecuentemente.
4. Evitar juntar las tres ramas al final.
5. Designar una única persona responsable de:
   - `Server::run`.
   - Ciclo de vida de clientes.
   - Gestión de `pollfds`.
   - Política de salida.
6. Cada cambio debe compilar en C++98 y aportar una prueba o pasos de reproducción.

## Reparto entre tres integrantes

### Integrante A: red y ciclo de vida

- `main.cpp`.
- `Server`.
- Sockets.
- `poll()`.
- `accept()`.
- Lectura y escritura.
- Buffers.
- Señales.
- Desconexiones.
- Makefile.

Debe mantener la autoridad sobre `Server::run`, `sendToClient` y `removeClient`.

### Integrante B: protocolo y registro

- Parser IRC.
- Respuestas numéricas.
- Dispatcher.
- `Client`.
- `PASS`.
- `NICK`.
- `USER`.
- `CAP`.
- `PING/PONG`.

Debe definir el contrato que utilizarán los comandos de canal.

### Integrante C: dominio y comandos

- `Channel`.
- Membresías.
- Operadores.
- Invitaciones.
- `JOIN`.
- `PRIVMSG`.
- `PART`.
- `QUIT`.
- `WHO/NAMES`.
- `MODE`.
- `TOPIC`.
- `INVITE`.
- `KICK`.

No debe modificar el loop de red y debe utilizar los servicios públicos del servidor.

### Trabajo compartido

- Matriz de errores.
- Pruebas con varios clientes.
- Revisión de RFC y HexChat.
- Integración.
- Corrección de bugs.
- Revisión de memoria y descriptores.

## Archivos de referencia del repositorio actual

- `main.cpp`: entrada, validación de argumentos y señales.
- `includes/Server.hpp`: interfaz del servidor.
- `src/Server.cpp`: sockets, loop y gestión de clientes.
- `includes/Client.hpp`: datos y estado de usuarios.
- `src/Client.cpp`: implementación de usuarios y buffers.
- `includes/Channel.hpp`: datos y operaciones de canales.
- `src/Channel.cpp`: membresías, operadores, invitaciones y broadcast.
- `includes/commands/Command.hpp`: interfaz de comandos.
- `includes/commands/CommandHandler.hpp`: dispatcher y helpers.
- `src/commands/`: ejemplos de implementación de comandos.
- `Makefile`: compilación C++98.
- `bircd/`: ejemplo separado en C con `select()`. No mezclarlo con la implementación final, que debe utilizar una única llamada a `poll()`.

## Verificación final

1. Ejecutar `make fclean && make`.
2. Compilar con `-Wall -Wextra -Werror -std=c++98`.
3. Probar líneas fragmentadas.
4. Probar varias órdenes en un mismo paquete.
5. Probar `send()` parcial y buffers grandes.
6. Conectar al menos tres clientes simultáneos.
7. Probar registro, nick duplicado, JOIN y PRIVMSG.
8. Probar `+i`, `+k` y `+l`.
9. Probar MODE, TOPIC, INVITE, KICK, PART y QUIT.
10. Probar desconexiones abruptas.
11. Probar con HexChat.
12. Ejecutar AddressSanitizer y UndefinedBehaviorSanitizer si están disponibles.
13. Confirmar manualmente que:
    - Solo existe una llamada a `poll()`.
    - No existe `select()`.
    - No existe `fork()`.
    - No hay threads.
    - No se realizan I/O bloqueantes.
    - Los comandos no cierran directamente descriptores.

El bonus de transferencia de archivos y bot se debe abordar solamente después de que toda la parte obligatoria sea estable.

## Riesgos principales

1. TCP no conserva fronteras de mensajes: un `recv()` no equivale necesariamente a una línea IRC.
2. Un `send()` parcial exige conservar la salida pendiente.
3. El borrado de clientes puede invalidar iteradores y punteros almacenados en canales.
4. El parser debe conservar correctamente los espacios del trailing.
5. El código de referencia contiene decisiones simplificadas: siempre comparar su comportamiento con el subject y el cliente elegido.
