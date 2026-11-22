# File: /home/kami/motion-guard/apparatus_plugins/mining_canfd_bus.py

import math
import struct


class MiningCanFdPlugin:
    def init_state(self, engine):
        engine.domain_state["canfd"] = {
            "bus": "SOCKETCAN / CAN-FD",
            "arb_id": "0x201",
            "steer_deg": 8.4,
            "torque_nm": 95.0,
            "overrides_blocked": 0,
            "status": "CAN_BUS_SAFE",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        m = engine.domain_state["canfd"]
        if is_attack:
            m["steer_deg"] = 68.0
            m["overrides_blocked"] += 1
            m["status"] = "STEER_OVERRIDE_AIRGAPPED"

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            m = engine.domain_state["canfd"]
            if not engine.active_attack["active"]:
                m["steer_deg"] = round(math.sin(elapsed * 0.8) * 14.0, 2)
                m["torque_nm"] = round(95.0 + math.cos(elapsed * 1.1) * 18.0, 1)
                m["status"] = "CAN_BUS_SAFE"
            st, tq = m["steer_deg"], m["torque_nm"]
        pkt = struct.pack("<IB3xff", 0x201, 8, st, tq)
        engine.send_and_decode(pkt, is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        pkt = struct.pack("<IB3xff", 0x201, 8, 68.0, 120.0)
        engine.send_and_decode(pkt, is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        m = engine.domain_state["canfd"]
        m["steer_deg"] = 68.0
        m["overrides_blocked"] += 1
        m["status"] = "STEER_OVERRIDE_AIRGAPPED"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"canfd": dict(engine.domain_state["canfd"])}


APPARATUS_TYPES = {"mining_canfd_bus": MiningCanFdPlugin}
