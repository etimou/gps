#!/usr/bin/python
import serial
import time
import os


ser = serial.Serial('/dev/ttyACM0', 19200, timeout=2)
f = None
filename= "00.nmea"

def convertGPX(textfilename):
  command = "gpsbabel -i nmea -f " + textfilename + " -x discard,hdop=10 -o gpx -F " + textfilename[:-3]+"gpx"
  print command
  os.system(command)


while (True):
  line=ser.readline()
  if len(line)==0:
    break
  else:
    pass



ser.write('\r\n')
time.sleep(1)

ser.write('p')
while (True):
  line=ser.readline()
  if len(line)==0:
    if f:
      f.close()
      convertGPX(filename)
    break
  else:
    line=line[:-2]
    if ('.TXT' in line):
      if f:
        f.close()
        convertGPX(filename)
      filename=line
      f= open(filename,'w')
    if ('$' in line):
      f.write(line+'\n')
      
