#pragma once
#include <Arduino.h>

// Customer dashboard (pure client-side fills via /api/state)
inline String buildCustomerHtml() {
  String s;
  s.reserve(15000);
  s += R"HTML(<!doctype html><html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Touch Counter</title>
<style>
:root{--bg:#0e1621;--card:#162232;--text:#e8f1ff;--muted:#93a4bd;--accent:#00d4a6;}
*{box-sizing:border-box} body{margin:0;background:linear-gradient(180deg,#0b1220,#0e1621);color:var(--text);font-family:ui-sans-serif,system-ui,Segoe UI,Roboto,Helvetica,Arial}
.wrap{max-width:1100px;margin:24px auto;padding:0 16px}
.title{display:flex;align-items:center;gap:12px}
h1{margin:0 0 6px}
.pill{padding:6px 10px;border-radius:999px;background:#0f2d3c;color:#7fead6;font-size:12px;border:1px solid #164552}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(260px,1fr));gap:16px;margin-top:18px}
.card{background:radial-gradient(120% 100% at 100% 0%,#1a2a3e 0,#162232 40%,#142034 100%);border:1px solid #1e2e42;border-radius:16px;padding:18px;box-shadow:0 10px 30px rgba(0,0,0,.35)}
h2{margin:0 0 12px;font-size:18px;color:#bcd1ea}
.muted{color:var(--muted);font-size:14px}
.kv{margin:8px 0}.kv b{color:#bcd1ea}
.row{display:flex;gap:12px;align-items:center;flex-wrap:wrap}
.btn{background:var(--accent);color:#04261d;border:0;border-radius:10px;padding:10px 14px;font-weight:600;cursor:pointer}
.btn.secondary{background:#1f3853;color:#cfe5ff;border:1px solid #2d4a6b}
.btn.danger{background:#ff6b6b;color:#1e0f0f}
.inp{background:#0f1b2b;color:#dbe8ff;border:1px solid #2a3f5a;border-radius:10px;padding:10px;width:220px}
.tag{display:inline-block;padding:4px 8px;border:1px solid #264a62;border-radius:999px;color:#9dc7ff;font-size:12px}
.switch{position:relative;display:inline-block;width:46px;height:26px}
.switch input{opacity:0;width:0;height:0}
.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:#3a4d61;transition:.2s;border-radius:26px}
.slider:before{position:absolute;content:"";height:20px;width:20px;left:3px;top:3px;background:white;border-radius:50%;transition:.2s}
.switch input:checked + .slider{background:#00d4a6}
.switch input:checked + .slider:before{transform:translateX(20px)}
.gauge{width:150px;height:150px;border-radius:999px;background:#0d2b2b;display:flex;align-items:center;justify-content:center;border:8px solid #0a1c1c;margin:6px auto 10px}
.gauge span{font-size:40px;font-weight:800;color:#14ffd2}
.help{font-size:12px;color:#93a4bd}
</style>
</head><body><div class="wrap">
  <div class="title">
    <h1>Touch Counter</h1><span class="pill" id="sysState">All systems operational</span>
  </div>

  <div class="grid">
    <div class="card">
      <h2>Counter</h2>
      <div class="gauge"><span id="count">0</span></div>
      <p class="muted">Auto-updates every second</p>
      <div class="row">
        <button class="btn danger" id="btnReset">Reset Counter</button>
      </div>
    </div>

    <div class="card">
      <h2>Network & MQTT</h2>
      <div class="kv"><b>AP IP:</b> <span id="apIp">loading...</span></div>
      <div class="kv"><b>STA IP:</b> <span id="staIp">loading...</span></div>
      <div class="kv"><b>Connected SSID:</b> <span id="staSsid">(not connected)</span></div>
      <div class="kv"><b>RSSI:</b> <span id="rssi">-</span> dBm</div>
      <div class="kv"><b>MQTT:</b> <span id="mqttState">-</span> (QoS <span id="qos">2</span>)</div>
      <div class="row">
        <span class="tag">Status: esp32/touch/status</span>
        <span class="tag">Count: esp32/touch/count</span>
      </div>
      <div class="row">
        <label class="switch">
          <input type="checkbox" id="mqttToggle"><span class="slider"></span>
        </label>
        <span>Enable MQTT publishing</span>
      </div>
    </div>

    <div class="card">
      <h2>Wi-Fi Configuration</h2>
      <p class="help">Enter credentials to connect the STA interface (persists in NVS).</p>
      <div class="row"><input id="wifiSsid" placeholder="SSID" class="inp"></div>
      <div class="row"><input id="wifiPass" type="password" placeholder="Password" class="inp"></div>
      <div class="row">
        <button class="btn" id="btnWifi">Connect Network</button>
        <button class="btn secondary" id="btnWifiDisc">Disconnect</button>
      </div>
    </div>

    <div class="card">
      <h2>Threshold</h2>
      <p class="muted">Change touch threshold (1–255). Lower = more sensitive.</p>
      <div class="row">
        <input id="thrInput" type="number" min="1" max="255" value="30" class="inp" style="width:120px">
        <button class="btn" id="btnThr">Save Threshold</button>
      </div>
      <div class="kv"><b>Current:</b> <span id="thrVal">30</span></div>
    </div>

    <div class="card">
      <h2>System</h2>
      <div class="kv"><b>Uptime:</b> <span id="uptime">0s</span></div>
    </div>
  </div>
</div>
<script>
async function getState(){
  const r = await fetch('/api/state'); const j = await r.json();
  document.getElementById('count').textContent = j.count;

  const thrInput = document.getElementById('thrInput');
  if (document.activeElement !== thrInput) thrInput.value = j.threshold;
  document.getElementById('thrVal').textContent = j.threshold;

  document.getElementById('apIp').textContent  = j.ap_ip || '-';
  document.getElementById('staIp').textContent = j.sta_ip || 'not connected';
  document.getElementById('staSsid').textContent = j.sta_ssid || '(not connected)';
  document.getElementById('rssi').textContent  = j.rssi || '-';
  document.getElementById('mqttState').textContent = j.mqtt_enabled ? (j.mqtt_connected ? 'enabled & connected' : 'enabled, not connected') : 'disabled';
  document.getElementById('qos').textContent = j.qos || 2;
  document.getElementById('mqttToggle').checked = !!j.mqtt_enabled;
  document.getElementById('sysState').textContent = j.sta_connected ? 'All systems operational' : 'Local AP mode only';

  const s=j.uptime_s||0, h=Math.floor(s/3600), m=Math.floor((s%3600)/60), sec=s%60;
  document.getElementById('uptime').textContent = `${h}h ${m}m ${sec}s`;
}
async function post(u,b){ return fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b)}); }
document.getElementById('btnReset').addEventListener('click', async()=>{ await post('/api/reset',{}); await getState(); });
document.getElementById('btnThr').addEventListener('click', async()=>{
  const v=parseInt(document.getElementById('thrInput').value||'30',10);
  const resp = await post('/api/threshold',{value:v});
  const j = await resp.json();
  document.getElementById('thrVal').textContent = (j && j.threshold!=null)? j.threshold : v;
  document.getElementById('thrInput').value     = (j && j.threshold!=null)? j.threshold : v;
});
document.getElementById('mqttToggle').addEventListener('change', async(e)=>{ await post('/api/mqtt',{enabled:e.target.checked}); await getState(); });
document.getElementById('btnWifi').addEventListener('click', async()=>{
  const ssid=document.getElementById('wifiSsid').value.trim();
  const pass=document.getElementById('wifiPass').value;
  if(!ssid){ alert('SSID required'); return; }
  await post('/api/wifi',{ssid:ssid,password:pass});
  setTimeout(getState,1500);
});
document.getElementById('btnWifiDisc').addEventListener('click', async()=>{ await post('/api/disconnect',{}); setTimeout(getState,800); });
setInterval(getState,1000); getState();
</script>
</body></html>)HTML";
  return s;
}

// Company console; pass in the AP SSID to render
inline String buildCompanyHtml(const char* apSsid) {
  String s;
  s.reserve(6000);
  s += String(R"HTML(<!doctype html><html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Company Console (DRD)</title>
<style>
body{margin:0;font-family:system-ui,Segoe UI,Roboto;background:#0e1621;color:#e8f1ff}
.wrap{max-width:900px;margin:26px auto;padding:0 16px}
.card{background:#162232;border:1px solid #22344a;border-radius:16px;padding:18px;box-shadow:0 10px 30px rgba(0,0,0,.35);margin-bottom:16px}
.pill{display:inline-block;background:#0f2d3c;color:#79e7d3;border:1px solid #15505f;padding:6px 10px;border-radius:999px;font-size:12px}
.btn{background:#00d4a6;border:0;color:#04261d;padding:10px 14px;border-radius:10px;font-weight:700;cursor:pointer}
kbd{background:#0f1b2b;border:1px solid #264a62;border-radius:6px;padding:2px 6px}
</style>
</head><body>
  <div class="wrap">
    <h1>Company Mode (via Double Reset or Double-Tap T6)</h1>
    <div class="pill">AP: )HTML") + apSsid + R"HTML(</div>

    <div class="card">
      <h2>Quick Test</h2>
      <p>This page is isolated from the customer UI. Use it for engineering/service tasks.</p>
      <button class="btn" id="btnPing">Ping Backend</button>
      <p id="out"></p>
    </div>

    <div class="card">
      <h2>How to exit</h2>
      <p>Press the reset button once to boot back into <b>Customer Mode</b>.</p>
    </div>
  </div>
<script>
document.getElementById('btnPing').addEventListener('click', async ()=>{
  const r = await fetch('/svc/ping',{method:'POST'});
  const j = await r.json();
  document.getElementById('out').textContent = 'Reply: ' + JSON.stringify(j);
});
</script>
</body></html>)HTML";
  return s;
}
