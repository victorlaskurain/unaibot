#!/usr/bin/python3

import curses
from curses.textpad import Textbox
from curses import panel

import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.WARNING)

from pymodbus.client.sync import ModbusSerialClient
from pymodbus import utilities
import os
import time
import sys

READ_COILS    = 0
READ_COUNTERS = 1
READ_ANALOG   = 2
READ_OP_COUNT = 3

COLOR_WHITE_ON_RED_ID = 1
COLOR_WHITE_ON_RED = None # initialize after curses

SERIAL_PORT = '/dev/ttyACM0'
UNIT_ID     = 0x76
HELP_MSG    = """
BIT DESCRIPTION

- AU: 1 if the pin is available for the user. Read only.
- AI: 1 if the pin supports analog input. Read only.
- AO: 1 if the pin supports analog output. Read only.
- IOD: Input Output Direction. Set to use this pin as output, clear to
  use it as input.
- IOM: Input Output Mode. Set to use  pin for analog I/O. Clear to use
  the pin for digital IO. Subject to the value of IOD.
- EIP:  Enable  Internal pullup.  Set  to  activate internal  pull  up
  resistor. Setting EIP automatically clears IOD.
- EC: Enable Counter. Set to enable 16 bits counter linked to the pin
- PI: Pin Input. Read only. The current digital value of the pin.
- PO: Pin Output.  Output value of the  pin. Ignored if IOD  is set to
  input.

USER INTERACTION

- Use cursor keys to change the selected bit.
- Type 1 to set, 0 to clear or t to toggle the selected bit.
- Type h to display help.
- Type q to quit
        """.strip()

class Table:
    def __init__(self, definition, window = None, data = []):
        self.row_count   = definition['row_count']
        self.col_formats = definition['col_formats']
        self.header      = definition.get('header', None)
        self.header_h = 0
        if self.header:
            self.header_h = 1 + max([
                len(header_txt.split('\n'))
                for header_txt in self.header])
        y, x = 0, 0
        self._calculate_positions()
        if not window:
            self.window = window or curses.newwin(self.h, self.w, y, x)
            self.panel = panel.new_panel(self.window)
        else:
            self.window = window
        self._draw_lines()
        self.set_data(data)
    def _calculate_positions(self):
        col_count = len(self.col_formats)
        col_widths = [len(f.format('')) for f in self.col_formats]
        offset = 1
        text_ranges = []
        for i, width in enumerate(col_widths):
            text_ranges.append((offset, width, self.col_formats[i]))
            offset += width + 1
        self.text_ranges = text_ranges
        self.w = offset
        self.h = self.row_count + 2 + self.header_h
    def _draw_lines(self):
        w = self.window
        # draw border around
        w.box()
        # draw vertical lines separating columns an tees where the
        # lines join.
        for x, _, _ in self.text_ranges[1:]:
            w.vline(1, x - 1, curses.ACS_VLINE, self.h - 2)
            w.addch(0     , x - 1, curses.ACS_TTEE)
            w.addch(self.h - 1, x - 1, curses.ACS_BTEE)
        if not self.header:
            return
        # draw header separator and tees
        w.hline(self.header_h, 1, curses.ACS_HLINE, self.w - 2)
        w.addch(self.header_h, 0, curses.ACS_LTEE)
        w.addch(self.header_h, self.w - 1, curses.ACS_RTEE)
        # draw crosses
        for x, _, _ in self.text_ranges[1:]:
            w.addch(self.header_h, x - 1, curses.ACS_PLUS)
        # write actual header text, centered
        for (x, width), txt in zip([(r[0], r[1]) for r in self.text_ranges], self.header):
            format_str = '{:^%d.%d}'%(width, width)
            for y, hline in enumerate(txt.split('\n')):
                w.addstr(y + 1, x, format_str.format(hline), curses.A_BOLD)
    def move(self, y, x):
        try:
            self.panel.move(y, x)
        except:
            pass
    def move_delta(self, dy, dx):
        y, x = self.window.getbegyx()
        try:
            self.panel.move(y + dy, x + dx)
        except:
            pass
    def set_data(self, data):
        self._data = data
        def _draw_line(window, ranges, data, row_i):
            for (x, w, f), d in zip(ranges, data):
                txt = f.format(d)
                window.addstr(row_i, x, txt)
        row0 = 1 + self.header_h
        for i, row in enumerate(self._data[:self.row_count]):
            _draw_line(self.window, self.text_ranges, row, row0 + i)
    def hide_selection(self, hide = True):
        pass
    def handle_ch(self, key):
        pass

class CoilTable(Table):
    _addresses = [
        '0x0000', '0x0008', '0x0010', '0x0018', '0x0020',
        '0x0028', '0x0030', '0x0038', '0x0040', '0x0048',
        '0x0050', '0x0058', '0x0060', '0x0068', '0x0070',
        '0x0078', '0x0080', '0x0088', '0x0090', '0x0098',
        '0x00A0', '0x00A8', '0x00B0', '0x00B8', '0x00C0',
        '0x00C8', '0x00D0']
    _descriptions = [
        'AU  PORTB (RO)  ', '    PORTC (RO)  ', '    PORTD (RO)  ',
        'AI  PORTB (RO)  ', '    PORTC (RO)  ', '    PORTD (RO)  ',
        'AO  PORTB (RO)  ', '    PORTC (RO)  ', '    PORTD (RO)  ',
        'IOD PORTB (0=IN)', '    PORTC (0=IN)', '    PORTD (0=IN)',
        'IOM PORTB (1=AN)', '    PORTC (1=AN)', '    PORTD (1=AN)',
        'EIP PORTB       ', '    PORTC       ', '    PORTD       ',
        'EC  PORTB       ', '    PORTC       ', '    PORTD       ',
        'PI  PORTB (RO)  ', '    PORTC (RO)  ', '    PORTD (RO)  ',
        'PO  PORTB       ', '    PORTC       ', '    PORTD       ',
    ]
    _coil_count = len(_addresses) * 8
    def __init__(self):
        self._selected = [0, 0]
        self._selection_visible = False
        super().__init__({
            'row_count': int(self._coil_count / 8),
            'col_formats': ['{:6}', '{:8}', '{:16}'],
            'header': ['\nADDR', 'VALUE\n76543210', 'DESCRIPTION']
        }, data = [' ' for i in range(self._coil_count)])
        self._draw_selection()
    def _format_table_data(self, coils):
        bits = [' ' == b and ' ' or str(int(b)) for b in coils]
        lines = [
            ''.join(bits[i * 8:i * 8 + 8][::-1])
            for i in range(int(len(bits) / 8))]
        return list(zip(self._addresses, lines, self._descriptions))
    def handle_ch(self, key):
        action = None
        self._clear_selection()
        if key == curses.KEY_UP:
            self._selected[0] = (self._selected[0] - 1) % (self.row_count)
            self._selection_visible = True
        elif key == curses.KEY_RIGHT:
            self._selected[1] = (self._selected[1] + 1) % 8
            self._selection_visible = True
        elif key == curses.KEY_DOWN:
            self._selected[0] = (self._selected[0] + 1) % (self.row_count)
            self._selection_visible = True
        elif key == curses.KEY_LEFT:
            self._selected[1] = (self._selected[1] - 1) % 8
            self._selection_visible = True
        elif key == ord('1'):
            action = ['write_coils', self.selected_coil_addr(), [1]]
        elif key == ord('0'):
            action = ['write_coils', self.selected_coil_addr(), [0]]
        elif key == ord('t'):
            coil_offset = self.selected_coil_addr()
            action = ['write_coils', coil_offset, [int(not self._coils[coil_offset])]]
        self._draw_selection()
        return action
    def hide_selection(self, hide = True):
        self._selection_visible = not hide
    def selected_coil_addr(self):
        return self._selected[0] * 8 + 7 - self._selected[1]
    def _draw_selection(self):
        if not self._selection_visible:
            return
        self.window.chgat(self._selected[0] + 4, self._selected[1] + 8, 1, curses.A_REVERSE)
    def _clear_selection(self):
        self.window.chgat(self._selected[0] + 4, self._selected[1] + 8, 1, curses.A_NORMAL)
    def set_data(self, coils):
        self._coils = coils
        assert len(coils) == self._coil_count
        super().set_data(self._format_table_data(coils))
        self._draw_selection()

def get_text(h=1, w=8, y=10, x=10, txt=''):
    curs = curses.curs_set(1)
    win = curses.newwin(h + 2, w + 2, y, x)
    p = panel.new_panel(win)
    win.box()
    # w + 1 so that we can read w chars
    sw = win.derwin(h, w + 0, 1, 1)
    sw.addstr(0, 0, txt)
    win.refresh()
    box = Textbox(sw)
    box.edit()
    panel.update_panels()
    sw.refresh()
    curses.curs_set(curs)
    return box.gather()

def get_int(h=1, w=8, y=10, x=10, txt=''):
    txt = get_text(h, w, y, x, txt)
    if '0x' == txt[:2]:
        return int(txt, 16)
    else:
        return int(txt)

class RegisterTable(Table):
    _analog_in_count =  8
    _tc_control_count = 2
    _counter_count   = 16
    _register_count = _counter_count + _analog_in_count + _tc_control_count
    def __init__(self):
        def _pins():
            for port in ('B', 'C', 'D'):
                for pin in range(8):
                    yield '%s%s'%(port, pin)
        self._selection_visible = False
        self._selected     = 0
        self._addresses    = ['0x%04x'%i for i in range(self._register_count)]
        self._descriptions = ['ANALOGIN%02d'%i for i in range(self._analog_in_count)] + \
                             ['TC2ACONF', 'TC2BCONF'] + \
                             ['COUNTER%02d (%s)'%(i, pin) for i, pin in zip(range(self._counter_count), _pins())]
        super().__init__({
            'row_count': int(self._register_count),
            'col_formats': ['{:6}', '{:6}', '{:14}'],
            'header': ['ADDR\n', 'VALUE', 'DESCRIPTION']
        }, data = [0 for i in range(self._register_count)])
    def _clear_selection(self):
        self.window.chgat(self._selected + 4, 8, 6, curses.A_NORMAL)
    def _draw_selection(self):
        if not self._selection_visible:
            return
        self.window.chgat(self._selected + 4, 8, 6, curses.A_REVERSE)
    def _format_table_data(self, counters, analog_values):
        lines = ['0x%04x'%c for c in counters] + \
                ['0x%04x'%a for a in analog_values]
        return list(zip(self._addresses, lines, self._descriptions))
    def handle_ch(self, key):
        action = None
        self._clear_selection()
        if key == curses.KEY_UP:
            self._selected = (self._selected - 1) % (self.row_count)
            self._selection_visible = True
        elif key == curses.KEY_DOWN:
            self._selected = (self._selected + 1) % (self.row_count)
            self._selection_visible = True
        elif key == ord('\n'):
            wy, wx = self.window.getbegyx()
            initial_txt = '0x%04x'%self._values[self._selected]
            try:
                value = get_int(1, 7, wy + self._selected + 3, wx + 7, initial_txt)
                if value != self._values[self._selected]:
                    action = ['write_registers', self._selected, [value]]
            except ValueError:
                pass
        elif key == ord('+'):
            action = ['write_registers', self._selected, [self._values[self._selected] + 1]]
            pass
        elif key == ord('-'):
            action = ['write_registers', self._selected, [self._values[self._selected] - 1]]
            pass
        self._draw_selection()
        return action
    def hide_selection(self, hide = True):
        self._selection_visible = not hide
    def set_data(self, values):
        self._values = values
        super().set_data(self._format_table_data(
            values[:self._counter_count],
            values[self._counter_count:]))
        self._draw_selection()

def show_dialog(parent, title, msg):
    lines = msg.split('\n')
    h = len(lines)
    w = max([len(line) for line in lines])
    maxy, maxx = parent.getmaxyx()
    y = int((maxy - h) / 2)
    x = int((maxx - w) / 2)
    window = curses.newwin(h + 2, w + 2, y, x)
    textwindow = window.derwin(h, w, 1, 1)
    p = panel.new_panel(window)
    window.box()
    window.addstr(0, 1, ' %s '%title, curses.A_BOLD)
    for y, line in enumerate(lines):
        textwindow.addstr(y, 0, line)
    window.getch()
    p.hide()

def _center(container, coils, registers):
    container_y, container_x = container.getmaxyx()
    w = coils.w + registers.w + 2
    h = max([coils.h, registers.h])
    coils_x     = int((container_x - w) / 2)
    registers_x = coils_x + coils.w + 2
    y = int((container_y - h) / 2)
    coils    .move(y, coils_x)
    registers.move(y, registers_x)

def _configure_curses(stdscr):
    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(
        COLOR_WHITE_ON_RED_ID,
        curses.COLOR_WHITE,
        curses.COLOR_RED)
    global COLOR_WHITE_ON_RED
    COLOR_WHITE_ON_RED = curses.color_pair(COLOR_WHITE_ON_RED_ID)
    stdscr.clear()
    stdscr.border(0)
    stdscr.timeout(50)
    curses.curs_set(0)

class Circular(list):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.i = 0
    def current(self):
        return self[self.i]
    def next(self):
        self.i = (self.i + 1) % len(self)

def tui_main(stdscr):
    # This prevents arduino autoreset. It does not work on the first call
    # because it doesn't give time to the Arduino program to
    # initialize. After that it is not necessary until the configuration
    # of the port changes again but keeping it here seems like the
    # cleanest and simplest solution.
    os.system('stty -hup -F %s'%SERIAL_PORT)
    _configure_curses(stdscr)
    coil_table = CoilTable()
    register_table = RegisterTable()
    tables = Circular([coil_table, register_table])
    _center(stdscr, coil_table, register_table)
    client = ModbusSerialClient(
        method = "rtu",
        port=SERIAL_PORT,
        stopbits = 1,
        bytesize = 8,
        parity = 'E',
        baudrate= 19200,
        dsrdtr=False,
        timeout=0.01)
    conn  = client.connect()
    read_op       = 0
    coils         = []
    counters      = []
    analog_values = []
    while True:
        if read_op == READ_COILS:
            coils = client.read_coils(0x0000, 0xd8, unit=UNIT_ID).bits
        elif read_op == READ_COUNTERS:
            counters = client.read_holding_registers(0x000a, 16, unit=UNIT_ID).registers
        elif read_op == READ_ANALOG:
            analog_values = client.read_holding_registers(0x0000, 10, unit=UNIT_ID).registers
        read_op = (read_op + 1) % READ_OP_COUNT
        coil_table.set_data(coils)
        registers     = analog_values + counters
        register_table.set_data(registers)
        stdscr.touchwin()
        panel.update_panels()
        curses.doupdate()
        ch = stdscr.getch()
        action = None
        if ch == -1:
            continue
        if ch == ord('q'):
            return
        elif ch == ord('\t'):
            tables.current().hide_selection(True)
            tables.next()
            tables.current().hide_selection(False)
        elif ch == ord('h'):
            show_dialog(stdscr, 'Help', HELP_MSG)
        elif ch == curses.KEY_RESIZE:
            stdscr.clear()
            stdscr.border(0)
            _center(stdscr, coil_table, register_table)
        else:
            action = tables.current().handle_ch(ch)
        tl_attr = curses.A_NORMAL
        if action:
            cmd = action[0]
            if cmd == 'write_coils':
                cmd, addr, bits = action
                result = client.write_coils(addr, bits, unit=UNIT_ID)
                tl_msg = 'write_coils(0x%04x, %s, unit=0x%x) -> %s'%(addr, bits, UNIT_ID, result)
                if result.function_code & 0x80:
                    tl_attr = COLOR_WHITE_ON_RED|curses.A_BOLD
                read_op = READ_COILS
            if cmd == 'write_registers':
                cmd, addr, words = action
                result = client.write_registers(addr, words, unit=UNIT_ID)
                tl_msg = 'write_registers(0x%04x, %s, unit=0x%x) -> %s'%(addr, words, UNIT_ID, result)
                if result.function_code & 0x80:
                    tl_attr = COLOR_WHITE_ON_RED|curses.A_BOLD
                read_op = READ_COUNTERS
            else:
                tl_msg = str(action)
        else:
            tl_msg = curses.keyname(ch).decode('utf-8')
        stdscr.addstr(1, 1, tl_msg, tl_attr)
        stdscr.clrtoeol()

def main():
    curses.wrapper(tui_main)

if __name__ == '__main__':
    main()
