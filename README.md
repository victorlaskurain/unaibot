# Unaibot #

This project  is a  remote controller for  any robot  with independent
propulsion for the  left and right wheels: most  tricycle style robots
or any caterpillar robot, among others.

The code is currently being use to remotely control a robot build
around the
[Merkur Toys robot chassis 01](http://eshop.merkureducation.cz/show_product/?id=QuLuudfk8g).

I started  the project with a  double objective. In the  first place I
wanted  to  build a  remote  controlled  robot  for my  children.  The
secondary   objective  was   learning   a  little   bit  about   (AVR)
microcontroller programming. It's  been published in the  hope that it
will be  a good  starting point  for people sharing  any of  these two
goals.

## About this file ##

This file is expected  to be useful as a guide to  the code for anyone
willing to understand  how the whole setup works and  how to customize
it.

## High level overview ##

The hardware setup is as follows:

- A smartphone is the remote controller.
- A Raspberry Pi together with an Arduino board make up the controller.
- The chassis is a Merku Toys robot chassis 01.
- Optionally the robot can stream what it sees through a webcam.

This components  form a stack and  communicate with each other  in the
following way:

- The chassis has two motors, one for the left caterpillar and one for
  the right caterpillar.
- The Arduino UNO board controls the motors with a Motor Shield v1 and
  reads commands from the serial port.
- A  Raspberry Pi sends commands  setting motor speeds to  the arduino
  using the serial port and serves  the web that implements the remote
  control.  It  gets remote  commands  through  a websocket.  It  also
  streams the webcams video through a second websocket.
- The smartphone loads the remote control interface from the Raspberry
  pi, processes user  input and  sends commands  to the  Raspberry via
  websockets. It also renders the video stream if available.

The  user  interface running  on  the  smartphone  is plain  HTML  and
Javascript.

The  implementation of  the  web server  and the  server  side of  the
websockets is Nodejs based.

The code  running on the  Arduino uses the  AVR libraries but  not the
Arduino  libraries. The  reason not  to use  the Arduino  libraries is
twofold:

- Being very high  level and focused on being easy  to use for novices
  they  get in  the way  of  learning how  the microcontroller  really
  works.
- Although this is more a personal opinion rather than a fact based on
  rigorous measurements,  they are not  as type safe and  efficient as
  the might.

The following sections describe the  implementation details of each of
the aforementioned components.

# Motor control #

The motor driver implementation consists of several submodules:

- serial: implementation of a serial  port driver for the HW serial of
  the atmega328p.
- registers: type  safe interface to access the  hardware registers of
  the atmega328p.
- shift_register: implementation of a driver for a 74HCT595N IC.
- command:  implementation of  a command dispatcher.   robot_main uses
  this module to  parse and process the commands read  from the serial
  port  in an  organised  way  (currently there  is  a single  command
  defined so this is a bit of an overkill).
- pwm: implementation of the driver for the HW PWM of the atmega328p.
- motor\_driver: implementation  of a controller for  the Motor Shield
  v1. Uses the shift\_register module and the pwm module.
- robot_main: main   program,  uses serial,  command an  motor\_driver
  modules  and  implements  the   main  program  that  the  atmega328p
  executes.

## Serial port driver ##

### Quickstart ###

To use the serial just instantiate one of the available clases:

- `serial_9600`: unbuffered  read and write support at  9600 baud. All
  operations are synchronous, meaning that they take place in the main
  loop busy waiting as necessary.
- `serial_19200`:   unbuffered   read  and  write   support  at  19200
  baud. Same as the previous one, only faster.
- `serial_9600_async`: buffered  read and write support  at 9600 baud.
  All  operations are  asynchronous, meaning  that the  implementation
  uses interrupts. Reading  does not block as long as  the read buffer
  is not  empty. Conversely,  writing does  not block  as long  as the
  write buffer is not full.
- `serial_19200_async`:  buffered  read  and  write  support at  19200
  baud. Same as the previous one, only faster.

These are  just some of  the available  classes. There are  some other
options and more can be constructed on demand (see next section).

Async  versions   are  generally  more  efficient   but  require  more
memory. The memory requiremente derives  from the need to allocate the
buffers and the interruption handling routines.

In all cases parity and stop bits are the hardware's defaults.

The basic operations are:

- `inline uint8_t read()`: read one byte.
- `inline void write(const uint8_t byte)`: write one byte.

There are also versions to read and write whole buffers which could be
more  efficient than  plain old  loop.  There  is also  a set  of free
function  templates which  are  the primary  public  interface to  the
serial port classes:

```c++
template <typename Serial>
uint8_t read(Serial &s);

template<typename Serial, typename SizeType>
inline SizeType read(Serial &s, uint8_t *buff, SizeType size);

template<typename Serial, typename SizeType>
inline SizeType read_line(Serial &s, uint8_t *buff, SizeType size,
                          uint8_t mark = '\n');

// write message and wait for enter key
template<typename serial_t>
void wait(serial_t &ser, const char* msg = 0);
```

### Implementation ###

The  serial  port  driver  is  a class  template  with  the  following
signature:

```c++
template<typename SerialSpeed,
         typename SerialReadMode,
         typename SerialWriteMode>
class serial : public SerialReadMode, public SerialWriteMode [...]
```

It  follows a  simple  policy  based design  where  each  of the  type
parameters represents a policy:

- `SerialSpeed`:  initialises the  serial port to  a given  baud rate,
  parity and stop  bit settings. The currently  available options only
  define different  baud rate  settings leaving  parity and  stop bits
  settings to the hardware's defaults.
- `SerialReadMode`: defines how to read bytes from the serial port. It
  can be set to `not_available` to instantiate a write only serial
  port. This could be useful to save some memory.
- `SerialWriteMode`: the  same   as `SerialReadMode`,  only for  write
  operations.

## Type safe and efficient register access ##

## Shift register driver ##

## Command processor ##

## PWD driver ##

## Motor driver ##

## Main program ##

# Web based remote control #

## User interface ##

## Remote API ##

### Simple query / response protocol ###

### Asynchronous events ###

## Webcam streaming ##
