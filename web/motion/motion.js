// File: /home/kami/motion-guard/web/motion/motion.js
// ==============================================================================
// ARYORITHM MOTION-GUARD UI PLUGIN:
//   1. Cobot ISO 13849 EtherCAT L2 Scope (:18099)
//   2. Mining Haul Truck SocketCAN CAN-FD Steering Bus (:18100)
//   3. AMR POSIX ShmRing Obstacle & Brake Radar (:18101)
//   4. Robotics & Motion C2 Tab
// ==============================================================================

(function () {
    if (!window.SentinelUI) return;

    const HOST = window.location.hostname || "127.0.0.1";
    const COBOT_API = `http://${HOST}:18099`;
    const CANFD_API = `http://${HOST}:18100`;
    const AMR_API = `http://${HOST}:18101`;

    const isMotionNode = (node) =>
        /cobot|mining|amr|motion|ethercat|canfd|shm/i.test(`${node?.site || ""} ${node?.hostname || ""}`);

    function mountCanvas(apiUrl, drawFn) {
        return function (container) {
            container.innerHTML = `<div style="position:relative; width:100%; height:100%; background:#05080C; border-radius:4px; overflow:hidden; aspect-ratio:1.25;"><canvas width="750" height="600" style="width:100%; height:100%; object-fit:cover; display:block;"></canvas></div>`;
            const canvas = container.querySelector("canvas");
            const state = { animId: null, alive: true, data: null, tick: 0 };
            const poll = async () => {
                try { const r = await fetch(`${apiUrl}/telemetry`, { cache: "no-store" }); if (r.ok) state.data = await r.json(); } catch (_) { }
            };
            poll();
            const timer = setInterval(poll, 200);
            function loop() { if (!state.alive) return; state.tick += 0.04; drawFn(canvas, state); state.animId = requestAnimationFrame(loop); }
            loop();
            state.cleanup = () => clearInterval(timer);
            return state;
        };
    }

    function unmountCanvas(h) { if (!h) return; h.alive = false; if (h.animId) cancelAnimationFrame(h.animId); if (h.cleanup) h.cleanup(); }

    window.SentinelUI.registerApparatus({
        id: "motion_cobot_ethercat",
        title: "COBOT ISO 13849 PL-e ETHERCAT SCOPE (0x88A4)",
        shortLabel: "🦾 ETHERCAT CBF",
        aspectRatio: 1.25,
        protocol: "AF_PACKET / 0x88A4",
        matches: (n) => isMotionNode(n) && /cobot|ethercat/i.test(`${n?.site} ${n?.hostname}`),
        mount: mountCanvas(COBOT_API, (canvas, st) => {
            const ctx = canvas.getContext("2d"), w = canvas.width, h = canvas.height, cx = w / 2, cy = h / 2;
            const c = st.data?.cobot || {};
            const alert = Boolean(st.data?.active_attack?.active);
            ctx.fillStyle = "#040810"; ctx.fillRect(0, 0, w, h);

            // Safe workspace boundary circle
            ctx.strokeStyle = alert ? "#FF3366" : "#00FFA3"; ctx.lineWidth = 2;
            ctx.beginPath(); ctx.arc(cx, cy, 190, 0, Math.PI * 2); ctx.stroke();

            // Articulated 2-link arm
            const a1 = Math.sin(st.tick) * 0.9, a2 = Math.cos(st.tick * 1.4) * 1.1;
            const x1 = cx + Math.cos(a1) * 95, y1 = cy + Math.sin(a1) * 95;
            const x2 = x1 + Math.cos(a1 + a2) * (alert ? 120 : 75), y2 = y1 + Math.sin(a1 + a2) * (alert ? 120 : 75);
            ctx.strokeStyle = alert ? "#FF3366" : "#00E5FF"; ctx.lineWidth = 6;
            ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(x1, y1); ctx.lineTo(x2, y2); ctx.stroke();

            ctx.fillStyle = "rgba(7,11,20,0.88)"; ctx.fillRect(0, 0, w, 28); ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace"; ctx.fillStyle = alert ? "#FF3366" : "#00FFA3";
            ctx.fillText(alert ? `[!] ETHERCAT JOINT SURGE CLAMPED (${c.joint_accel_mps2 || 48} m/s²)` : `[+] COBOT ISO 13849 PL-e | ACCEL: ${c.joint_accel_mps2 || 4.2} m/s² | R: ${c.workspace_r_mm || 320} mm`, 12, 18);
            ctx.font = "9px monospace"; ctx.fillStyle = "#94A3B8";
            ctx.fillText(`NON-IP L2 ETHERCAT (0x88A4) | WKC: ${c.wkc || 1} | CBF CLAMPS: ${c.cbf_clamps || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({ apiBase: COBOT_API, attacks: [{ label: "[!] Inject EtherCAT 0x88A4 Joint Surge", pcap: "ethercat_surge_attack.pcap", attack_type: "surge", mode: "overlay", duration_sec: 15 }] })
    });

    window.SentinelUI.registerApparatus({
        id: "motion_mining_canfd",
        title: "MINING HAUL TRUCK CAN-FD STEERING BUS",
        shortLabel: "🚛 CAN-FD BUS",
        aspectRatio: 1.25,
        protocol: "SOCKETCAN / AF_UNIX",
        matches: (n) => isMotionNode(n) && /mining|can/i.test(`${n?.site} ${n?.hostname}`),
        mount: mountCanvas(CANFD_API, (canvas, st) => {
            const ctx = canvas.getContext("2d"), w = canvas.width, h = canvas.height, cx = w / 2, cy = h / 2;
            const m = st.data?.canfd || {};
            const alert = Boolean(st.data?.active_attack?.active);
            ctx.fillStyle = "#040812"; ctx.fillRect(0, 0, w, h);

            ctx.strokeStyle = alert ? "#FF3366" : "#00E5FF"; ctx.lineWidth = 3;
            ctx.beginPath(); ctx.arc(cx, cy, 140, Math.PI * 0.75, Math.PI * 2.25); ctx.stroke();
            const rad = ((Number(m.steer_deg || 0) - 90) * Math.PI) / 180.0;
            ctx.strokeStyle = alert ? "#FF3366" : "#00FFA3"; ctx.lineWidth = 4;
            ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(cx + Math.cos(rad) * 120, cy + Math.sin(rad) * 120); ctx.stroke();

            ctx.fillStyle = "rgba(7,11,20,0.88)"; ctx.fillRect(0, 0, w, 28); ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace"; ctx.fillStyle = alert ? "#FF3366" : "#00E5FF";
            ctx.fillText(alert ? `[!] ROGUE CAN-FD STEER OVERRIDE BLOCKED (${m.steer_deg || 68}°)` : `[+] CAN-FD ARB ${m.arb_id || "0x201"} | STEER: ${m.steer_deg || 8.4}° | TORQUE: ${m.torque_nm || 95} Nm`, 12, 18);
            ctx.font = "9px monospace"; ctx.fillStyle = "#94A3B8";
            ctx.fillText(`BUS B: AF_UNIX / SOCKETCAN BRIDGE | OVERRIDES BLOCKED: ${m.overrides_blocked || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({ apiBase: CANFD_API, attacks: [{ label: "[!] Inject Rogue CAN-FD Steer Override", pcap: "", attack_type: "steer_override", mode: "overlay", duration_sec: 15 }] })
    });

    window.SentinelUI.registerApparatus({
        id: "motion_amr_shm",
        title: "AMR SHMRING OBSTACLE & BRAKE RADAR",
        shortLabel: "🤖 AMR SHMRING",
        aspectRatio: 1.25,
        protocol: "POSIX /dev/shm RING",
        matches: (n) => isMotionNode(n) && /amr|shm/i.test(`${n?.site} ${n?.hostname}`),
        mount: mountCanvas(AMR_API, (canvas, st) => {
            const ctx = canvas.getContext("2d"), w = canvas.width, h = canvas.height, cx = w / 2, cy = h - 70;
            const a = st.data?.amr || {};
            const alert = Boolean(st.data?.active_attack?.active);
            ctx.fillStyle = "#040810"; ctx.fillRect(0, 0, w, h);

            [120, 240, 360].forEach(r => { ctx.strokeStyle = "rgba(0,229,255,0.22)"; ctx.beginPath(); ctx.arc(cx, cy, r, Math.PI, 0); ctx.stroke(); });
            const obsY = cy - Math.min(340, (Number(a.min_obstacle_m || 3.4) / 4.5) * 340);
            ctx.fillStyle = alert ? "#FF3366" : "#00FFA3";
            ctx.fillRect(cx - 35, obsY - 14, 70, 28);

            ctx.fillStyle = "rgba(7,11,20,0.88)"; ctx.fillRect(0, 0, w, 28); ctx.fillRect(0, h - 26, w, 26);
            ctx.font = "bold 10.5px monospace"; ctx.fillStyle = alert ? "#FF3366" : "#00FFA3";
            ctx.fillText(alert ? `[!] COLLISION RISK -> E-BRAKE ENGAGED (OBS=${a.min_obstacle_m}m)` : `[+] AMR SHMRING | V: ${a.velocity_mps || 1.8} m/s | OBS: ${a.min_obstacle_m || 3.4} m | BRAKE: ${a.brake_temp_c || 94}°C`, 12, 18);
            ctx.font = "9px monospace"; ctx.fillStyle = "#94A3B8";
            ctx.fillText(`BUS C: LOCK-FREE /dev/shm RING (ipc: shareable) | E-BRAKE TRIPS: ${a.ebrake_trips || 0}`, 12, h - 9);
        }),
        unmount: unmountCanvas,
        getPcapConfig: () => ({ apiBase: AMR_API, attacks: [{ label: "[!] Inject AMR Imminent Collision Vector", pcap: "", attack_type: "collision", mode: "overlay", duration_sec: 15 }] })
    });

    let c2Timer = null;
    function renderMotionC2(container) {
        if (c2Timer) clearInterval(c2Timer);
        container.innerHTML = `
            <div style="display:flex; flex-direction:column; gap:14px;">
                <div class="mot-c2-bar">
                    <span style="font-size:12px; font-weight:800; color:#00E5FF;">MOTION-GUARD: ETHERCAT, CAN-FD &amp; SHMRING SAFETY C2</span>
                    <span class="badge badge-green" style="font-size:9px;">3 NON-IP BUSES LIVE</span>
                </div>
                <div class="mot-c2-grid">
                    <div id="mot-card-cobot" class="mot-c2-card">
                        <div style="color:#00FFA3; font-weight:800;">🦾 COBOT ETHERCAT (:18099)</div>
                        <div id="mot-cobot-kpis">Connecting...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="mot-c2-btn danger" data-m-atk="cobot">INJECT JOINT SURGE</button>
                            <button class="mot-c2-btn" data-m-stop="cobot">RESTORE</button>
                        </div>
                    </div>
                    <div id="mot-card-can" class="mot-c2-card">
                        <div style="color:#00E5FF; font-weight:800;">🚛 MINING CAN-FD (:18100)</div>
                        <div id="mot-can-kpis">Connecting...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="mot-c2-btn danger" data-m-atk="can">INJECT STEER SPOOF</button>
                            <button class="mot-c2-btn" data-m-stop="can">RESTORE</button>
                        </div>
                    </div>
                    <div id="mot-card-amr" class="mot-c2-card">
                        <div style="color:#FFB800; font-weight:800;">🤖 AMR SHMRING (:18101)</div>
                        <div id="mot-amr-kpis">Connecting...</div>
                        <div style="display:flex; gap:8px;">
                            <button class="mot-c2-btn danger" data-m-atk="amr">INJECT COLLISION</button>
                            <button class="mot-c2-btn" data-m-stop="amr">RESTORE</button>
                        </div>
                    </div>
                </div>
            </div>
        `;
        const eps = {
            cobot: { url: COBOT_API, body: { attack_type: "surge", mode: "overlay", duration_sec: 15 } },
            can: { url: CANFD_API, body: { attack_type: "steer_override", mode: "overlay", duration_sec: 15 } },
            amr: { url: AMR_API, body: { attack_type: "collision", mode: "overlay", duration_sec: 15 } }
        };
        container.querySelectorAll("[data-m-atk]").forEach(b => b.addEventListener("click", () => {
            const c = eps[b.getAttribute("data-m-atk")];
            if (c) fetch(`${c.url}/attack/inject`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(c.body) }).catch(() => { });
        }));
        container.querySelectorAll("[data-m-stop]").forEach(b => b.addEventListener("click", () => {
            const c = eps[b.getAttribute("data-m-stop")];
            if (c) fetch(`${c.url}/attack/stop`, { method: "POST" }).catch(() => { });
        }));
        async function refresh() {
            const c = await fetch(`${COBOT_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            const m = await fetch(`${CANFD_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            const a = await fetch(`${AMR_API}/telemetry`, { cache: "no-store" }).then(r => r.ok ? r.json() : null).catch(() => null);
            if (c?.cobot) document.getElementById("mot-cobot-kpis").innerHTML = `<div class="mot-c2-kpi"><span>Accel</span><b>${c.cobot.joint_accel_mps2} m/s²</b></div><div class="mot-c2-kpi"><span>Status</span><b>${c.cobot.status}</b></div>`;
            if (m?.canfd) document.getElementById("mot-can-kpis").innerHTML = `<div class="mot-c2-kpi"><span>Steer</span><b>${m.canfd.steer_deg}°</b></div><div class="mot-c2-kpi"><span>Status</span><b>${m.canfd.status}</b></div>`;
            if (a?.amr) document.getElementById("mot-amr-kpis").innerHTML = `<div class="mot-c2-kpi"><span>Obstacle</span><b>${a.amr.min_obstacle_m} m</b></div><div class="mot-c2-kpi"><span>Status</span><b>${a.amr.status}</b></div>`;
        }
        refresh();
        c2Timer = setInterval(refresh, 500);
    }

    window.SentinelUI.registerTab({ id: "tab-motion-c2", label: "Robotics & CAN-FD C2", icon: "🦾", group: "Operations", renderView: renderMotionC2 });
})();
