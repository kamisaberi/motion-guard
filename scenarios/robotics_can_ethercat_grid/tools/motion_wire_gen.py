#!/usr/bin/env python3
# File: /home/kami/motion-guard/scenarios/robotics_can_ethercat_grid/tools/motion_wire_gen.py

import mmap
import os
import socket
import struct
import time

SHM_PATH = "/dev/shm/sentinel_motion_ring"
CAN_SOCK = "/var/run/sentinel-bus/can.sock"
SHM_MAGIC = 0x53484D52


def try_write_shm(seq_counter: int):
    if not os.path.exists(SHM_PATH):
        return False
    try:
        fd = os.open(SHM_PATH, os.O_RDWR)
        size = os.fstat(fd).st_size
        if size < 64 + 64 * 4096:
            os.close(fd)
            return False
        mm = mmap.mmap(fd, size)
        magic, _ver, slot_cnt, slot_sz = struct.unpack("<IIII", mm[0:16])
        if magic != SHM_MAGIC or slot_cnt == 0:
            mm.close()
            os.close(fd)
            return False

        wseq = struct.unpack("<Q", mm[16:24])[0]
        slot_off = 64 + (wseq % slot_cnt) * slot_sz
        mot_payload = struct.pack("<IIffff", 0x4D4F5431, 1, 1.8, 0.2, 3.4, 94.0)
        slot_hdr = struct.pack("<QIHH", wseq, len(mot_payload), 1, 2)
        mm[slot_off:slot_off + 16] = slot_hdr
        mm[slot_off + 16:slot_off + 16 + len(mot_payload)] = mot_payload
        mm[16:24] = struct.pack("<Q", wseq + 1)
        mm.close()
        os.close(fd)
        return True
    except Exception:
        return False


def main():
    raw_ecat = None
    try:
        raw_ecat = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(0x88A4))
        raw_ecat.bind(("eth0", 0))
        print("[+] Motion Wire Gen: AF_PACKET EtherCAT (0x88A4) bound on eth0", flush=True)
    except Exception as exc:
        print(f"[-] AF_PACKET EtherCAT deferred: {exc}", flush=True)

    can_sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    seq = 1
    while True:
        # 1. Bus A: Non-IP L2 EtherCAT 0x88A4 -> multicast MAC
        if raw_ecat is not None:
            ecat_pdu = struct.pack("<H", (1 << 12) | 22) + (b"\x00" * 10) + struct.pack("<ffH", 4.2, 320.0, 1)
            l2_frame = b"\x01\x01\x05\x01\x00\x01\x02\x42\x0a\xf0\x46\x32\x88\xa4" + ecat_pdu
            try:
                raw_ecat.send(l2_frame)
            except Exception:
                pass

        # 2. Bus B: AF_UNIX CAN-FD frame -> Edge-Mining-CANFD-02
        if os.path.exists(CAN_SOCK):
            can_frame = struct.pack("<IB3xff", 0x201, 8, 8.4, 95.0)
            try:
                can_sock.sendto(can_frame, CAN_SOCK)
            except Exception:
                pass

        # 3. Bus C: Lock-free POSIX /dev/shm Ring -> Edge-AMR-ShmRing-03
        try_write_shm(seq)

        seq += 1
        time.sleep(0.5)


if __name__ == "__main__":
    main()
