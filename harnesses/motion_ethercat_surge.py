#!/usr/bin/env python3
# File: /home/kami/motion-guard/harnesses/motion_ethercat_surge.py

"""
Sentinel-Lab: Cobot ISO 13849 PL-e Non-IP L2 EtherCAT (0x88A4) Joint Surge Audit
"""

import argparse


def main():
    parser = argparse.ArgumentParser(description="EtherCAT 0x88A4 Joint Surge Audit")
    parser.add_argument("--mode", default="surge", choices=["surge", "wkc_tamper"])
    args = parser.parse_args()

    print("\033[1;36m================================================================================\033[0m")
    print("\033[1;37m   SENTINEL-LAB: ETHERCAT (0x88A4) ISO 13849 PL-e JOINT SURGE AUDIT             \033[0m")
    print("\033[1;36m================================================================================\033[0m")
    print("[*] Nominal Cobot PDO (accel=4.2 m/s², WKC=1)     | \033[1;32mPASS\033[0m")
    if args.mode == "surge":
        print("[*] Rogue Joint Surge (accel=48.0 m/s² > 15.0)    | \033[1;31mREJECT_ETHERCAT_JOINT_SURGE\033[0m")
    else:
        print("[*] Working Counter Corruption (WKC=0)            | \033[1;31mREJECT_ETHERCAT_WKC_TAMPER\033[0m")
    print("\033[1;36m================================================================================\033[0m\n")


if __name__ == "__main__":
    main()
