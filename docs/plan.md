# Guía razonada para construir `ft_irc` desde cero

Esta guía está escrita para empezar sin conocer todavía el proyecto. No es una lista de metas para marcar al final: explica qué problema resuelve cada parte, cómo se relaciona con las demás y qué debes implementar después de entenderla.

La guía de GitHub utilizada como referencia sigue esta progresión: primero explica qué es un servidor, después cómo funcionan los sockets y la comunicación no bloqueante, luego presenta el protocolo IRC, las respuestas que esperan los clientes y finalmente la implementación de autenticación, comandos generales y comandos de canales. Seguiremos esa misma lógica.

El subject oficial tiene prioridad sobre cualquier implementación de ejemplo.

---

## 0. Qué debes entender antes de escribir código

### El objetivo real

No estamos construyendo una aplicación gráfica ni un cliente IRC. Estamos construyendo el programa que recibe conexiones de clientes IRC y actúa como árbitro central.

Un usuario utiliza HexChat y escribe, por ejemplo, `/join #general`. HexChat no le envía al servidor una orden abstracta como "añade este usuario al canal". Le envía texto por TCP:

```text
JOIN #general\r\n
```

Nuestro servidor tiene que:

1. Recibir los bytes.
2. Reconstruir una orden completa.
3. Entender su sintaxis.
4. Comprobar si el usuario puede ejecutarla.
5. Cambiar el estado interno.
6. Construir las respuestas IRC correctas.
7. Entregarlas sin bloquear a los demás clientes.

Ese es el proyecto completo. Todo lo demás son piezas de este recorrido.

### Las dos capas del programa

Conviene separar mentalmente dos capas:

```text
Capa de transporte
  sockets, TCP, poll, recv, send, buffers, desconexiones

Capa IRC
  parser, usuarios, canales, permisos, comandos, respuestas
```

La capa de transporte no debe saber qué significa `JOIN`. La capa IRC no debe llamar directamente a `send()` ni manipular `pollfd`. Esta separación evita que cada comando invente su propia forma de gestionar la red.

### Las fuentes que debes leer

1. El subject de `ft_irc`.
2. La guía de GitHub proporcionada.
3. RFC 1459 para el protocolo IRC clásico.
4. RFC 2812 para el comportamiento del cliente.
5. RFC 2813 para el contexto de servidores, aunque no implementaremos comunicación servidor-servidor.
6. La documentación del cliente elegido, normalmente HexChat.
7. El repositorio local, como ejemplo de una posible arquitectura.

No hay que implementar todo el RFC. Hay que implementar lo que exige el subject y lo que necesita el cliente de referencia para funcionar correctamente.

---

## 1. Qué es un servidor

Un servidor es un proceso que espera peticiones y ofrece un servicio a otros programas llamados clientes.

En este proyecto:

- El cliente IRC inicia la conexión.
- El servidor escucha en un puerto.
- El cliente envía órdenes de texto.
- El servidor mantiene usuarios y canales.
- El servidor responde a quien corresponda.

El servidor no puede asumir que solo habrá un cliente. Si Ana está conectada y no está enviando nada, eso no debe impedir que Luis se conecte o que Marta reciba un mensaje.

### Primera imagen mental

```text
HexChat Ana ─────┐
HexChat Luis ────┼── TCP ── servidor IRC ── estado de usuarios/canales
HexChat Marta ──┘
```

Cada conexión tiene un socket distinto. El servidor tiene además un socket especial que solo sirve para aceptar conexiones nuevas.

### Qué debe existir al terminar esta fase

Todavía no necesitamos usuarios IRC ni canales. Solo debemos tener un proceso que:

- Reciba un puerto y una password.
- Abra un socket TCP.
- Escuche conexiones.
- Acepte más de un cliente.
- Cierre todo correctamente.

### Prueba conceptual

Antes de seguir, debes poder explicar con tus palabras la diferencia entre:

- El socket de escucha.
- El socket de Ana.
- El socket de Luis.

Si no puedes distinguirlos, todavía no conviene implementar comandos.

---

## 2. Cómo se crea el servidor TCP

La creación del servidor no es una sola función. Es una cadena donde cada llamada prepara la siguiente.

### 2.1 `socket()`

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

Esto crea un descriptor para comunicación TCP IPv4.

- `AF_INET`: direcciones IPv4.
- `SOCK_STREAM`: conexión TCP, fiable y ordenada.
- `0`: protocolo por defecto para esa combinación.

El resultado es solamente un descriptor abierto. Todavía no está asociado a ningún puerto.

### 2.2 `setsockopt()`

Se suele activar `SO_REUSEADDR` para poder volver a iniciar el servidor después de cerrarlo sin esperar a que el sistema libere completamente el puerto.

Esta opción no hace que el servidor sea no bloqueante. Son problemas distintos.

### 2.3 `bind()`

`bind()` asocia el socket con una dirección y un puerto.

Hay que preparar una estructura `sockaddr_in`:

- Familia `AF_INET`.
- Dirección `INADDR_ANY` para aceptar conexiones en las interfaces disponibles.
- Puerto convertido con `htons()`.

Después de `bind()`, el socket tiene una dirección, pero aún no está aceptando conexiones.

### 2.4 `listen()`

`listen()` transforma el socket en un socket de escucha. El segundo argumento representa la cola de conexiones pendientes.

Después de esta llamada, los clientes pueden intentar conectarse y el kernel los mantiene en espera hasta que llamemos a `accept()`.

### 2.5 `accept()`

`accept()` retira una conexión pendiente y devuelve un nuevo descriptor.

Esto es fundamental:

```text
serverFd = socket que escucha
clientFd = socket dedicado a un cliente
```

Nunca debemos leer mensajes IRC desde `serverFd`. Los mensajes se leen desde `clientFd`.

### Orden de implementación

En `Server::_initSocket()` o una función equivalente:

1. Crear socket.
2. Comprobar errores.
3. Activar `SO_REUSEADDR`.
4. Activar modo no bloqueante.
5. Preparar dirección.
6. Ejecutar `bind()`.
7. Ejecutar `listen()`.
8. Registrar `serverFd` para observarlo.

### Prueba de esta fase

Ejecuta el servidor y usa dos terminales con `nc`:

```sh
nc 127.0.0.1 6667
```

Todavía no esperes respuestas IRC. Solo comprueba que el servidor registra dos conexiones sin quedarse detenido en la primera.

---

## 3. Por qué la I/O debe ser no bloqueante

Una llamada bloqueante puede detener todo el proceso.

Imagina este código:

```cpp
recv(clientA, buffer, size, 0);
recv(clientB, buffer, size, 0);
```

Si Ana se conecta pero no envía nada, el primer `recv()` puede quedarse esperando indefinidamente. Luis no será atendido aunque tenga un mensaje listo.

La solución del subject es:

- No usar `fork()`.
- No crear un thread por cliente.
- Configurar los sockets como no bloqueantes.
- Esperar eventos con una única llamada a `poll()`.

### `fcntl()` y `O_NONBLOCK`

Se debe aplicar `O_NONBLOCK` al socket servidor y a cada socket cliente.

Cuando un descriptor no bloqueante no puede realizar temporalmente una operación, la función devuelve un error como `EAGAIN` o `EWOULDBLOCK` en lugar de detener el proceso.

Eso significa que `EAGAIN` normalmente no es una desconexión: significa "vuelve a intentarlo cuando el descriptor esté listo".

---

## 4. Cómo `poll()` permite atender muchos clientes

`poll()` recibe un array de estructuras:

```cpp
struct pollfd
{
    int fd;
    short events;
    short revents;
};
```

- `fd`: descriptor observado.
- `events`: eventos que nos interesan.
- `revents`: eventos que ocurrieron realmente.

El array podría representar:

```text
pollfds[0] = serverFd, POLLIN
pollfds[1] = clientFdAna, POLLIN
pollfds[2] = clientFdLuis, POLLIN | POLLOUT
```

### Qué significa cada evento

- `POLLIN`: se puede leer sin bloquear.
- `POLLOUT`: se puede escribir sin bloquear.
- `POLLHUP`: el otro extremo cerró o colgó la conexión.
- `POLLERR`: error del descriptor.
- `POLLNVAL`: descriptor inválido.

### El loop lógico

```text
mientras el servidor siga activo:
    poll(pollfds)

    para cada descriptor que tenga eventos:
        si es serverFd:
            aceptar conexiones
        si tiene POLLIN:
            leer bytes
        si tiene POLLOUT:
            enviar bytes pendientes
        si tiene error o cierre:
            eliminar cliente
```

El subject exige una sola llamada a `poll()`, no una llamada distinta por cliente.

### Un detalle importante: modificar el vector

Aceptar un cliente añade un `pollfd`. Eliminarlo lo quita. Esto puede invalidar índices e iteradores durante el recorrido.

Por eso debemos diseñar desde el principio una estrategia segura:

- Guardar el descriptor antes de procesarlo.
- Comprobar si sigue existiendo después de ejecutar un comando.
- No asumir que el índice antiguo sigue representando el mismo cliente.
- No continuar usando un `Client*` después de que una función pueda haberlo eliminado.

### Primera implementación real

La persona responsable de red debe implementar ahora:

1. `std::vector<pollfd> _pollfds`.
2. El listener como primer elemento.
3. `_acceptClient()`.
4. Registro de cada `Client` por fd.
5. `run()` con una única llamada a `poll()`.
6. Eliminación de fd de `_pollfds`.

### Criterio para avanzar

Se puede avanzar cuando:

- Se conectan tres clientes.
- Ningún cliente bloquea a los demás.
- Se detecta la desconexión de un cliente.
- El servidor termina limpiamente con una señal.

---

## 5. TCP es un flujo: buffers de entrada y salida

Esta es una de las ideas más importantes de todo el proyecto.

### 5.1 Buffer de entrada

TCP no sabe qué es una orden IRC. El kernel entrega los bytes que hayan llegado, no necesariamente una orden completa.

Supongamos que el cliente envía:

```text
NICK ana\r\nUSER ana 0 * :Ana\r\n
```

Podemos recibirlos así:

```text
recv 1: "NICK a"
recv 2: "na\r\nUSER ana 0 * :Ana\r\n"
```

El `Client` debe guardar temporalmente:

```text
buffer = bytes recibidos pero todavía no procesados
```

Algoritmo:

1. Leer bytes.
2. Añadirlos al buffer.
3. Buscar `\r\n`.
4. Si no aparece, esperar otra lectura.
5. Si aparece, extraer una línea.
6. Procesar todas las líneas completas que haya.
7. Conservar el fragmento final incompleto.

### 5.2 Buffer de salida

El servidor puede producir respuestas más rápido de lo que el cliente las consume. Además, `send()` puede aceptar solo una parte.

Si el buffer contiene:

```text
1234567890
```

y `send()` solo escribe `1234`, el buffer debe quedar así:

```text
567890
```

No se puede descartar la parte no enviada.

Algoritmo:

1. El comando construye una respuesta.
2. `Server::sendToClient()` la añade al buffer de salida.
3. Se activa `POLLOUT`.
4. `poll()` avisa de que se puede escribir.
5. `send()` envía una parte.
6. Se elimina del buffer exactamente esa parte.
7. Se repite hasta vaciarlo.
8. Se desactiva `POLLOUT` cuando no queda nada.

### 5.3 Nunca escribir directamente desde un comando

Un comando no debe hacer:

```cpp
send(client.getFd(), response.c_str(), response.size(), 0);
```

Debe hacer algo conceptualmente equivalente a:

```cpp
server.sendToClient(client.getFd(), response);
```

Así todas las escrituras pasan por la misma política no bloqueante.

### Pruebas necesarias

Antes de implementar IRC:

- Enviar solo una parte de `NICK`.
- Enviar dos órdenes juntas.
- Enviar una orden y media.
- Forzar una respuesta grande.
- Cerrar el cliente mientras tiene salida pendiente.

---

## 6. Entender el protocolo IRC antes de crear comandos

IRC es un protocolo de texto. El cliente y el servidor hablan intercambiando líneas terminadas normalmente en `\r\n`.

### 6.1 Forma general

```text
[:prefix] COMMAND [parameter1 parameter2 ...] [:trailing]\r\n
```

Ejemplos:

```text
PASS secret\r\n
NICK ana\r\n
USER ana 0 * :Ana García\r\n
JOIN #general\r\n
PRIVMSG #general :Hola a todos\r\n
```

### 6.2 Prefijo

El prefijo puede ser opcional en mensajes cliente-servidor. En mensajes servidor-cliente puede identificar al servidor o al usuario.

Un prefijo de usuario tiene normalmente esta forma:

```text
:ana!ana@localhost
```

El cliente utiliza esta información para saber quién realizó una acción.

### 6.3 Comando

Es la palabra que indica la operación: `PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`, etc.

Conviene normalizarlo a mayúsculas, porque los clientes pueden enviarlo con distintas combinaciones de mayúsculas y minúsculas.

### 6.4 Parámetros normales

Son palabras separadas por espacios:

```text
MODE #general +o luis
```

Los parámetros normales son:

```text
#general
+o
luis
```

### 6.5 Parámetro trailing

Cuando aparece `:`, el resto de la línea es un único parámetro, aunque tenga espacios:

```text
PRIVMSG #general :Este mensaje tiene varios espacios
```

El mensaje no tiene cinco parámetros de texto. Tiene:

- Destino: `#general`.
- Mensaje: `Este mensaje tiene varios espacios`.

Un parser que use `stringstream` para todo puede destruir esta información. Hay que separar primero la parte trailing y después tokenizar solo la parte anterior.

### 6.6 Parser recomendado

Crear una estructura como:

```text
IrcMessage
    prefix: opcional
    command: string
    params: vector<string>
    trailing: opcional
```

El parser no debe conocer `Client`, `Channel` ni permisos. Solo debe responder: "esta línea significa estos campos".

### Prueba del parser

Antes de conectarlo a comandos, comprobar manualmente:

```text
NICK ana
USER ana 0 * :Ana García
PRIVMSG #general :Hola, ¿cómo estás?
:ana!u@host PRIVMSG luis :mensaje privado
```

---

## 7. Las respuestas IRC y por qué el formato importa

El servidor no puede responder con texto libre. El cliente espera una estructura concreta.

### 7.1 Respuesta directa del servidor

Ejemplo de bienvenida:

```text
:irc.example.com 001 ana :Welcome to the Internet Relay Network ana\r\n
```

Tiene:

- Prefijo del servidor.
- Código numérico `001`.
- Nickname del destinatario.
- Mensaje trailing.

Los códigos numéricos comunican éxito o error. El cliente puede cambiar su comportamiento según el código.

### 7.2 Respuesta distribuida a un canal

Cuando Ana entra en un canal, no basta con responderle solo a Ana. Los miembros deben recibir:

```text
:ana!ana@localhost JOIN #general\r\n
```

Cuando Ana escribe:

```text
:ana!ana@localhost PRIVMSG #general :Hola\r\n
```

El servidor debe enviar ese mensaje a los miembros apropiados del canal.

### 7.3 Respuesta privada entre clientes

Para un mensaje a Luis:

```text
:ana!ana@localhost PRIVMSG luis :Hola\r\n
```

El servidor encuentra el `Client` asociado a `luis` y coloca el mensaje en su buffer de salida.

### 7.4 Consecuencia de diseño

Cada comando debe responder estas preguntas antes de implementarse:

1. ¿Quién puede ejecutar la orden?
2. ¿Qué estado modifica?
3. ¿Quién recibe la respuesta?
4. ¿Qué prefijo se utiliza?
5. ¿Qué ocurre si falla?
6. ¿Qué código numérico corresponde?

Esta tabla debe formar parte del diseño, no escribirse después de programar.

---

## 8. Elegir y observar el cliente de referencia

La guía de GitHub utiliza HexChat porque un cliente real envía más mensajes de los que aparecen en la lista mínima del subject.

Al conectar, puede enviar algo parecido a:

```text
CAP LS 302
PASS secret
NICK ana
USER ana 0 * :Ana García
```

No debemos asumir que solo llegará:

```text
PASS secret
NICK ana
USER ana 0 * :Ana
```

### Qué observar con HexChat

1. Qué envía inmediatamente después de conectar.
2. Qué respuestas necesita para mostrar la conexión como correcta.
3. Qué envía al entrar en un canal.
4. Qué utiliza para mostrar la lista de usuarios.
5. Qué manda al cambiar el topic.
6. Qué manda al cerrar la conexión.
7. Cómo representa mensajes privados.

Usa `nc` para pruebas exactas y HexChat para comprobar compatibilidad visual.

---

## 9. Diseñar el estado antes de programar la autenticación

### 9.1 `Client`

Un cliente existe desde que se acepta su socket, pero todavía no es un usuario IRC registrado.

Debe tener estados distinguibles:

```text
conectado
  └── password válida
        └── nickname definido
              └── USER recibido
                    └── registrado
```

Campos mínimos:

- fd.
- nickname.
- username.
- realname.
- host o datos para el prefijo.
- password validada.
- registro completado.
- buffer de entrada.
- buffer de salida.

### 9.2 `Server`

Debe ser el propietario lógico de:

- Password del servidor.
- Clientes conectados.
- Búsqueda por fd.
- Búsqueda por nickname.
- Canales existentes.
- Socket de escucha.
- `pollfds`.

### 9.3 `Channel`

Debe contener:

- Nombre.
- Topic.
- Miembros.
- Operadores.
- Invitados.
- Clave `+k`.
- Límite `+l`.
- Invite-only `+i`.
- Topic restringido `+t`.

Los canales almacenan referencias o punteros a clientes, pero el servidor debe eliminarlos antes de destruir el `Client`.

### 9.4 Propiedad y destrucción

Hay que decidir quién elimina cada objeto.

Una política simple es:

- `Server` crea y destruye `Client`.
- `Server` crea y destruye `Channel`.
- `Channel` nunca destruye `Client`.
- Antes de destruir un `Client`, se elimina de todos sus canales.
- No se vuelve a usar un puntero después de `removeClient()`.

Sin una política clara aparecerán punteros colgantes al ejecutar `QUIT`, `KICK` o cerrar un socket abruptamente.

---

## 10. Implementar el registro: PASS, NICK y USER

### 10.1 PASS

El servidor se inicia con una password. El cliente debe demostrar que la conoce.

Flujo:

```text
cliente: PASS secret
servidor: compara con la password configurada
servidor: marca password válida o devuelve 464
```

La password válida no significa que el usuario esté registrado. Solo completa una parte del registro.

### 10.2 NICK

El nickname es la identidad visible del usuario.

Hay que validar:

- Que existe el parámetro.
- Que cumple la política de caracteres.
- Que no está ocupado por otro cliente.
- Qué ocurre si el cliente ya tenía nickname y quiere cambiarlo.

Si el nickname está ocupado, normalmente se responde con `433`.

### 10.3 USER

`USER` suele tener esta forma:

```text
USER <username> 0 * :<realname>
```

Hay que guardar el username y todo el realname después de `:`.

### 10.4 Finalización del registro

El usuario queda registrado cuando se cumplen las condiciones acordadas:

```text
password válida && nickname válido && USER recibido
```

En ese momento se envía la bienvenida, normalmente empezando por `001`.

Debe emitirse una sola vez. Para lograrlo, el `Client` necesita un estado como `registered` y una función central que compruebe la transición.

### Orden correcto de implementación

1. Campos y getters/setters de `Client`.
2. Búsqueda de nickname en `Server`.
3. Parser de PASS/NICK/USER.
4. Comandos individuales.
5. Función `tryFinishRegistration()`.
6. Respuestas numéricas.
7. Pruebas en distinto orden.

---

## 11. Primer comando que modifica el dominio: JOIN

Un canal no es un chat gráfico. Es un conjunto de clientes y reglas almacenado en el servidor.

### Flujo de JOIN

Cuando Ana envía:

```text
JOIN #general
```

el servidor debe:

1. Comprobar que Ana está registrada.
2. Validar el nombre del canal.
3. Buscar `#general`.
4. Crear el canal si la política del proyecto lo permite.
5. Comprobar si es invite-only.
6. Comprobar si necesita clave.
7. Comprobar si está lleno.
8. Añadir a Ana como miembro.
9. Hacerla operadora si es la primera.
10. Enviar el mensaje JOIN a los miembros.
11. Enviar topic o indicar que no existe.
12. Enviar la lista de nombres.

### Por qué JOIN depende de todo lo anterior

JOIN necesita:

- El parser para leer `#general`.
- `Client` para conocer a Ana.
- `Server` para encontrar o crear el canal.
- `Channel` para membership y modos.
- La cola de salida para notificar a todos.

Por eso no tiene sentido implementar JOIN antes del transporte, parser y modelo de estado.

### Primer operador

Una política común es que el primer miembro del canal sea operador. Esto permite después probar TOPIC, MODE, INVITE y KICK.

---

## 12. PRIVMSG: comunicación privada y de canal

`PRIVMSG` demuestra que el servidor puede enrutar mensajes.

### Mensaje privado

```text
PRIVMSG luis :Hola Luis
```

Pasos:

1. Validar que el emisor esté registrado.
2. Separar destino y trailing.
3. Buscar a Luis por nickname.
4. Si no existe, devolver `401`.
5. Construir el prefijo de Ana.
6. Colocar el mensaje en la salida de Luis.

### Mensaje de canal

```text
PRIVMSG #general :Hola a todos
```

Pasos:

1. Buscar el canal.
2. Comprobar que Ana es miembro.
3. Construir un único mensaje IRC.
4. Enviarlo a los miembros apropiados, normalmente excluyendo al emisor.

### Errores que hay que decidir

- Sin destino o sin mensaje: parámetros insuficientes.
- Nickname inexistente: `401`.
- Canal inexistente: `403` o respuesta equivalente según la política.
- Emisor fuera del canal: `442`.

### Por qué es una prueba importante

Si PRIVMSG funciona, ya se han integrado:

- Parser con trailing.
- Búsqueda de usuarios.
- Búsqueda de canales.
- Broadcast.
- Buffers de salida.
- Prefijos IRC.

---

## 13. PART y QUIT: limpieza del estado

Aunque el foco del subject esté en otros comandos, no se puede construir un servidor fiable sin salidas correctas.

### PART

Un cliente abandona un canal, pero mantiene su conexión con el servidor.

Hay que:

1. Comprobar que el canal existe.
2. Comprobar membership.
3. Enviar PART a los miembros.
4. Retirar al cliente.
5. Retirar privilegios e invitaciones.
6. Eliminar el canal si queda vacío, según la política elegida.

### QUIT

El cliente abandona el servidor completo.

Hay que:

1. Construir el mensaje QUIT.
2. Notificarlo en todos sus canales.
3. Retirarlo de todos los canales.
4. Resolver qué ocurre si era el único operador.
5. Eliminar su fd de `pollfds`.
6. Cerrar el socket.
7. Destruir el `Client`.

### Desconexión sin QUIT

Si `recv()` devuelve cero o aparece `POLLHUP`, el servidor debe ejecutar la misma limpieza aunque el cliente no haya enviado `QUIT`.

---

## 14. TOPIC, INVITE y KICK

Estos comandos enseñan la diferencia entre pertenecer a un canal y tener privilegios dentro de él.

### TOPIC

Consulta:

```text
TOPIC #general
```

Cambio:

```text
TOPIC #general :Tema de hoy
```

Debemos decidir y aplicar:

- Si el usuario pertenece al canal.
- Si `+t` exige ser operador.
- Qué respuesta se da cuando no existe topic.
- A quién se notifica el nuevo topic.

### INVITE

```text
INVITE luis #general
```

El servidor debe:

1. Encontrar a Luis.
2. Encontrar el canal.
3. Comprobar que Ana tiene permiso.
4. Guardar que Luis está invitado.
5. Enviar `341` a Ana.
6. Enviar el mensaje INVITE a Luis.

La invitación no mete automáticamente a Luis en el canal. Solo le permite superar la regla `+i` cuando envíe JOIN.

### KICK

```text
KICK #general luis :motivo
```

El servidor debe:

1. Comprobar que Ana es operadora.
2. Comprobar que Luis está en el canal.
3. Notificar el KICK.
4. Retirar a Luis.
5. Retirar su privilegio y su invitación.

El motivo es un trailing y puede contener espacios.

---

## 15. MODE: implementarlo después de entender las reglas

`MODE` es el comando más delicado porque una misma orden modifica distintas categorías de estado.

### `+i` y `-i`: invite-only

- `+i`: solo pueden entrar usuarios invitados.
- `-i`: cualquier usuario puede entrar si supera las demás restricciones.

JOIN debe consultar este estado.

### `+t` y `-t`: topic restringido

- `+t`: solo operadores pueden cambiar topic.
- `-t`: cualquier miembro puede cambiarlo, según la política implementada.

TOPIC debe consultar este estado.

### `+k` y `-k`: clave

- `+k clave`: establece password de canal.
- `-k clave`: elimina la clave, según la sintaxis que se decida soportar.

JOIN debe comparar la clave recibida con la almacenada.

### `+o` y `-o`: operadores

- `+o nickname`: añade operador.
- `-o nickname`: retira operador.

Hay que comprobar que el usuario existe y pertenece al canal.

### `+l` y `-l`: límite

- `+l 20`: máximo de veinte miembros.
- `-l`: elimina el límite.

JOIN debe contar miembros antes de aceptar a otro.

### Cómo diseñar MODE

No conviene escribir un único bloque enorme sin estructura. El proceso debe ser:

1. Leer canal.
2. Leer cadena de modos.
3. Mantener si estamos sumando o quitando (`+` o `-`).
4. Consumir los argumentos necesarios según cada letra.
5. Validar permisos.
6. Aplicar cada modificación.
7. Construir la respuesta con los modos realmente aplicados.
8. Notificar al canal.

La dificultad no está en cambiar cinco booleanos. Está en que cada modo tiene argumentos, efectos y reglas distintas.

---

## 16. Comandos auxiliares para que el cliente funcione

El subject fija los comandos importantes, pero un cliente real puede necesitar otros.

### CAP

HexChat puede enviar `CAP LS`. Podemos responder indicando que no ofrecemos capacidades, si esa es la decisión del equipo.

### PING/PONG

Sirve para comprobar que la conexión sigue viva:

```text
cliente: PING token
servidor: PONG token
```

Es sencillo y muy útil para probar el loop.

### WHO y NAMES

HexChat los utiliza para saber quién está en un canal.

- `NAMES` muestra nicknames y operadores.
- `WHO` puede mostrar información más detallada.

### LIST, WHOIS y OPER

La guía de GitHub los presenta como ampliaciones útiles, pero no deben desplazar los requisitos obligatorios. Implementarlos solo después de tener estable el camino principal.

---

## 17. Cómo traducir la explicación a archivos del proyecto

Una organización compatible con el repositorio actual sería:

```text
main.cpp
includes/
    Server.hpp
    Client.hpp
    Channel.hpp
    IrcMessage.hpp          # puede añadirse
    IrcParser.hpp           # puede añadirse
    commands/
        Command.hpp
        CommandHandler.hpp
src/
    Server.cpp
    Client.cpp
    Channel.cpp
    IrcParser.cpp           # puede añadirse
    commands/
        CommandHandler.cpp
        PassCommand.cpp
        NickCommand.cpp
        UserCommand.cpp
        JoinCommand.cpp
        PrivmsgCommand.cpp
        TopicCommand.cpp
        InviteCommand.cpp
        KickCommand.cpp
        ModeCommand.cpp
        QuitCommand.cpp
```

### Orden de lectura del proyecto de ejemplo

1. `main.cpp`: entrada al programa.
2. `Server.hpp`: datos privados y servicios públicos.
3. `Server.cpp`: socket, `poll`, aceptar, leer, escribir y eliminar.
4. `Client.hpp`: identidad, autenticación y buffers.
5. `Channel.hpp`: miembros, operadores, invitados y modos.
6. `Command.hpp`: contrato común de comandos.
7. `CommandHandler`: dispatcher.
8. `PassCommand`, `NickCommand`, `UserCommand`.
9. `JoinCommand` y `PrivmsgCommand`.
10. `ModeCommand`, `TopicCommand`, `InviteCommand` y `KickCommand`.

El directorio `bircd/` es un ejemplo antiguo en C con `select()`. Ayuda a estudiar descriptores, pero no es la arquitectura final del proyecto.

---

## 18. Orden de trabajo real para el equipo

No repartiría los comandos al azar desde el primer día. Primero hay dependencias que todo el equipo debe comprender.

### Paso común 1: estudiar y dibujar

Los tres integrantes deben poder dibujar:

```text
socket servidor
    ↓ accept
socket cliente
    ↓ poll POLLIN
recv
    ↓
buffer de entrada
    ↓ línea completa
parser IRC
    ↓
dispatcher
    ↓
comando
    ↓
estado + buffer de salida
    ↓ poll POLLOUT
send
```

### Paso común 2: congelar contratos

Antes de trabajar en paralelo, acordar:

- Cómo se representa un `Client`.
- Cómo se representa un `Channel`.
- Cómo se entrega un mensaje parseado.
- Cómo se solicitan respuestas.
- Quién crea y destruye objetos.
- Qué función termina el registro.
- Qué funciones de `Server` pueden usar los comandos.

### Paso común 3: construir un camino vertical mínimo

En vez de que cada persona cree muchas clases aisladas, completar un recorrido pequeño:

```text
conectar
→ recibir PING
→ responder PONG
→ recibir PASS/NICK/USER
→ enviar 001
```

Cuando ese recorrido funciona, ampliar a:

```text
JOIN
→ crear canal
→ responder JOIN
→ PRIVMSG a canal
```

Después añadir permisos y comandos de canal.

---

## 19. Reparto recomendado entre tres personas

### Persona A: red y ciclo de vida

Primero debe entender sockets, `fcntl`, `poll`, `recv`, `send` y señales.

Implementa:

1. Validación de argumentos y arranque.
2. Socket de escucha.
3. `pollfds`.
4. Aceptación de clientes.
5. Buffer de entrada y lectura.
6. Buffer de salida y escritura parcial.
7. `POLLHUP`, `POLLERR` y desconexiones.
8. Eliminación segura de un cliente.
9. Destructor y cierre de recursos.

Es la responsable de que los comandos nunca necesiten tocar sockets directamente.

### Persona B: protocolo y registro

Primero debe entender sintaxis IRC, prefijos, trailing, códigos numéricos y la secuencia de HexChat.

Implementa:

1. Estructura del mensaje IRC.
2. Parser.
3. Formateadores de respuestas.
4. Dispatcher.
5. Estado de `Client`.
6. PASS.
7. NICK.
8. USER.
9. Finalización del registro.
10. CAP y PING/PONG si son necesarios.

Es la responsable de que una línea de texto se convierta correctamente en una petición interpretable.

### Persona C: dominio IRC

Primero debe entender el modelo de canales y la relación entre miembros, operadores e invitados.

Implementa:

1. Estado y operaciones de `Channel`.
2. JOIN.
3. PART.
4. QUIT a nivel de canales.
5. PRIVMSG.
6. WHO/NAMES.
7. TOPIC.
8. INVITE.
9. KICK.
10. MODE.

No debe cerrar sockets ni modificar `pollfds`.

### Integración

La integración debe hacerse por orden:

1. Red mínima.
2. Parser y PING/PONG.
3. Registro.
4. JOIN.
5. PRIVMSG.
6. Salidas y limpieza.
7. Permisos.
8. MODE y demás comandos.
9. HexChat.

Cada integración debe compilar y tener una prueba reproducible.

---

## 20. Pruebas guiadas por etapas

### Después del socket

- Arrancar con un puerto válido.
- Conectar varios clientes con `nc`.
- Cerrar uno y comprobar que los demás siguen conectados.

### Después de los buffers

- Enviar `NICK an` y después `a\r\n`.
- Enviar `PING a\r\nPING b\r\n` en una sola escritura.
- Enviar una orden sin `\r\n` y comprobar que no se procesa prematuramente.

### Después del parser

Probar:

```text
PING abc

NICK ana

USER ana 0 * :Ana García

PRIVMSG #general :Un mensaje con muchos espacios
```

Comprobar los campos resultantes, no solo que compile.

### Después del registro

- Password correcta e incorrecta.
- PASS después de NICK.
- USER antes de NICK.
- Nick duplicado.
- Bienvenida una única vez.

### Después de JOIN y PRIVMSG

Con dos o tres clientes:

1. Registrar Ana y Luis.
2. Ana entra en `#general`.
3. Luis entra en `#general`.
4. Ana escribe al canal.
5. Ana escribe a Luis.
6. Luis abandona.
7. Ana cierra la conexión.

### Después de permisos

Probar de forma positiva y negativa:

- Usuario normal intentando `MODE`.
- Operador cambiando `+i`.
- Usuario no invitado intentando entrar en `+i`.
- Entrada con clave incorrecta y correcta.
- Canal lleno.
- Cambio de topic con y sin `+t`.
- INVITE y JOIN posterior.
- KICK por operador y por usuario normal.

### Revisión final

Comprobar manualmente:

- Solo existe una llamada a `poll()`.
- No existe `select()` en la solución C++.
- No existe `fork()`.
- No se crean threads.
- Todas las lecturas y escrituras de sockets son no bloqueantes.
- Los comandos no llaman directamente a `send()`.
- No quedan clientes en canales después de destruirlos.
- No hay canales que conserven punteros inválidos.

---

## 21. Qué significa que el proyecto esté terminado

La parte mandatory no está terminada porque compile. Está terminada cuando se puede explicar y demostrar este recorrido:

1. Un cliente conecta.
2. El servidor acepta el socket sin bloquear a otros clientes.
3. Los bytes se acumulan correctamente.
4. Una línea IRC se parsea sin perder el trailing.
5. PASS, NICK y USER registran al usuario.
6. El servidor envía una bienvenida válida.
7. El cliente entra en un canal.
8. El primer usuario obtiene los privilegios definidos.
9. Varios usuarios reciben mensajes correctamente.
10. Los permisos de topic, invitación, kick y modos se respetan.
11. `send()` parcial no pierde respuestas.
12. `QUIT`, `PART` y cierres abruptos limpian todo el estado.
13. HexChat puede utilizar el servidor de manera estable.

Solo entonces tiene sentido estudiar el bonus. En particular, la transferencia DCC la gestionan normalmente los clientes directamente; el bot sí requiere decidir si será un cliente externo o una entidad interna del servidor.

---

## 22. Regla práctica para cualquier nueva función

Antes de programar una función o comando, responder siempre:

1. ¿Qué mensaje exacto recibe el servidor?
2. ¿Cómo se parsea?
3. ¿Qué datos del estado necesito consultar?
4. ¿Qué condiciones hacen que falle?
5. ¿Qué estado cambia si tiene éxito?
6. ¿Quién recibe cada respuesta?
7. ¿Cómo se encolan esas respuestas?
8. ¿Qué ocurre si el cliente se desconecta en mitad del proceso?
9. ¿Qué prueba demuestra que funciona?

Si estas respuestas están claras, escribir el código suele ser una traducción directa del diseño. Si no están claras, todavía falta entender el comportamiento antes de implementar.
