import time
import binascii
from bluepy import btle

print "Sabar jeh..."
dev = btle.Peripheral("80:7D:3A:CB:0F:7E")

print "Services..."
for svc in dev.services:
	print str(svc)

dht11 = btle.UUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b")

dht11Data = dev.getServiceByUUID(dht11)
for ch in dht11Data.getCharacteristics():
	print str(ch)

uuidValue  = btle.UUID("beb5483e-36e1-4688-b7f5-ea07361b26a8")
dht11Value = dht11Data.getCharacteristics(uuidValue)[0]
# Read the sensor
val = dht11Value.read()
print "DHT11 sensor raw value", binascii.b2a_hex(val)

