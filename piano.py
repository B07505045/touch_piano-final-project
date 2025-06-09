import socket
import struct
import time
import fluidsynth

# pip3 install pyFluidSynth
fs = fluidsynth.Synth() # Initialize FluidSynth with the SoundFont
fs.start()  # start the audio driver

sfid = fs.sfload("./FluidR3_GM/FluidR3_GM.sf2")  # load the piano SoundFont
fs.program_select(0, sfid, 0, 0)    # channel, sfid, bank, preset

status = 0
cur_instruments = 0
cur_note = 0

HOST = '0.0.0.0'
PORT = 65432

print(f"Starting server at {HOST}:{PORT}")
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(1)
    conn, addr = s.accept()
    print("Connected at", addr)

    with conn:
        buffer = b''
        while True:
            data = conn.recv(1024)
            if not data:
                break
            buffer += data

            # 處理完整的2位元組資料
            while len(buffer) >= 2:
                key_bytes = buffer[:2]
                buffer = buffer[2:]
                # 解析為16位元整數（依照 STM32 設定為大端或小端，預設為小端）
                key_value = struct.unpack('>H', key_bytes)[0]  # 小端：<H；大端：>H
                # change note: +12
                if 0x00000fe0 & key_value == 0x00000fe0 and 0x00000fe0 & status != 0x00000fe0:
                    cur_note += 12
                # change note: -12
                elif 0x0000001f & key_value == 0x0000001f and 0x0000001f & status != 0x0000001f:
                    cur_note -= 12
                # change instruments
                if 0x00008000 & key_value == 0x00008000 and 0x00008000 & status == 0:
                    if cur_instruments == 0:
                        cur_instruments = 24
                        fs.program_select(0, sfid, 0, 24)   # Guitar
                    elif cur_instruments == 24:
                        cur_instruments = 40
                        fs.program_select(0, sfid, 0, 40)   # violin
                    elif cur_instruments == 40:
                        cur_instruments = 56
                        fs.program_select(0, sfid, 0, 56)   # Trumpet
                    elif cur_instruments == 56:
                        cur_instruments = 0
                        fs.program_select(0, sfid, 0, 0)   # piano
                for bit in range(12):
                    if pow(2, bit) & key_value > 0 and pow(2, bit) & status == 0:
                        if bit == 0:
                            fs.noteon(0, 61 + cur_note, 100)
                        elif bit == 1:
                            fs.noteon(0, 63 + cur_note, 100)
                        elif bit == 2:
                            fs.noteon(0, 66 + cur_note, 100)
                        elif bit == 3:
                            fs.noteon(0, 68 + cur_note, 100)
                        elif bit == 4:
                            fs.noteon(0, 70 + cur_note, 100)
                        elif bit == 5:
                            fs.noteon(0, 60 + cur_note, 100)
                        elif bit == 6:
                            fs.noteon(0, 62 + cur_note, 100)
                        elif bit == 7:
                            fs.noteon(0, 64 + cur_note, 100)
                        elif bit == 8:
                            fs.noteon(0, 65 + cur_note, 100)
                        elif bit == 9:
                            fs.noteon(0, 67 + cur_note, 100)
                        elif bit == 10:
                            fs.noteon(0, 69 + cur_note, 100)
                        elif bit == 11:
                            fs.noteon(0, 71 + cur_note, 100)
                    elif pow(2, bit) & key_value == 0 and pow(2, bit) & status > 0:
                        if bit == 0:
                            fs.noteoff(0, 61 + cur_note)
                        elif bit == 1:
                            fs.noteoff(0, 63 + cur_note)
                        elif bit == 2:
                            fs.noteoff(0, 66 + cur_note)
                        elif bit == 3:
                            fs.noteoff(0, 68 + cur_note)
                        elif bit == 4:
                            fs.noteoff(0, 70 + cur_note)
                        elif bit == 5:
                            fs.noteoff(0, 60 + cur_note)
                        elif bit == 6:
                            fs.noteoff(0, 62 + cur_note)
                        elif bit == 7:
                            fs.noteoff(0, 64 + cur_note)
                        elif bit == 8:
                            fs.noteoff(0, 65 + cur_note)
                        elif bit == 9:
                            fs.noteoff(0, 67 + cur_note)
                        elif bit == 10:
                            fs.noteoff(0, 69 + cur_note)
                        elif bit == 11:
                            fs.noteoff(0, 71 + cur_note)
                status = key_value
                print(f"Key Value: 0x{key_value:08x}", cur_instruments, cur_note)

fs.delete()
