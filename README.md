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

## How to build ##

The recommended  way to build  the project is  to execute make  in the
directory where the makefile is located.

```
make
```

This will create  a build directory alongside the  makefile which will
contain all the built files, both  the final binaries and the temporal
artifacts.

### About the build system ###

The  top  level  makefile  is a  non-recursive  makefile.   Using  the
utilities  provided by  the  top level  makefile  defining modules  is
straightforward.  A  module can  be  either  a  static library  or  an
executable:

Defining a module requires three steps:

1. Create a directory for the module, for example `registers`.
2. Put  the source files  in the directory. It  is advised to  put the
   header files in a a subdirectory lik `registers/include`.
3. Add   the   module.mk   module  definition   file,  in   this  case
   `registers/module.mk`.

All the  module definition files  are pretty much alike.  For example,
this is the module definition for the `registers` module:

```make
local_src  := $(wildcard $(src_subdirectory)/*.cpp)
$(eval $(call make-library, $(subdirectory)/libshift_register.a, $(local_src)))
```

The first line stores  the list of all the source  files of the module
in  a  variable. `src_subdirectory`  is  a  handy make  function  that
returns the source directory.

The second line adds the  library `libshift_register.a` to the list of
libraries to  build.  Make  will use  the source  files listed  in the
variable `local_src`  to build the library.  `subdirectory` is similar
to `src_subdirectory` but  returns the build directory  instead of the
source directory.

The module  definition file for  an executable is very  similar. Let's
take for example the module definition of `robot_main`:

```make
local_src  := $(wildcard $(src_subdirectory)/*.cpp)
$(eval $(call make-program, $(subdirectory)/program, $(local_src), command/libcommand.a motor_driver/libmotor_driver.a pwm/libpwm.a shift_register/libshift_register.a serial/libserial.a))
```

As  you  can  see  the  only  noticeable  different  is  the  call  to
`make-program` instead  of `make-library` and the  fact that libraries
on which the program depends must be specified as third parameter.

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
  streams the webcam's video through a second websocket.
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
more efficient  than a plain  old loop.  There is  also a set  of free
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

The atmega328p  presents all the  control registers and all  the input
and  output registers  mapped in  the memory  space. The  AVR avr/io.h
defines symbolic names for all  the registers as memory addresses. For
example, io.h defines the B io port  (PORTB) as 25h.  In turn the same
file defines each of this port's  bits as plain integers: PORTB0 is 0,
PORTB1 is 1, etc. This is not type safe, nothing prevents accidentally
mixing register PORTB with bit PORTC3, for example.

registers.hpp  defines  different  types  to  represent  each  of  the
registers uses the template class pin\_t to define different types for
each of the pins and tie  them to their respective register types. The
pin\_t template class defines a set  of functions to set and clear the
bits it represents in a safe way.

This is the interface of pin_t:

```c++
template <typename param_port_t, int pin_number>
struct pin_t
{
    typedef param_port_t port_t;
    inline static int offset();
    inline static void set();
    inline static void clear();
    inline static void set_mode_input();
    inline static void set_mode_output();
};
```

Using registers.hpp clearing the bit number 2 of PORTB turns from:

```
PORB &= ~_BV(PORTB2)
```

into the safer and more expressive:

```
PORTB2_t::clear();
```

The first  version is error  prone because if  we forget say  "~", the
compiler  won't complain  at  all  but the  result  won't  be the  one
expected. In other words, writing incorrect  code is very easy and the
compiler won't  help us.  With the  second option  the intent  is more
clear an writing wrong code is harder.

Moreover, the  implementation of  registers.hpp achieves this  with no
computational   cost.  Compiling   with  optimiseations   enabled  the
following two functions produce exactly the same assembler code:

```c++
uint8_t f1()
{
    PORTB |= _BV(PORTB0);
    PORTB &= ~_BV(PORTB0);
    return -1;
}

uint8_t f2()
{
    PORTB0_t::set();
    PORTB0_t::clear();
    return -1;
}
```

## Shift register driver ##

The  shift_register module  implements a  driver for  the the  74HC595
shift register integrated circuit. The API is a single template class:

```c++
template<typename shift_clk,
         typename store_clk,
         typename output_enable,
         typename ds,
         typename bit_field_t,
         size_t bit_field_size = 8 * sizeof(bit_field_t)>
struct shift_register
{
public:
    shift_register();
    ~shift_register();
    bit_field_t data;
    inline void commit(bit_field_t d)
    inline void commit() const
private:
    shift_register(const shift_register&)                  = delete;
    shift_register(shift_register&&)                       = delete;
    shift_register& operator=(const shift_register &other) = delete;
    shift_register& operator=(shift_register &&other)      = delete;
};

```

These are the template's parameters:

- shift\_clk: the  shift clock pin. A raising edge  in this pin shifts
  the existing bits of the shift register and stores a new bit.

- store\_clk:  the store clock pin.  A raising edge in  this ping puts
  the value of the value of the shift register into the store register
  (the parallel output).

- output_enable: if this pin is high then the output pins will assume
  the high impedance - OFF state.

- ds: the  rising edge of the  shift_clk takes the new  value from the
  value present at this pin.

- bit\_field\_t: the  data type used to represent in  memory the value
  of  the shift  register. Any  type should  be usable  as long  as it
  supports the binary and (&) and  shift bits left (<<) operations. In
  practice integer uint8_t has ever been used.

- bit\_field\_size: the number of  bits in bit\_field\_t. It takes the
  right value depending  on the size of bit\_field\_t, no  need to set
  it manually.

shift\_register expects the  pins to be represented  using an instance
of the  pin\_t described in the  previous section so an  example usage
would be the following:

```c++
#include <vla/shift_register.hpp>
#include <stdint.h>

using namespace vla;

int main(void) {
    // define de type and an instance
    shift_register<PORTD4_t/* arduino pin  4 */,
                   PORTB4_t/* arduino pin 12 */,
                   PORTD7_t/* arduino pin  7 */,
                   PORTB0_t/* arduino pin  8 */,
                   uint8_t> r;
    r.data = 0x81; // clear shift register
    r.commit();    // transfer the value to the parallel outuput
    while(true);   // shift register should show 0x81
}

```

## Command processor ##

The command processor  module provides a simple  framework to dispatch
command to handler  functions. In this context the  commands are plain
strings and the handlers have the following signature:

```c++
uint8_t operator()(const char* str)
```

The dispatcher calls each handler in order and until one of them
returns 0.

There is  no provision  to add handlers  dynamically, instead  all the
handlers   must    be   added   at    the   same   time    using   the
```make_command_dispatcher```  which  takes  any  number  of  callable
objects (functors, plain functions, ...) and returns a dispatcher:

```c++
template<typename ...Ts>
command_dispatcher<Ts...> make_command_dispatcher(const Ts&... args)
```

## PWD driver ##

### Quickstart ###

To  use  one of  the  4  PWD pins  supported  instantiate  one of  the
available classes:

-   ```pcpwm_portd6```,   ```pcpwm_portd5```,  ```pcpwm_portb3```   or
  ```pcpwm_portd3```.

For example:

```c++
// instantiate and configure timer
vla::timer0_pcpwm timer0;
timer0.set_clock(vla::clock_source::PRESCALE_1);
// set up PWM on pin d6
vla::pcpwm_portd6 pwm0a(timer0);
// set 50% duty cycle
pwm0a.set_level(UINT8_MAX / 2);
```

The implementation relays  on two templates.  In the  first place, the
```timer_configuration```  template  class   packs  together  the  two
registers used  to configure any  of the  two supported 8  bit timers.
For  example   the  ```timer0_pcpwm```  instantiation  helps   in  the
configuration of the 0 timer by binding together the two timer control
registers (```TCCR0A_t``` and ```TCCR0B_t```)  and the timer mode.  It
also  provides  a method  (```set_clock```)  to  change the  prescaler
(frequency) programmatically.

In  the second  place,  the  ```pwm``` template  class  binds a  timer
configuration  together with  the comparator  register and  the output
pin. This  class takes the  timer type  as template parameter  and the
timer instance  as constructor parameter.  The later's function  is to
ensure  that  the  user  doesn't  try to  use  a  PWM  output  without
configuring the timer first.

## Motor driver ##

This   module   is  an   driver   implementation   for  the   Adafruit
Motor/Stepper/Servo  Shield  for  Arduino.  The  shield  supports  two
stepper motors  or four DC  motors. This  driver supports only  the DC
motor configuration.

The shield uses a shift register  to set the turning direction of each
of the  four motors. The  implementation relays on the  shift register
module to  handle the shift  register and the  PWM module to  set each
motors power level.

The API is pretty straighforward:

```c++
class motor_driver
{
public:
    motor_driver();
    ~motor_driver();
public:
    motor0_t motor0;
    motor1_t motor1;
    motor2_t motor2;
    motor3_t motor3;
};
```

Once  instantiated  each of  the  four  motors  is available  via  the
corresponding  member  variable. The  type  of  each of  these  member
variables is an instantiation of  a class template. The class template
provides the  following public  member functions, which  hopefully are
self explanatory.

```c++
enum class direction_t : uint8_t
{
    IDLE      = 0x00,
    FORWARD   = 0x01,
    BACKWARD  = 0x02,
    STOP      = 0x11,
    FIRST     = IDLE,
    LAST      = STOP
};

void set_direction(direction_t d);
void set_speed(uint8_t s);
```

## Main program ##

The main program simply reads commands  from the serial port using the
serial port driver, parses them using the command dispatcher and sends
the corresponding lower  level commands to the motor  shield using the
motor driver.

```c++
int main()
{
    const auto CMD_BUFFER_SIZE = 64;
    uint8_t cmd_buffer[CMD_BUFFER_SIZE], err_code;
    serial_9600 ser;
    motor_driver motors;
    auto dispatch = make_command_dispatcher(set_speeds_cmd(motors));
    write_line(ser, "READY");
    while(true) {
        read_line(ser, cmd_buffer, CMD_BUFFER_SIZE, '\r');
        err_code = dispatch(reinterpret_cast<const char*>(&cmd_buffer[0]));
        if (SUCCESS == err_code) {
            write_line(ser, "OK");
        } else {
            write(ser, "ERROR ");
            write_line(ser, hex(err_code));
        }
    }
}
```

# Web based remote control #

The  web based  remote control  is  in the  web_control directory.   A
description  of  the  most  significant  contents  of  this  directory
follows:

- `bootstrap.sh`:   shell   script   to   install  all   the  required
  dependencies.

- `install.sh`:  shell script  to install the  remote controller  as a
  systemd service.  Copies  unaibot_web.js file and the  html, lib and
  node_modules  directories  into   `/opt/unaibot`  and  configures  a
  systemd service to run the the server.

- `lib/unaibot.js`:   it's  a   node   module.  Exports   the  UnaiBot
  constructor which takes as parameters the serial port and optionally
  its  configuration.  Instances of  this  object  provide high  level
  interface  for serial  port communication  between the  main program
  running  on the  Arduino  and the  world, in  this  case the  remote
  controller.

- `lib/botcam.js`: it's a node  module. Exports the BotCam constructor
  which takes as  paramenter the name of a video  device. Instances of
  this object provide the means  to start/stop the capture process and
  signal   the  activation/deactivation   of   the   camera  and   the
  availability of new captured frames.

- `unaibot_web.js`:  this is  the main program,  serves the  HTML page
  that  implements   the  remote's   GUI,  handles  the   server  side
  implementation of the remote's  commands and manages the websocket's
  server side.

## User interface ##

The user interface  is a multitouch web page which  shows two vertical
walls, one at each border.  The left war controls the left caterpillar
while the right  bar controls the right one. Pressing  on the top half
of the  bar lets  the caterpillar  run forward  while pressing  on the
bottom half lets it run backward. The round button at the center allow
activating and deactivating the camera.

A  couple  of Javascript  files  implement  touch event  handling  and
client/server communication.

- `api.js`: implementation of the client side of the protocol.

- `main.js`:  touch  event and  video  activation/deactivation   event
  handling.

## Remote API ##

At page  load the browsers opens  a connection to the  command socket.
Automatically reconnects if the connection breaks for any reason.

Two  types of  messages travel  through the  command socket.  The most
common are the requests and their  responses. The server can also send
asynchronous events to the browser.

There is a  second WebSocket endpoint handled by the  same node server
to stream the webcams video.

### Simple query / response protocol ###

The client  sends the request  and receives responses as  JSON encoded
Javascript objects.

The requests have the following properties:

- id: identifies  each request and must be unique  during the lifetime
  of a connection.

- type: identifies  the type of the message which  the server uses for
  message dispatching.

- data: if  the  messages requires  any  more  information  it can  be
  encoded in this property. The type field dictates the interpretation
  of this property.

The responses have the following properties:

- id: identifies each response, the value is equal to the value of the
  id field of the request to which this response answers.

- data:  the response data. The  type of the requests  dictates how to
  interpret this field and whether it is present at all.

### Asynchronous events ###

Asynchronous events  originate at  the server. The  server is  free to
send events  to the client  at any time.  The events are  JSON encoded
Javascript objects with the following properties:

- evt: a string with the event name. The client library triggers a DOM
  event to communicate the event to any interested listeners.

## Webcam streaming ##

[Jsmpeg](https://github.com/phoboslab/jsmpeg) provides  the Javascript
library to  render MPEG1 into a  canvas element. It also  provides the
necessary framework for  transmitting the vide frames  from the server
to the  client live using  websockets as transport. Although  it might
not be the most efficient codec the whole setup is very simple.

Client side, this three lines of Javascript direct the video player to
connect to the the given  websocket address start playback. The player
just sits there waiting for any video frames to render.

```javascript
var canvas = document.getElementById('video-canvas');
var url = 'ws://'+document.location.hostname+':8080/unaibot/cam';
var player = new JSMpeg.Player(url, {canvas: canvas});
```

Server side three different pieces of code work together to stream the
video.

On the first place the streaming process uses ffmpeg to initialize the
video  device,  capture raw  frames  and  encode  then in  the  format
required by Jsmpeg.

On the  second place there  is of course there  is the handler  of the
`/unaibot/cam` websocket endpoint.  Its  work is accepting connections
and broadcasting the video stream to all connected clients.

On the third place, the `botcam.js` module provides a simple interface
to the video capture device tailored to the needs of Jsmpeg. It how to
run ffmpeg encoder with the appropriate parameters. The module exports
a constructor (BotCam) whose instances have two public methods:

- capture(bool): activates the video  capture (true) or deactivates it
  (false).

- isVideoOn: returns true is the video capture is active.

and emit three events:

- videoOff:  notifies that the video  capture is off.  If  the library
  emits this event before the videoOn  event, then that means that the
  capture could not initialized for  whatever reason (no access to the
  device,  wrong ffmpeg  configuration or  any other  reason). If  the
  library emits this event after  a call to `capture(false)` it simply
  means that the capture shutdown process worked. The library can emit
  this event  at any other time,  which usually means that  the webcam
  got disconnected or for some reason the ffmpeg command died.

- videoOn: notifies that the video capture is on.

- data: when the video capture is  on the video data is made available
  to any interested listeners using  by emitting this event. The event
  object is  in the format required  by Jsmpeg so that  the web socket
  just needs to forward it to the remote clients.
