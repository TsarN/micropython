import _ledmatrix
import uctypes
import framebuf
import pyb
import stm

initialized = False
buf = None
screen = None
_tim = None
_key_hooks = [] # array of tuples (func, key, onpress?)
buttons_pressed = 0

WIDTH = const(64)
HEIGHT = const(64)

BUTTON_A = const(0x01)
BUTTON_B = const(0x02)
BUTTON_SELECT = const(0x04)
BUTTON_START = const(0x08)
BUTTON_UP = const(0x10)
BUTTON_DOWN = const(0x20)
BUTTON_LEFT = const(0x40)
BUTTON_RIGHT = const(0x80)


def initialize():
    global initialized, buf, screen, _tim
    if initialized:
        return False
    buf = uctypes.bytearray_at(_ledmatrix.init(), WIDTH * HEIGHT)
    screen = framebuf.FrameBuffer(buf, WIDTH, HEIGHT, framebuf.RGB332)
    _tim = pyb.Timer(4)
    pyb.Pin.board.PC0.init(pyb.Pin.OUT_PP)
    pyb.Pin.board.PC1.init(pyb.Pin.IN, pyb.Pin.PULL_UP)
    pyb.Pin.board.PC2.init(pyb.Pin.OUT_PP)
    initialized = True
    return True

def update_input():
    global buttons_pressed
    old_buttons_pressed = buttons_pressed
    buttons_pressed = 0
    pyb.Pin.board.PC2.value(True)
    pyb.Pin.board.PC2.value(False)
    for btn in range(8):
        if not pyb.Pin.board.PC1.value():
            buttons_pressed |= (1 << btn)
        pyb.Pin.board.PC0.value(True)
        pyb.Pin.board.PC0.value(False)
    if old_buttons_pressed == buttons_pressed:
        return
    did_press = ~old_buttons_pressed & buttons_pressed
    did_release = old_buttons_pressed & ~buttons_pressed
    for func, key, pressed in _key_hooks:
        if key & (did_press if pressed else did_release):
            func()

def is_pressed(btn):
    global buttons_pressed
    return bool(btn & buttons_pressed)


class events:
    def clear_hooks():
        global _tim
        _tim.callback(None)
    def update(freq=50):
        global _tim
        def decorator(func):
            def callback(timer):
                update_input()
                func(freq)
            _tim.init(freq=freq)
            _tim.callback(callback)
            return func
        return decorator

    def pressed(key=0xffff):
        def decorator(func):
            global _key_hooks
            _key_hooks.append((func, key, True))
            return func
        return decorator

    def released(key=0xffff):
        def decorator(func):
            global _key_hooks
            _key_hooks.append((func, key, False))
            return func
        return decorator