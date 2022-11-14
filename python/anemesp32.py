

import serial
import time
import argparse
import threading

from xmlrpc.server import SimpleXMLRPCServer

def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

class EnvDaqThread(threading.Thread):
    def __init__(self, dev):
        threading.Thread.__init__(self)
        self.dev = dev
        return
    def run(self):
        self.dev.scan()
    
    
class AnemESP32(object):

    def __init__(self, comport='/dev/ttyUSB0', baud=9600, timeout=5):
        self.availablechans = ['P', 'PT', 'H', 'HT', 'T1',
                               'T2', 'T3', 'T4', 'T5']
        self.daqchans = ['P', 'H', 'T1', 'T2', 'T3', 'T4']
        self.acquiring = False
        self.tinit = 0
        self.ttotal = 1
        self.nsamples = 0
        self.stopaq = False
        self.frames = []
        self.rate = 0.0
        self.thrd = None
        
        self.dev = serial.Serial(comport, baud, timeout=timeout);
        return
    def daqtime(self, val=None):
        if self.acquiring:
            raise RuntimeError("Can't do this while acquiring data!")
        if val is None:
            return self.ttotal
        else:
            val = float(val)
            if val < 0 or val > 300:
                raise RuntimeError("daqtime should be a positive time interval in seconds, less than 300!")
                
            self.ttotal = val
            return None
    def addinput(self, chans):
        new_chans = []
        for ch in chans:
            if ch in self.availablechans:
                new_chans.append(ch)
            else:
                raise ValueError("Channel {} is not available!".format(ch))
        if new_chans:  # It is not empty!
            self.chans = new_chans
            
    
    def acquirechan(self, chan='P'):
        if chan=='P':
            return self.press()
        elif chan=='PT':
            return self.presstemp()
        elif chan=='H':
            return self.humidity()
        elif chan=='HT':
            return self.humtemp()
        elif chan=='T1':
            return self.temp(1)
        elif chan=='T2':
            return self.temp(2)
        elif chan=='T3':
            return self.temp(3)
        elif chan=='T4':
            return self.temp(4)
        elif chan=='T5':
            return self.temp(5)
        else:
            error('Channel {} does not exist!'.format(chan))
            
    def acquiresample(self):
        values = [time.monotonic()]
        for chan in self.daqchans:
            values.append(self.acquirechan(chan))
        return values
    def scan(self, ttotal=None):
        if self.acquiring:
            raise RuntimeError("Can't do this while acquiring data!")
        if ttotal is not None:
            ttotal = float(ttotal)
            if ttotal < 0 or ttotal > 300:
                ttotal = self.ttotal
        else:
            ttotal = self.ttotal
        
        self.frames = []
        self.stopaq = False
        self.acquiring = True
        self.nsamples = 0
        
        t1 = time.monotonic()
        self.tinit = t1
        tend = t1 + ttotal
        n = 0
        while True:
            self.frames.append(self.acquiresample())
            n = n + 1
            self.nsamples = n
            t2 = time.monotonic()
            if t2 > tend or self.stopaq:
                break
        # Measure the sampling frequency
        rate = n / (t2-t1)
        self.acquiring = False
        self.rate = rate

    def acquire(self):

        self.scan()
        return self.frames, self.rate
    
    def start(self):
        if self.acquiring:
            raise RuntimeError("Illegal operation: System is already acquiring!")
        self.thrd = EnvDaqThread(self)
        self.thrd.start()
        self.acquiring = True
    def read(self):
        if self.thrd is not None:
            self.thrd.join()
            self.thrd = None
        
        return self.frames, self.rate
    def channels(self):
        return self.daqchans
    def availablechannels(self):
        return self.availablechans
    
    
    def stop(self):
        if self.acquiring:
            self.stopaq = True
        return
    def isacquiring(self):
        return self.acquiring
    def samplesread(self):
        return self.nsamples
    
    def open(self):
        if self.dev.is_open:
            self.dev.close()
        self.dev.open()
        self.dev.flushInput()
        self.dev.flushOutput()
        time.sleep(0.1)
    def close(self):
        self.flush()
        self.dev.close()
        
    def flush(self):
        self.dev.flushInput()
        self.dev.flushOutput()

    def command(self, cmd, ntries=3):
        for i in range(ntries):
            self.dev.write(cmd)
            self.dev.flushOutput()
            s = self.dev.readline()
            try:
                val = float(s)
                return val
            except ValueError:
                time.sleep(0.2)
        return -9999.9

    def press(self):
        return self.command(b'*PP')
    
    def presstemp(self):
        return self.command(b'*PT')
    
    def humidity(self):
        return self.command(b'*HH')
    
    def humtemp(self):
        return self.command(b'*HT')
    
    def temp(self, i=1):
        if (i < 1 or i > 5):
            error("Only temperature sensors 1-5 are available")
            return
        i = int(i)
        cmd = "*T{}".format(i).encode('ascii')
        return self.command(cmd)
    def ads(self, i=0):
        i = int(i)
        if (i < 0 or i > 3):
            error("Only ADC channels 0-3 are available")
            return
        cmd = "*A{}".format(i).encode('ascii')
        return self.command(cmd)
    
    def clear(self):
        cmd = b'%'
        self.dev.write(cmd)
        self.dev.flushOutput()
        time.sleep(0.1)
        return
    
    def status(self):
        cmd = b'*S1'
        self.dev.write(cmd)
        self.dev.flushOutput()
        s = self.dev.readline()
        return s
    


def start_server(ip='localhost', port=9539, comport='/dev/ttyUSB0', baud=9600):
    dev = EnvConds(comport, baud)
    print("Starting XML-RPC server")
    print("IP: {}, port: {}".format(ip, port))
    server = SimpleXMLRPCServer((ip, port), allow_none=True)
    server.register_instance(dev)
    server.serve_forever()


if __name__ == "__main__":
    print("Creating interface ...")
    parser = argparse.ArgumentParser(description="EnvConds XML-RPC server")
    parser.add_argument("-i", "--ip", help="IP address of the XML-RPC server", default="localhost")
    parser.add_argument("-p", "--port", help="XML-RPC server port", default=9539, type=int)
    parser.add_argument("-s", "--comport", help="Serial port to be used", default="/dev/ttyUSB0")

    args = parser.parse_args()
    start_server(args.ip, args.port, args.comport)
    
    
