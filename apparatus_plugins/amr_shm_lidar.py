# File: /home/kami/motion-guard/apparatus_plugins/amr_shm_lidar.py

import math
import struct


class AmrShmPlugin:
    def init_state(self, engine):
        engine.domain_state["amr"] = {
            "bus": "POSIX /dev/shm RING",
            "velocity_mps": 1.8,
            "min_obstacle_m": 3.4,
            "brake_temp_c": 94.0,
            "ebrake_trips": 0,
            "status": "CORRIDOR_CLEAR",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        a = engine.domain_state["amr"]
        if is_attack:
            a["velocity_mps"] = 4.5
            a["min_obstacle_m"] = 0.15
            a["ebrake_trips"] += 1
            a["status"] = "COLLISION_EBRAKE_ENGAGED"

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            a = engine.domain_state["amr"]
            if not engine.active_attack["active"]:
                a["velocity_mps"] = round(1.8 + math.sin(elapsed) * 0.25, 2)
                a["min_obstacle_m"] = round(3.4 + math.cos(elapsed * 0.7) * 0.6, 2)
                a["brake_temp_c"] = round(94.0 + math.sin(elapsed * 0.4) * 4.0, 1)
                a["status"] = "CORRIDOR_CLEAR"
            v, obs, bt = a["velocity_mps"], a["min_obstacle_m"], a["brake_temp_c"]
        pkt = struct.pack("<IIffff", 0x4D4F5431, 1, v, 0.2, obs, bt)
        engine.send_and_decode(pkt, is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        pkt = struct.pack("<IIffff", 0x4D4F5431, 1, 4.5, 0.2, 0.15, 94.0)
        engine.send_and_decode(pkt, is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        a = engine.domain_state["amr"]
        a["velocity_mps"] = 4.5
        a["min_obstacle_m"] = 0.15
        a["ebrake_trips"] += 1
        a["status"] = "COLLISION_EBRAKE_ENGAGED"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"amr": dict(engine.domain_state["amr"])}


APPARATUS_TYPES = {"amr_shm_lidar": AmrShmPlugin}
