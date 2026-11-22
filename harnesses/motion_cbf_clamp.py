#!/usr/bin/env python3
# File: /home/kami/motion-guard/harnesses/motion_cbf_clamp.py

"""
Sentinel-Lab: Nagumo Control Barrier Function (CBF) Motion Velocity & Workspace Clamp
"""

import argparse


def main():
    parser = argparse.ArgumentParser(description="Nagumo CBF Motion Clamp Audit")
    parser.add_argument("--v-max", type=float, default=2.5)
    parser.add_argument("--alpha", type=float, default=4.0)
    args = parser.parse_args()

    print("\033[1;36m================================================================================\033[0m")
    print("\033[1;37m   SENTINEL-LAB: NAGUMO CBF MOTION BOUNDARY CLAMP AUDIT                         \033[0m")
    print("\033[1;36m================================================================================\033[0m")
    for label, v, a in [("Nominal Motion", 1.0, 0.5), ("Aggressive Accel Breach", 2.3, 3.0)]:
        h = args.v_max - abs(v)
        inv = (-a if v >= 0 else a) + args.alpha * h
        ok = h >= 0.0 and inv >= 0.0
        col = "\033[1;32mPASS\033[0m" if ok else "\033[1;31mREJECT_CBF_BOUNDARY_BREACH\033[0m"
        print(f"[*] {label:<26} | v={v:.1f}m/s a={a:.1f} | h={h:+.2f} dh/dt+ah={inv:+.2f} -> {col}")
    print("\033[1;36m================================================================================\033[0m\n")


if __name__ == "__main__":
    main()
