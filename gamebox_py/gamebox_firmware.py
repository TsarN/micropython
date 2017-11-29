import gamebox
import pyb
import random

from gamebox_data import *

gamebox.initialize()


selection = 0

# copy sprites to canvas
gamebox.screen.blit(sprite_games, 0, 0, -1, 255)
gamebox.screen.blit(sprite_config, 32, 0, -1, 255)

def update_screen():
    global selection
    gamebox.screen.rect(0, 0, 32, 32, 255 if selection == 0 else 0)
    gamebox.screen.rect(32, 0, 32, 32, 255 if selection == 1 else 0)

update_screen()

@gamebox.events.update(50) # 50 FPS
def on_update(delta):
    pass

@gamebox.events.pressed(gamebox.BUTTON_LEFT)
def on_button_left_pressed():
    global selection
    selection = 0
    update_screen()

@gamebox.events.pressed(gamebox.BUTTON_RIGHT)
def on_button_left_pressed():
    global selection
    selection = 1
    update_screen()
