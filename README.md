Alfonso Vila - MissileRider 2021

# Material
* Arduino Pro Micro 5V: Si fuera la versión 3.3v no haría falta regulador de tensión adicional.
* BNO055: Sensor. Si no tiene regulador incorporado (como mi CJMCU-055) es necesario el regulador. Es necesario que funcione en modo I2C, por lo que en el CJMCU-055 hay que soldar S0 y S1 a "-" (ver manual si es necesario!).
* LM1117-3.3V: Regulador de tensión a 3.3v. Sólo si es necesario.

## URLs de material para vagos
**(Ojo, revisar precios!)**
-  Arduino Pro Micro 5v [(Aliexpress)](https://es.aliexpress.com/item/32822216726.html?spm=a2g0o.productlist.0.0.60c161ffkTJm2X&algo_pvid=88f708c6-e26f-44d0-b3bd-03d78fec5f8c&algo_expid=88f708c6-e26f-44d0-b3bd-03d78fec5f8c-27&btsid=2100bddd16140388959056007e2b0e&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_) [(Amazon)](https://www.amazon.es/ARCELI-Atmega32U4-Bootedered-Desarrollo-Microcontrolador/dp/B07J2Q3ZD5/ref=sr_1_5?__mk_es_ES=%C3%85M%C3%85%C5%BD%C3%95%C3%91&dchild=1&keywords=pro+micro+5v&qid=1614039024&sr=8-5)
- BNO055 [(Aliexpress)](https://es.aliexpress.com/item/1005001860541569.html?spm=a2g0o.productlist.0.0.2fd52dfbbE1aNq&algo_pvid=14fcc3ed-bf50-4ecd-9fb4-e3d2ad61d043&algo_expid=14fcc3ed-bf50-4ecd-9fb4-e3d2ad61d043-1&btsid=2100bdd816140390564453496ebba1&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_)
- LM1117-3.3v [(Aliexpress)](https://es.aliexpress.com/item/1005001265813037.html?spm=a2g0o.productlist.0.0.5e5a5b86Wzt34K&algo_pvid=5f505d78-fc54-4474-b8ff-bb9b0ef01c0a&algo_expid=5f505d78-fc54-4474-b8ff-bb9b0ef01c0a-9&btsid=0b0a119a16140392517081573e89f8&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_)
- Botón [(Aliexpress, versión 6x6x3.1mm)](https://es.aliexpress.com/item/1005001298009129.html?spm=a2g0o.productlist.0.0.7ad643f1rM0ev7&algo_pvid=dd5050ae-8298-467e-b525-ae5c1d99b640&algo_expid=dd5050ae-8298-467e-b525-ae5c1d99b640-37&btsid=0b0a187916140393226384863ef249&ws_ab_test=searchweb0_0,searchweb201602_,searchweb201603_)

# Diagrama de conexión

| Pro Micro | BNO055 | Regulador | Botón |
| --------- | ------ | --------- | ----- |
| PIN 2     | SDA    |           |       |
| PIN 3     | SCL    |           |       |
| PIN 10    |        |           | PIN 2 |
| GND       |        |           | PIN 1 |
| VCC       |        | VCC IN    |       |
| GND       |        | GND IN    |       |
|           | VCC    | VCC OUT   |       |
|           | GND    | GND OUT   |       |


Notas para subir el sketch: Juntar RST y GND (y soltar) para reiniciar mientras se intenta subir el programa en el PC.

# Manual de uso

## Uso normal

Tras conectar se recupera la calibración más reciente. Al pulsar el botón se toma como nueva referencia de origen la posición actual. Debe utilizarse cuando se descentre (que pasa, sobre todo cuando lleva poco encendido).

Existen cuatro botones en el joystick que se presenta. Estos botones representan la calibración de cada una de las partes del BNO055:

| Botón 1 | Botón 2 | Botón 3 | Botón 4 |
|-|-|-|-|
| Sistema | Giróscopo | Acelerómetro | Magnetómetro |

Si el botón se encuentra pulsado la calibración es *imperfecta* y puede mejorarse. En el caso del botón 1 (calibración del sistema), se encenderá y apagará cuando le pete.

Al pulsar el botón físico se alterna entre el encendido de estos botones físicos a modo de aviso y mantener todos los botones sueltos, para facilitar el calibrado o asignación de botones en los juegos.

## Calibración del sensor

- Giróscopo: Con movimientos normales se calibrará él solito
- Acelerómetro: Se debe posar y rotar de 45ª en 45ª hasta completar una vuelta completa sobre eje X o eje Y
- Magnetómetro: Necesita una vuelta completa sobre Z para ver norte, sur, ese y oeste
- Sistema: cuando lo demás esté bien, sistema estará bien (o no)

## Modo de calibración del joystick

Para facilitar la calibración se puede mantener pulsado el botón físico al conectar la gorra. La vista quedará siempre centrada para calibrar el centro. Para las posiciones máximas y mínimas se debe pulsar el botón de forma que los ejes se moverán de forma automática a posiciones extremas. Todo fácil.

Para devolverlo al modo de uso normal es necesario desconectarlo y volver a conectarlo.

