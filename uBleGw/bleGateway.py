#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.1-or-later

import dbus
try:
  from gi.repository import GObject
except ImportError:
  import gobject as GObject
import sys

from dbus.mainloop.glib import DBusGMainLoop

#libs for uart
import time
import serial

btn1Found = 0
b1Val = 0
b2Val = 0
pVal = 0
bus = None
mainloop = None
ser = serial.Serial()

BLUEZ_SERVICE_NAME = 'org.bluez'
DBUS_OM_IFACE =      'org.freedesktop.DBus.ObjectManager'
DBUS_PROP_IFACE =    'org.freedesktop.DBus.Properties'

GATT_SERVICE_IFACE = 'org.bluez.GattService1'
GATT_CHRC_IFACE =    'org.bluez.GattCharacteristic1'

#HR_SVC_UUID =        '0000180d-0000-1000-8000-00805f9b34fb'

WHEELBTNS_SVC_UUID =  '59c88760-5364-11e7-b114-b2f933d5fe66'

WHEELPEDALS_SVC_UUID ='0000fff0-0000-1000-8000-00805f9b34fb' #/org/bluez/hci0/dev_54_6C_0E_A0_39_40/service001c


#HR_MSRMT_UUID =      '00002a37-0000-1000-8000-00805f9b34fb'
#BODY_SNSR_LOC_UUID = '00002a38-0000-1000-8000-00805f9b34fb'
#HR_CTRL_PT_UUID =    '00002a39-0000-1000-8000-00805f9b34fb'


WHEELBTN1_CHR_UUID =  '59c889e0-5364-11e7-b114-b2f933d5fe66'
WHEELBTN2_CHR_UUID =  '59c889e0-5364-11e7-b114-b2f933d5fe66'

WHEELPEDALS_CHR_UUID ='0000fff4-0000-1000-8000-00805f9b34fb' #/org/bluez/hci0/dev_54_6C_0E_A0_39_40/service001c/char0026

# The objects that we interact with.
hr_service = None
hr_msrmt_chrc = None
body_snsr_loc_chrc = None
hr_ctrl_pt_chrc = None


def generic_error_cb(error):
    print('D-Bus call failed: ' + str(error))
    mainloop.quit()


def body_sensor_val_to_str(val):
    if val == 0:
        return 'Other'
    if val == 1:
        return 'Chest'
    if val == 2:
        return 'Wrist'
    if val == 3:
        return 'Finger'
    if val == 4:
        return 'Hand'
    if val == 5:
        return 'Ear Lobe'
    if val == 6:
        return 'Foot'

    return 'Reserved value'


def sensor_contact_val_to_str(val):
    if val == 0 or val == 1:
        return 'not supported'
    if val == 2:
        return 'no contact detected'
    if val == 3:
        return 'contact detected'

    return 'invalid value'


def body_sensor_val_cb(value):
    if len(value) != 1:
        print('Invalid body sensor location value: ' + repr(value))
        return

    print('Body sensor location value: ' + body_sensor_val_to_str(value[0]))

def wbtn1_startnotify_cb():
    print('wheel button 1 Measurement notifications enabled')


def wheel_btn1_changed_cb(iface, changed_props, invalidated_props):
    global b1Val
    global ser
    if iface != GATT_CHRC_IFACE:
        return

    if not len(changed_props):
        return

    value = changed_props.get('Value', None)
    if not value:
        return

    print('New Measurement')
    if value[0] == dbus.Byte(1):
        print('wheel button 1 not pressed')
        b1Val = 0
    else:
        print('wheel button 1 pressed')
        b1Val = 1
    serCmd = 'b1=' + str(b1Val) + '\r\n'
    ser.write(serCmd.encode())

def wbtn2_startnotify_cb():
    print('wheel button 2 Measurement notifications enabled')


def wheel_btn2_changed_cb(iface, changed_props, invalidated_props):
    global b2Val
    global ser
    if iface != GATT_CHRC_IFACE:
        return

    if not len(changed_props):
        return

    value = changed_props.get('Value', None)
    if not value:
        return

    print('New Measurement')
    if value[0] == dbus.Byte(1):
        print('wheel button 2 not pressed')
        b2Val = 0
    else:
        print('wheel button 2 pressed')
        b2Val = 1
    serCmd = 'b2=' + str(b2Val) + '\r\n'
    ser.write(serCmd.encode())

def wpedals_startnotify_cb():
    print('wheel pedals Measurement notifications enabled')


def wheel_pedals_changed_cb(iface, changed_props, invalidated_props):
    global pVal
    global ser
    if iface != GATT_CHRC_IFACE:
        return

    if not len(changed_props):
        return

    value = changed_props.get('Value', None)
    if not value:
        return

    print('New Measurement')
    #print(value)
    pVal = ''.join(bytes([v]).hex() for v in value)
    print(pVal)
    #print("type is", type(pVal))
    serCmd = 'p=' + str(pVal) + '\r\n'
    ser.write(serCmd.encode())

#def hr_msrmt_start_notify_cb():
#    print('HR Measurement notifications enabled')


#def hr_msrmt_changed_cb(iface, changed_props, invalidated_props):
#    if iface != GATT_CHRC_IFACE:
#        return
#
#    if not len(changed_props):
#        return
#
#    value = changed_props.get('Value', None)
#    if not value:
#        return
#
#    print('New HR Measurement')
#
#    flags = value[0]
#    value_format = flags & 0x01
#    sc_status = (flags >> 1) & 0x03
#    ee_status = flags & 0x08
#
#    if value_format == 0x00:
#        hr_msrmt = value[1]
#        next_ind = 2
#    else:
#        hr_msrmt = value[1] | (value[2] << 8)
#        next_ind = 3
#
#    print('\tHR: ' + str(int(hr_msrmt)))
#    print('\tSensor Contact status: ' +
#          sensor_contact_val_to_str(sc_status))
#
#    if ee_status:
#        print('\tEnergy Expended: ' + str(int(value[next_ind])))


def start_client():
    # Read the Body Sensor Location value and print it asynchronously.
    #wheel_btn1_chrc[0].ReadValue({}, reply_handler=wbtn1_val_cb,
    #                                error_handler=generic_error_cb,
    #                                dbus_interface=GATT_CHRC_IFACE)
    # Listen to PropertiesChanged signals from the Heart Measurement
    # Characteristic.
    wheel_btn1_prop_iface = dbus.Interface(wheel_btn1_chrc[0], DBUS_PROP_IFACE)
    wheel_btn1_prop_iface.connect_to_signal("PropertiesChanged",
                                          wheel_btn1_changed_cb)

    wheel_btn1_chrc[0].StartNotify(reply_handler=wbtn1_startnotify_cb,
                                 error_handler=generic_error_cb,
                                 dbus_interface=GATT_CHRC_IFACE)


    wheel_btn2_prop_iface = dbus.Interface(wheel_btn2_chrc[0], DBUS_PROP_IFACE)
    wheel_btn2_prop_iface.connect_to_signal("PropertiesChanged",
                                          wheel_btn2_changed_cb)

    wheel_btn2_chrc[0].StartNotify(reply_handler=wbtn2_startnotify_cb,
                                 error_handler=generic_error_cb,
                                 dbus_interface=GATT_CHRC_IFACE)

    wheel_pedals_prop_iface = dbus.Interface(wheel_pedals_chrc[0], DBUS_PROP_IFACE)
    wheel_pedals_prop_iface.connect_to_signal("PropertiesChanged",
                                          wheel_pedals_changed_cb)

    wheel_pedals_chrc[0].StartNotify(reply_handler=wpedals_startnotify_cb,
                                 error_handler=generic_error_cb,
                                 dbus_interface=GATT_CHRC_IFACE)

    # Read the Body Sensor Location value and print it asynchronously.
    #body_snsr_loc_chrc[0].ReadValue({}, reply_handler=body_sensor_val_cb,
    #                                error_handler=generic_error_cb,
    #                                dbus_interface=GATT_CHRC_IFACE)

    # Listen to PropertiesChanged signals from the Heart Measurement
    # Characteristic.
    #hr_msrmt_prop_iface = dbus.Interface(hr_msrmt_chrc[0], DBUS_PROP_IFACE)
    #hr_msrmt_prop_iface.connect_to_signal("PropertiesChanged",
    #                                      hr_msrmt_changed_cb)

    # Subscribe to Heart Rate Measurement notifications.
    #hr_msrmt_chrc[0].StartNotify(reply_handler=hr_msrmt_start_notify_cb,
    #                             error_handler=generic_error_cb,
    #                             dbus_interface=GATT_CHRC_IFACE)


def process_chrc(chrc_path):
    global btn1Found
    chrc = bus.get_object(BLUEZ_SERVICE_NAME, chrc_path)
    chrc_props = chrc.GetAll(GATT_CHRC_IFACE,
                             dbus_interface=DBUS_PROP_IFACE)

    uuid = chrc_props['UUID']

    print('\n\rcharacteristic uuid:' + uuid)


    if uuid == WHEELBTN1_CHR_UUID and btn1Found == 0:
        print ('\n\rbtn1 characterisic found')
        btn1Found = 1
        global wheel_btn1_chrc
        wheel_btn1_chrc = (chrc, chrc_props)
    elif uuid == WHEELBTN2_CHR_UUID:
        print ('\n\rbtn2 characterisic found')
        global wheel_btn2_chrc
        wheel_btn2_chrc = (chrc, chrc_props)
    elif uuid == WHEELPEDALS_CHR_UUID:
        print ('\n\rpedals characterisic found')
        global wheel_pedals_chrc
        wheel_pedals_chrc = (chrc, chrc_props)
    #if uuid == HR_MSRMT_UUID:
    #    global hr_msrmt_chrc
    #    hr_msrmt_chrc = (chrc, chrc_props)
    #elif uuid == BODY_SNSR_LOC_UUID:
    #    global body_snsr_loc_chrc
    #    body_snsr_loc_chrc = (chrc, chrc_props)
    #elif uuid == HR_CTRL_PT_UUID:
    #    global hr_ctrl_pt_chrc
    #    hr_ctrl_pt_chrc = (chrc, chrc_props)
    else:
        print('Unrecognized characteristic: ' + uuid)

    return True


def process_hr_service(service_path, chrc_paths):
    service = bus.get_object(BLUEZ_SERVICE_NAME, service_path)
    service_props = service.GetAll(GATT_SERVICE_IFACE,
                                   dbus_interface=DBUS_PROP_IFACE)

    uuid = service_props['UUID']

    print('\n\rservice uuid:' + uuid)

    if (uuid != WHEELBTNS_SVC_UUID) and (uuid != WHEELPEDALS_SVC_UUID):
        return False

    print('uWheel Service found: ' + service_path)

    # Process the characteristics.
    for chrc_path in chrc_paths:
        print ('\n\rchrc:'+chrc_path)
        process_chrc(chrc_path)

    global hr_service
    hr_service = (service, service_props, service_path)

    return True


def interfaces_removed_cb(object_path, interfaces):
    if not hr_service:
        return

    if object_path == hr_service[2]:
        print('Service was removed')
        mainloop.quit()


def main():
    #connect uart
    global ser
    ser.port='/dev/ttyS0'
    ser.baudrate=9600
    ser.parity=serial.PARITY_NONE
    ser.stopbits=serial.STOPBITS_ONE
    ser.bytesize=serial.EIGHTBITS

    try:
        ser.open()
    except:
        print("error opening serial port")
        exit()


    # Set up the main loop.
    DBusGMainLoop(set_as_default=True)
    global bus
    bus = dbus.SystemBus()
    global mainloop
    mainloop = GObject.MainLoop()

    om = dbus.Interface(bus.get_object(BLUEZ_SERVICE_NAME, '/'), DBUS_OM_IFACE)
    om.connect_to_signal('InterfacesRemoved', interfaces_removed_cb)

    print('Getting objects...')
    objects = om.GetManagedObjects()
    chrcs = []

    # List characteristics found
    for path, interfaces in objects.items():
        if GATT_CHRC_IFACE not in interfaces.keys():
            continue
        chrcs.append(path)

    srvCount = 0

    # List sevices found
    for path, interfaces in objects.items():
        if GATT_SERVICE_IFACE not in interfaces.keys():
            continue

        print('\n\rpath found:' + path)
        chrc_paths = [d for d in chrcs if d.startswith(path + "/")]

        if process_hr_service(path, chrc_paths):
            srvCount = srvCount + 1
        if srvCount >= 2: #both wheel serviced found (pedals and wheelButtons)
            break

    if not hr_service:
        print('No known service found; are the ble peripherals connected?')
        sys.exit(1)

    start_client()

    mainloop.run()


if __name__ == '__main__':
    main()
