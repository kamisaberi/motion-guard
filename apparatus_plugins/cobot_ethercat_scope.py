# File: /home/kami/motion-guard/apparatus_plugins/cobot_ethercat_scope.py

import math
import struct


class CobotEthercatPlugin:
    def init_state(self, engine):
        engine.domain_state["cobot"] = {
            "ethertype": "0x88A4 (ETHERCAT L2)",
            "joint_accel_mps2": 4.2,
            "workspace_r_mm": 320.0,
            "wkc": 1,
            "cbf_clamps": 0,
            "status": "ISO13849_PLE_NOMINAL",
        }

    def decode_packet(self, engine, payload: bytes, is_attack: bool, src_ip: str = ""):
        c = engine.domain_state["cobot"]
        if is_attack:
            c["joint_accel_mps2"] = 48.0
            c["cbf_clamps"] += 1
            c["status"] = "CBF_JOINT_SURGE_CLAMPED"

    def emit_nominal_step(self, engine, elapsed: float):
        with engine.lock:
            c = engine.domain_state["cobot"]
            if not engine.active_attack["active"]:
                c["joint_accel_mps2"] = round(4.2 + math.sin(elapsed * 2.0) * 1.1, 2)
                c["workspace_r_mm"] = round(320.0 + math.cos(elapsed * 1.2) * 35.0, 1)
                c["status"] = "ISO13849_PLE_NOMINAL"
            acc, rad = c["joint_accel_mps2"], c["workspace_r_mm"]
        pkt = struct.pack("<H", (1 << 12) | 22) + (b"\x00" * 10) + struct.pack("<ffH", acc, rad, 1)
        engine.send_and_decode(pkt, is_attack=False)

    def emit_attack_step(self, engine, attack_type: str):
        pkt = struct.pack("<H", (1 << 12) | 22) + (b"\x00" * 10) + struct.pack("<ffH", 48.0, 320.0, 1)
        engine.send_and_decode(pkt, is_attack=True)

    def on_attack_triggered(self, engine, attack_type: str):
        c = engine.domain_state["cobot"]
        c["joint_accel_mps2"] = 48.0
        c["cbf_clamps"] += 1
        c["status"] = "CBF_JOINT_SURGE_CLAMPED"

    def on_attack_stopped(self, engine):
        self.init_state(engine)

    def handle_post(self, engine, path: str, req: dict):
        return None

    def get_snapshot(self, engine) -> dict:
        return {"cobot": dict(engine.domain_state["cobot"])}


APPARATUS_TYPES = {"cobot_ethercat_scope": CobotEthercatPlugin}
