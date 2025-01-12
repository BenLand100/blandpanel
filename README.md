# BLandPanel

This aims to be a reference design for a DYI telescope automated flat panel.
Any RP2040 dev board, or similar device, capable of running MicroPython,
a PWM modulated servo, and some commodity electronics components make up the
electronics. Some Python and C++ for writing the firmware and software driver
(respectively - which is quite ironic) to allow INDI capable clients to control
the device and a bit of CAD and 3D printing to hold it all together.

The flat panel ~~and CAD itself are~~ is TBD.

## RP2040 Firmware

Firmware for the RP2040 microcontroller is available in `rpico-firmare`.

Any RP2040 dev board with some reasonable pinout will work. This repo is tested
and configured for the vanilla Raspberry Pi Pico using some arbitrary GPIO and 
one optional-ish ADC pin. A small current driver for the LED panel will be 
required. 

## INDI driver

A driver based heavily off the reference INDI driver that can control the 
firmware is available in `indi-driver`.

## ASCOM driver

The firmware also impelemnts (untested) the flat panel serial protocol found in 
[DarkSkyGeek's ascom-flat-panel repo](https://github.com/jlecomte/ascom-flat-panel/)
so the ASCOM driver found there would very likely control this firmware fine.

## CAD Design

There is a FreeCAD project in the `design` directory along with exported models
of the current build. This is very much a WIP and the current model is a test 
design primarily to figure out what needs to be changed for the next revision.
