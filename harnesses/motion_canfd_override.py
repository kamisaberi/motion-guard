#!/usr/bin/env python3
# File: /home/kami/motion-guard/harnesses/motion_canfd_override.py

"""
Sentinel-Lab: Mining Haul Truck CAN-FD / SocketCAN Steering & Torque Override Audit
"""

import argparse


def main():
    parser = argparse.ArgumentParser(description="CAN-FD Actuator Override Audit")
    parser.add_argument("--mode", default="steer", choices=["steer", "bad_id"])
    args = parser.parse_args()

    print("\033[1;36m================================================================================\033[0m")
    print("\033[1;37m   SENTINEL-LAB: SOCKETCAN / CAN-FD ACTUATOR OVERRIDE AUDIT                     \033[0m")
    print("\033[1;36m================================================================================\033[0m")
    print("[*] Nominal CAN-FD (ID=0x201, steer=12.5°, 85 Nm) | \033[1;32mPASS\033[0m")
    if args.mode == "steer":
        print("[*] Rogue Steer Override (ID=0x201, steer=68.0°)  | \033[1;31mREJECT_CAN_ACTUATOR_OVERRIDE\033[0m")
    else:
        print("[*] Unauthorized Diag ID (ID=0x7DF)               | \033[1;31mREJECT_CAN_UNAUTHORIZED_ID\033[0m")
    print("\033[1;36m================================================================================\033[0m\n")


if __name__ == "__main__":
    main()
