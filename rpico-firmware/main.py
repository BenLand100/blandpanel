import machine
import time
import sys
from machine import SoftI2C, SoftSPI, PWM, ADC, Pin

# Power LED
led = Pin(25, Pin.OUT)
led.value(1)

# PWM for flat panel
panel = PWM(Pin(13))
panel.freq(10000)
panel.duty_ns(0)

# 5V from 12V regulator (for servo & pico)
en_5v = Pin(12, Pin.OUT)
en_5v.value(0)

# PWM servo 500us - 2500us pulse range (make configurable?)
servo = PWM(Pin(22))
servo.duty_ns(0) #Servo will ignore
servo.freq(100)

# Turn on Servo and 5V bus
en_5v.value(1)

# Analog value proportional to servo angle (optional)
angle_adc = ADC(2)

# TODO save in flash?
PULSE_ZERO = -0.026
PULSE_270 = 0.951
ADC_ZERO = 60850.8
ADC_270 = 6985.0
GUID = 'some-kind-of-unique-identifier'
DEFAULT_BRIGHTNESS = 0.75
ANGLE_TOLERANCE = 10.0
OPENED_ANGLE = 270.0
CLOSED_ANGLE = 0.0

def set_angle(angle, calibrating=False, wait=2):
    if not calibrating:
        assert angle >= -20 and angle <= 290, 'Angle out of bounds'
        angle = (angle / 270)*(PULSE_270-PULSE_ZERO)+PULSE_ZERO
    servo.duty_ns(int(500e3+angle*2e6))
    time.sleep(wait)
    
def get_angle(average=10, calibrating=False):
    avg = sum([angle_adc.read_u16() for i in range(average)])/average
    if calibrating:
        return avg
    else:
        return (avg-ADC_ZERO)*270/(ADC_270-ADC_ZERO)
    
def get_state():
    angle = get_angle()
    if abs(angle-CLOSED_ANGLE) < ANGLE_TOLERANCE:
        return 'closed'
    elif abs(angle-OPENED_ANGLE) < ANGLE_TOLERANCE:
        return 'opened'
    else:
        return 'intermediate'
    
def set_state(state):
    now = get_state()
    if state == 'opened':        
        if now != 'opened':
            set_angle(OPENED_ANGLE)
    elif state == 'closed':
        if now != 'closed':
            set_angle(CLOSED_ANGLE)
    else:
        raise Exception(f'Invalid state {state}')

ENABLED = False
BRIGHTNESS = DEFAULT_BRIGHTNESS

def set_enabled(enabled=True):
    global ENABLED
    if enabled:
        panel.duty_u16(int(BRIGHTNESS*(2**16-1)))
        ENABLED = True
    else:
        panel.duty_u16(0)
        ENABLED = False
    
def get_enabled(enabled=True):
    return ENABLED

def set_brightness(brightness):
    global BRIGHTNESS
    assert brightness >= 0 and brightness <= 1.0, 'Brightness out of bounds'
    BRIGHTNESS = brightness
    if ENABLED:
        panel.duty_u16(int(BRIGHTNESS*(2**16-1)))

def get_brightness():
    return BRIGHTNESS

ESTOP = False
def estop():
    global ESTOP
    en_5v.value(0)
    ESTOP = True
    
def allclear():
    global ESTOP
    if ESTOP:
        ESTOP = False
        en_5v.value(1)
        
while True:
    cmd = sys.stdin.readline().upper().strip()
    try:
        if len(cmd) < 2:
            continue
        elif cmd.startswith('ESTOP'):
            estop()
            print('ESTOP: ALLCLEAR REQUIRED TO RESUME MOTION')
        elif cmd.startswith('ALLCLEAR'):
            allclear()
            print('OK')
        elif cmd.startswith('COMMAND:'): # Using this ASCOM driver https://github.com/jlecomte/ascom-flat-panel/
            if cmd == 'COMMAND:PING':
                print(f'RESULT:PING:OK:{GUID}')
            elif cmd == 'COMMAND:INFO':
                print('RESULT:INFO:BLandPanel v0')
            elif cmd == 'COMMAND:CALIBRATOR:GETBRIGHTNESS':
                print('RESULT:CALIBRATOR:BRIGHTNESS:{BRIGHTNESS}')
            elif cmd.startswith('COMMAND:CALIBRATOR:BRIGHTNESS:'):
                parts = cmd.split(':')
                if len(parts) > 3:
                    value = float(parts[3])
                    set_brightness(value)
            elif cmd.startswith('COMMAND:CALIBRATOR:ON'):
                set_state('closed')
                parts = cmd.split(':')
                if len(parts) > 3:
                    value = float(parts[3])
                    set_brightness(value)
                else:
                    set_brightness(DEFAULT_BRIGHTNESS)
            elif cmd == 'COMMAND:CALIBRATOR:OFF':
                set_enabled(False)
                set_state('opened')
            else:
                print('ERROR:INVALID_COMMAND')
        elif cmd == 'REPL':
            print('GOODBYE')
            break
        elif cmd == 'RESET':
            print('GOODBYE')
            machine.reset() #exits
        elif cmd == 'STATUS':
            print(f'OK STATE: {get_state().upper()} BRIGHTNESS: {get_brightness():0.2f} ENABLED: {str(get_enabled()).upper()}')
        elif cmd.startswith('ON'):
            parts = cmd.split(' ')
            if len(parts) > 1:
                value = float(parts[1])
                set_brightness(value)
            set_enabled(True)
            print('OK')
        elif cmd == 'OFF':
            set_enabled(False)
            print('OK')
        elif cmd.startswith('BRIGHT'):
            parts = cmd.split(' ')
            if len(parts) > 1:
                value = float(parts[1])
                set_brightness(value)
                print('OK')
            else:
                print(f'{get_brightness():0.2f}')
        elif cmd == 'OPEN':
            set_state('opened')
            if get_state() == 'opened':
                print('OK')
            else:
                print('FAIL')
        elif cmd == 'CLOSE':
            set_state('closed')
            if get_state() == 'closed':
                print('OK')
            else:
                print('FAIL')
        elif cmd.startswith('STATE'):
            parts = cmd.split(' ')
            if len(parts) > 1:
                state = parts[1].lower()
                set_state(state)
                if get_state() == state:
                    print('OK')
                else:
                    print('FAIL')
            else:
                print(get_state().upper())
        elif cmd.startswith('ANGLE'):
            parts = cmd.split(' ')
            if len(parts) > 1:
                value = float(parts[1])
                set_angle(value)
                angle = get_angle()
                if abs(angle-value) < ANGLE_TOLERANCE:
                    print('OK')
                else:
                    print('FAIL')
            else:
                angle = get_angle()
                print(f'{angle}')
        else:
            print(f'Useful Commands: STATUS, STATE [open|closed], ON [0-1], BRIGHT [0-1], OFF, OPEN, CLOSE, ANGLE [0-270]')
    except KeyboardInterrupt as e:
        raise
    except Exception as e:
        print(f'FAULT: {e}')