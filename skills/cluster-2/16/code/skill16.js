//Nnenna Eze (Team 15)
//Date: 10/08/2020

const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')
const port = new SerialPort('/dev/cu.SLAB_USBtoUART', { baudRate: 115200}) //Serial port event

const parser = new Readline()
port.pipe(parser)

parser.on('data',line => console.log(`>${line}`))
port.write('ROBOT POWER ON\n')
