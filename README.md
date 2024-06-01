# Proyecto_redes_n-1


# Cuatro en Línea - Juego Cliente/Servidor

Este es un proyecto implementado en C++ que simula el juego Cuatro en Línea utilizando el modelo Cliente/Servidor y sockets para la comunicación entre los procesos. El servidor gestiona el juego y permite múltiples conexiones de clientes simultáneamente, mientras que el cliente se conecta al servidor para jugar.

## Objetivos

- Utilizar diversos mecanismos relacionados con los procesos, incluyendo la creación y finalización de procesos, así como los mecanismos de comunicación.
- Definir e implementar un protocolo para la capa de aplicación.
- Utilizar el protocolo orientado a la conexión TCP de la capa de transporte.

## Funcionamiento

### Cliente

El cliente se conecta al servidor y sigue estos pasos:

1. Conexión al servidor.
2. Visualización del tablero.
3. Espera de la indicación del servidor sobre quién comienza.
4. Juego:
   - Indicación de la columna donde dejar caer la ficha.
   - Espera y visualización del resultado.
   - Repetición de los pasos anteriores hasta que haya un ganador o un empate.

### Servidor

El servidor se levanta y espera por conexiones de clientes, luego sigue estos pasos para cada jugador conectado:

1. Selección aleatoria del jugador que comienza el juego.
2. Solicita la columna o selecciona una columna aleatoria según corresponda el turno.
3. Verificación de las jugadas.
4. Envío de resultados al cliente.

### Consideraciones

- Tamaño del tablero: 6 filas por 7 columnas.
- Se utilizan letras S para el servidor y C para el cliente para identificar las fichas de los jugadores.
- El cliente debe permitir indicar la dirección IP y el puerto del servidor al momento de ejecutarlo.

## Ejecución

Para ejecutar el servidor:

$ ./a.out [puerto]


$ ./a.out [IP del servidor] [puerto]

## Salida por la Terminal

La salida por la terminal proporciona información sobre el estado de las conexiones y el progreso del juego tanto para el servidor como para los clientes.

## Normas Básicas

- Se incorporan comentarios que facilitan la comprensión del código.
- El código está tabulado.
- Los nombres de clases, atributos y funciones/métodos son descriptivos.
- El código está orientado a objetos.
