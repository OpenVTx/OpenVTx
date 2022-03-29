import serial
from xmodem import XMODEM
import time
import sys
import logging
import os
import serials_find
import BFinitPassthrough
import SerialHelper
import re

SCRIPT_DEBUG = 1
PROTOCOL_DEFAULT = "SA"
UPLOAD_BAUD = 57600 # faster doesnt work :(
UPLOAD_STOP_BITS = 1

PROTOCOL_DATA = {
    "SA": {
        "baud": 4800,
        "stopbits": 2,
    },
    "TRAMP": {
        "baud": 9600,
        "stopbits": 1,
    },
    "MSP": {
        "baud": 9600,
        "stopbits": 1,
    },
    "DEFAULT": {
        "baud": 57600,
        "stopbits": 1,
    },
}

def dbg_print(line=''):
    sys.stdout.write(line)
    sys.stdout.flush()
    return


def uart_upload(port, filename, protocol=None, half_duplex=True):
    dbg_print("=================== FIRMWARE UPLOAD ===================\n")
    dbg_print("  Bin file '%s'\n" % filename)
    dbg_print("  Port: %s, protocol: %s\n" % (port, protocol))

    PROTO = PROTOCOL_DATA.get(protocol, None)
    if PROTO is None:
        PROTO = PROTOCOL_DATA.get("DEFAULT")

    logging.basicConfig(level=logging.ERROR)

    if not os.path.exists(filename):
        msg = "[FAILED] file '%s' does not exist\n" % filename
        dbg_print(msg)
        raise Exception(msg)

    vtx_type = None

    # Try to init Betaflight passthrough
    try:
        vtx_type = BFinitPassthrough.bf_passthrough_init(port, 0, half_duplex)
        dbg_print("\nDetected VTX protocol: %s\n" % vtx_type)
    except BFinitPassthrough.PassthroughEnabled as info:
        dbg_print("  Warning: '%s'\n" % info)
    except BFinitPassthrough.PassthroughFailed as failed:
        raise

    PROTO = PROTOCOL_DATA.get(vtx_type, PROTO)

    start_time = time.time()

    # Prepare to upload
    conn = serial.Serial()
    conn.baudrate = UPLOAD_BAUD
    conn.stopbits = UPLOAD_STOP_BITS
    conn.port = port
    conn.timeout = 1
    conn.open()

    rl = SerialHelper.SerialHelper(conn, 2., ["CCC"], half_duplex)
    rl.clear()

    if "CCC" not in rl.read_line(2.):
        ## Send command to firmware to boot into bootloader
        dbg_print("\nAttempting to reboot into bootloader...\n")

        conn.baudrate = PROTO['baud']
        conn.stopbits = PROTO['stopbits']
        
        rl.set_timeout(2.)
        rl.set_delimiters(["\n", "CCC"])
        # request reboot
        if vtx_type in ['SA', None]:
            # Need to send dummy zero before actual data to get UART line into correct state!
            BootloaderInitSeq = bytes(
                [0x0, 0xAA, 0x55, ((0x78 << 1) & 0xFF),
                 0x03, ord('R'), ord('S'), ord('T'), 0xC3])
        elif vtx_type == 'TRAMP':
            data = [0x00] * 16
            data[0] = 0x0F      # TRAMP_HEADER = 0xF
            data[1] = ord('R')
            data[2] = ord('S')
            data[3] = ord('T')
            data[14] = (sum(data[1:]) & 0xFF)
            BootloaderInitSeq = bytes(data)
        elif vtx_type == 'MSP':
            data = [0x00] * 9
            data[0] = ord('$')
            data[1] = ord('X')
            data[2] = ord('<')
            data[3] = 0x00
            data[4] = 68       # MSP_REBOOT
            data[5] = 0x00
            data[6] = 0x00
            data[7] = 0x00
            data[8] = 0x06
            BootloaderInitSeq = bytes(data)

        # clear RX buffer before continuing
        rl.clear()
        rl.write(BootloaderInitSeq)
        rl.write(BootloaderInitSeq)
        time.sleep(.1)
        time.sleep(.1)
        time.sleep(.1)

        # setup serial for flashing
        conn.baudrate = UPLOAD_BAUD
        conn.stopbits = UPLOAD_STOP_BITS

    rl.clear()

    # sanity check! Make sure the bootloader is started
    dbg_print("Wait sync...")
    rl.set_delimiters(["CCC"])
    if "CCC" not in rl.read_line(15.):
        msg = "[FAILED] Unable to communicate with bootloader...\n"
        dbg_print(msg)
        raise Exception(msg)
    dbg_print(" sync OK\n")

    # change timeout to 5sec
    conn.timeout = 5.
    conn.write_timeout = 2.
    conn.read_timeout = 2.

    # open binary
    stream = open(filename, 'rb')
    filesize = os.stat(filename).st_size
    filechunks = filesize / 128

    dbg_print("\nuploading %d bytes...\n" % filesize)

    def StatusCallback(total_packets, success_count, error_count):
        #sys.stdout.flush()
        if (total_packets % 10 == 0):
            dbg = str(round((total_packets / filechunks) * 100)) + "%"
            if (error_count > 0):
                dbg += ", err: " + str(error_count)
            dbg_print(dbg + "\n")

    def getc(size, timeout=3):
        return conn.read(size) or None

    def putc(data, timeout=3):
        cnt = conn.write(data)
        if half_duplex:
            conn.flush()
            # Clean RX buffer in case of half duplex
            #   All written data is read into RX buffer
            conn.read(cnt)
        return cnt

    modem = XMODEM(getc, putc, mode='xmodem')
    conn.reset_input_buffer()
    #modem.log.setLevel(logging.DEBUG)
    status = modem.send(stream, retry=10, callback=StatusCallback)

    conn.close()
    stream.close()

    if (status):
        dbg_print("Success!!!!\n\n")
    else:
        dbg_print("[FAILED] Upload failed!\n\n")
        raise Exception('Failed to Upload')
    dbg_print("\n\nFlashing took: %s seconds\n\n" % int((time.time() - start_time)))


def on_upload(source, target, env):
    firmware_path = str(source[0])
    protocol = PROTOCOL_DEFAULT

    upload_port = env.get('UPLOAD_PORT', None)
    if upload_port is None:
        upload_port = serials_find.get_serial_port()

    upload_flags = env.get('UPLOAD_FLAGS', [])
    for line in upload_flags:
        flags = line.split()
        for flag in flags:
            if "PROTOCOL=" in flag:
                protocol = flag.split("=")[1]

    uart_upload(upload_port, firmware_path, protocol)


if __name__ == '__main__':
    filename = 'firmware.bin'
    protocol = PROTOCOL_DEFAULT
    try:
        filename = sys.argv[1]
    except IndexError:
        dbg_print("Filename not provided, going to use default firmware.bin")

    if 2 < len(sys.argv):
        port = sys.argv[2]
    else:
        port = serials_find.get_serial_port()

    if 3 < len(sys.argv):
        protocol = sys.argv[3]

    uart_upload(port, filename, protocol)
