# Client

## Objetivo

`Client` representa una conexión TCP aceptada por el servidor. Guarda el estado propio de un usuario y sus buffers, pero no crea sockets, no ejecuta comandos y no conoce canales.

## Cuándo aparece

`Client` se crea después de que `Server` recibe `POLLIN` en el socket de escucha:

```text
poll() -> accept() -> Client(clientFd)
```

Antes de aceptar la primera conexión no existe ningún `Client`.

## Entrada del constructor

```cpp
Client(int fd);
```

`fd` es el descriptor devuelto por `accept()` y debe ser válido para esa conexión.

Ejemplo:

```text
serverFd = 3
clientFd = 4
Client(4)
```

## Estado inicial

El constructor debe dejar el objeto en este estado:

```text
_fd                = fd
_host              = ""
_nickname          = ""
_username          = ""
_realname          = ""
_passwordAccepted  = false
_registered        = false
_inputBuffer       = ""
_outputBuffer      = ""
```

## Orden de construcción

1. Declarar los miembros privados.
2. Mantener constructor por defecto, copia y asignación fuera de la API pública.
3. Implementar `Client(int fd)` con una lista de inicialización completa.
4. Implementar el destructor.
5. Implementar getters constantes.
6. Implementar setters de identidad y registro.
7. Implementar acceso a los buffers.
8. Implementar `appendOutput()`.

## Responsabilidades

### Identidad

- `getFd()` devuelve el descriptor.
- `getHost()` devuelve el host conocido.
- `setNickname()`, `setUsername()` y `setRealname()` actualizan la identidad IRC.

### Registro

- `setPasswordAccepted()` modifica el resultado de `PASS`.
- `setRegistered()` indica que `PASS`, `NICK` y `USER` ya completaron el registro.
- `hasAcceptedPassword()` e `isRegistered()` solo consultan el estado.

### Buffers

Los bytes recibidos pueden llegar fragmentados o contener varias líneas. Por eso el buffer de entrada debe sobrevivir entre iteraciones de `poll()`.

```text
recv() -> inputBuffer -> Parser
```

Las respuestas no se envían directamente desde `Client`. Se acumulan:

```text
Replies -> appendOutput() -> outputBuffer -> send()
```

## Lo que no debe hacer

- No llamar a `socket()`, `recv()`, `send()`, `close()` ni `poll()`.
- No interpretar comandos IRC.
- No conocer `Channel` ni permisos.
- No decidir qué respuesta debe producir un comando.

## Flujo de ejemplo

```text
1. accept() devuelve fd 4.
2. Server crea Client(4).
3. Client recibe PASS: passwordAccepted = true.
4. Client recibe NICK y USER.
5. Server marca registered = true.
6. Parser consume inputBuffer.
7. Replies añade una respuesta a outputBuffer.
8. Server envía outputBuffer cuando POLLOUT está activo.
```

## Pruebas mínimas

- Crear `Client(4)` y verificar valores iniciales.
- Cambiar nickname, username y realname y leerlos.
- Verificar que el estado de contraseña comienza en `false`.
- Añadir dos respuestas y comprobar que `outputBuffer` conserva el orden.
- Añadir una orden incompleta a `inputBuffer` y comprobar que no se pierde entre lecturas.
- Cerrar una conexión y comprobar que `Server` elimina el cliente de sus mapas y de `_pollfds`.
