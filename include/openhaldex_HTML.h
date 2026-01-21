
// HTML pages as strings (embedded)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Haldex Control</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: black;
            min-height: 100vh;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            padding: 30px;
            max-width: 800px;
            width: 100%;
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 30px;
            font-size: 28px;
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 15px;
            margin-bottom: 30px;
        }
        .status-card {
            background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
            border-radius: 12px;
            padding: 20px;
            text-align: center;
            border: 2px solid #e0e0e0;
        }
        .status-label {
            font-size: 12px;
            text-transform: uppercase;
            color: #666;
            margin-bottom: 8px;
            font-weight: 600;
        }
        .status-value {
            font-size: 24px;
            font-weight: bold;
            color: #333;
        }
        .status-card.active {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            border-color: #667eea;
        }
        .status-card.active .status-label,
        .status-card.active .status-value {
            color: white;
        }
        .controls {
            display: flex;
            flex-direction: column;
            gap: 15px;
        }
        .control-button {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 18px;
            font-size: 18px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .control-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(102, 126, 234, 0.6);
        }
        .control-button.off {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
        }
        .next-button {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚗 Haldex Control</h1>
        <div class="status-grid">
            <div class="status-card" id="modeStatus">
                <div class="status-label">Actual Mode Status</div>
                <div class="status-value" id="modeValue">STOCK</div>
            </div>
            <div class="status-card" id="lockTarget">
                <div class="status-label">Wanted Lock Status</div>
                <div class="status-value" id="lockTargetValue">0%</div>
            </div>
            <div class="status-card" id="lockActual">
                <div class="status-label">Actual Lock Status</div>
                <div class="status-value" id="lockActualValue">0%</div>
            </div>
            <div class="status-card" id="speedStatus">
                <div class="status-label">Vehicle Speed</div>
                <div class="status-value" id="speedValue">0 km/h</div>
            </div>
            <div class="status-card" id="pedalStatus">
                <div class="status-label">Pedal Value</div>
                <div class="status-value" id="pedalValue">0 %</div>
            </div>
            <div class="status-card" id="RPMStatus">
                <div class="status-label">Vehicle RPM</div>
                <div class="status-value" id="RPMValue">0 RPM</div>
            </div>
        </div>
       <h2 style="text-align:center; margin: 25px 0 15px 0; color:#333;">System Flags</h2>
<div id="flagGrid" style="
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 10px 25px;
    max-width: 500px;
    margin: 0 auto 30px auto;
">
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="clutch1Flag" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="clutch1Flag">Clutch 1 Active</label>
        </li>
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="tempProtectFlag" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="tempProtectFlag">Temperature Protection</label>
        </li>
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="clutch2Flag" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="clutch2Flag">Clutch 2 Active</label>
        </li>
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="couplingOpenFlag" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="couplingOpenFlag">Coupling Open</label>
        </li>
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="received_status_bit_4" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="received_status_bit_4">Status Bit 4</label>
        </li>
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="received_status_bit_5" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="received_status_bit_5">Status Bit 5</label>
        </li>
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="speedLimitFlag" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="speedLimitFlag">Speed Limit Active</label>
        </li>
        <li style="display: flex; align-items: center; margin: 8px 0;">
            <input type="checkbox" id="received_status_bit_7" style="margin-right: 10px; accent-color: #00ff62;">
            <label for="received_status_bit_7">Status Bit 7</label>
        </li>
    </ul>
    </div>
        <div class="controls">
            <button class="control-button off" onclick="setMode(0)">Stock Mode</button>
            <button class="control-button" onclick="setMode(1)">FWD Mode</button>
            <button class="control-button" onclick="setMode(2)">75/25 Mode</button>
            <button class="control-button" onclick="setMode(3)">50/50 Mode</button>
            <button class="control-button" onclick="setMode(4)">Custom Mode</button>
            <button class="control-button next-button" onclick="location.href='/custom'">Custom Mode Setup</button>
            <button class="control-button next-button" onclick="location.href='/canlog'">CAN history</button>
        </div>
    </div>
    <script>
        const MODE_NAMES = ['STOCK', 'FWD', '75/25', '50/50', 'CUSTOM'];
        function updateDisplay() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('modeValue').textContent = MODE_NAMES[data.mode] || 'UNKNOWN';
                    document.getElementById('lockTargetValue').textContent = data.lock_target + '%';
                    document.getElementById('lockActualValue').textContent = data.haldex_engagement + '%';
                    document.getElementById('speedValue').textContent = data.vehicle_speed + ' km/h';
                    document.getElementById('modeStatus').className = data.mode !== 0 ? 'status-card active' : 'status-card';
                    document.getElementById('lockActual').className = data.haldex_engagement > 0 ? 'status-card active' : 'status-card';
                    document.getElementById('pedalValue').textContent = data.pedal_value + '%';
                    document.getElementById('RPMValue').textContent = data.vehicle_rpm + ' RPM';

                    document.getElementById('clutch1Flag').checked = data.received_report_clutch1;
                    document.getElementById('tempProtectFlag').checked = data.received_temp_protection;
                    document.getElementById('clutch2Flag').checked = data.received_report_clutch2;
                    document.getElementById('couplingOpenFlag').checked = data.received_coupling_open;
                    document.getElementById('received_status_bit_4').checked = data.received_status_bit_4;
                    document.getElementById('received_status_bit_5').checked = data.received_status_bit_5;
                    document.getElementById('speedLimitFlag').checked = data.received_speed_limit;
                    document.getElementById('received_status_bit_7').checked = data.received_status_bit_7;

                });
        }
        function setMode(mode) {
            fetch('/api/setMode', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ mode: mode })
            }).then(() => updateDisplay());
        }
        setInterval(updateDisplay, 120);
        updateDisplay();
    </script>
</body>
</html>
)rawliteral";

const char custom_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Custom Mode Setup</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            padding: 30px;
            max-width: 1000px;
            margin: 0 auto;
        }
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
        }
        h1 { color: #333; font-size: 28px; }
        .back-button {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            cursor: pointer;
            font-weight: 600;
        }
        .info-box {
            background: #f0f4ff;
            border-left: 4px solid #667eea;
            padding: 15px;
            margin-bottom: 25px;
            border-radius: 8px;
        }
        .chart-container {
            background: #f8f9fa;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 25px;
            height: 350px;
            position: relative;
        }
        canvas {
            width: 100% !important;
            height: 100% !important;
        }
        .lockpoint-item {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 15px;
            transition: all 0.3s ease;
        }
        .lockpoint-item:hover {
            border-color: #667eea;
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.2);
        }
        .lockpoint-header {
            display: flex;
            justify-content: space-between;
            margin-bottom: 15px;
        }
        .delete-btn {
            background: #f5576c;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 6px 12px;
            cursor: pointer;
        }
        .lockpoint-inputs {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
        }
        .input-group { display: flex; flex-direction: column; }
        .input-group label {
            font-size: 12px;
            color: #666;
            margin-bottom: 5px;
            font-weight: 600;
            text-transform: uppercase;
        }
        .input-group input {
            padding: 10px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 16px;
            transition: all 0.3s ease;
        }
        .input-group input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        .actions { display: flex; gap: 15px; margin-top: 20px; flex-wrap: wrap; }
        .btn {
            flex: 1;
            min-width: 150px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 15px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(102, 126, 234, 0.4);
        }
        .btn.add { background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); }
        .btn.load { background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); }
        .status { padding: 15px; border-radius: 8px; margin-bottom: 20px; display: none; font-weight: 600; }
        .status.success { background: #d4edda; color: #155724; display: block; border: 1px solid #c3e6cb; }
        .status.error { background: #f8d7da; color: #721c24; display: block; border: 1px solid #f5c6cb; }
        @media (max-width: 768px) {
            .lockpoint-inputs { grid-template-columns: 1fr; }
            .actions { flex-direction: column; }
            .btn { min-width: 100%; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>⚙️ Custom Mode Setup</h1>
            <button class="back-button" onclick="location.href='/'">← Back</button>
        </div>
        <div class="info-box">
            <p><strong>Custom Mode:</strong> Define lock points based on vehicle speed. Each point specifies the Haldex engagement percentage at a given speed. The system will interpolate between points.</p>
        </div>
        <div id="status" class="status"></div>
        <div class="chart-container">
            <canvas id="chart"></canvas>
        </div>
        <div id="lockpoints"></div>
        <div class="actions">
            <button class="btn add" onclick="addPoint()">+ Add Lock Point</button>
            <button class="btn load" onclick="loadConfig()">📥 Load</button>
            <button class="btn" onclick="saveConfig()">💾 Save</button>
        </div>
    </div>
    <script>
        let points = [];
        let canvas, ctx;
        
        function initChart() {
            canvas = document.getElementById('chart');
            ctx = canvas.getContext('2d');
            resizeCanvas();
            window.addEventListener('resize', resizeCanvas);
        }
        
        function resizeCanvas() {
            const container = canvas.parentElement;
            canvas.width = container.clientWidth - 40;
            canvas.height = container.clientHeight - 40;
            drawChart();
        }
        
        function drawChart() {
            if (!ctx) return;
            
            const w = canvas.width;
            const h = canvas.height;
            const padding = 50;
            const graphW = w - padding * 2;
            const graphH = h - padding * 2;
            
            // Clear
            ctx.clearRect(0, 0, w, h);
            
            // Background
            ctx.fillStyle = '#ffffff';
            ctx.fillRect(padding, padding, graphW, graphH);
            
            // Grid
            ctx.strokeStyle = '#e0e0e0';
            ctx.lineWidth = 1;
            for (let i = 0; i <= 10; i++) {
                const x = padding + (graphW / 10) * i;
                const y = padding + (graphH / 10) * i;
                ctx.beginPath();
                ctx.moveTo(x, padding);
                ctx.lineTo(x, padding + graphH);
                ctx.stroke();
                ctx.beginPath();
                ctx.moveTo(padding, y);
                ctx.lineTo(padding + graphW, y);
                ctx.stroke();
            }
            
            // Axes
            ctx.strokeStyle = '#333';
            ctx.lineWidth = 2;
            ctx.strokeRect(padding, padding, graphW, graphH);
            
            // Labels
            ctx.fillStyle = '#666';
            ctx.font = '12px sans-serif';
            ctx.textAlign = 'center';
            
            // X-axis labels (speed)
            for (let i = 0; i <= 10; i++) {
                const x = padding + (graphW / 10) * i;
                const speed = (200 / 10) * i;
                ctx.fillText(speed, x, h - 20);
            }
            
            // Y-axis labels (lock %)
            ctx.textAlign = 'right';
            for (let i = 0; i <= 10; i++) {
                const y = padding + graphH - (graphH / 10) * i;
                const lock = (100 / 10) * i;
                ctx.fillText(lock + '%', padding - 10, y + 4);
            }
            
            // Axis titles
            ctx.textAlign = 'center';
            ctx.font = 'bold 14px sans-serif';
            ctx.fillStyle = '#333';
            ctx.fillText('Vehicle Speed (km/h)', w / 2, h - 5);
            
            ctx.save();
            ctx.translate(15, h / 2);
            ctx.rotate(-Math.PI / 2);
            ctx.fillText('Lock Engagement (%)', 0, 0);
            ctx.restore();
            
            if (points.length === 0) return;
            
            // Sort points by speed
            const sorted = [...points].sort((a, b) => a.speed - b.speed);
            
            // Draw line
            ctx.strokeStyle = '#667eea';
            ctx.lineWidth = 3;
            ctx.beginPath();
            sorted.forEach((p, i) => {
                const x = padding + (p.speed / 200) * graphW;
                const y = padding + graphH - (p.lock / 100) * graphH;
                if (i === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            });
            ctx.stroke();
            
            // Draw points
            sorted.forEach(p => {
                const x = padding + (p.speed / 200) * graphW;
                const y = padding + graphH - (p.lock / 100) * graphH;
                
                ctx.fillStyle = '#667eea';
                ctx.beginPath();
                ctx.arc(x, y, 6, 0, Math.PI * 2);
                ctx.fill();
                
                ctx.strokeStyle = '#fff';
                ctx.lineWidth = 2;
                ctx.stroke();
            });
        }
        
        function render() {
            const html = points.map((p, i) => `
                <div class="lockpoint-item">
                    <div class="lockpoint-header">
                        <span><strong>Lock Point ${i + 1}</strong></span>
                        <button class="delete-btn" onclick="delPoint(${i})">Delete</button>
                    </div>
                    <div class="lockpoint-inputs">
                        <div class="input-group">
                            <label>Speed (km/h)</label>
                            <input type="number" min="0" max="255" value="${p.speed}" 
                                   onchange="points[${i}].speed=parseInt(this.value);drawChart()">
                        </div>
                        <div class="input-group">
                            <label>Lock Engagement (%)</label>
                            <input type="number" min="0" max="100" value="${p.lock}" 
                                   onchange="points[${i}].lock=parseInt(this.value);drawChart()">
                        </div>
                    </div>
                </div>
            `).join('');
            document.getElementById('lockpoints').innerHTML = html;
            drawChart();
        }
        
        function addPoint() {
            if (points.length >= 10) return alert('Max 10 points');
            points.push({speed: 0, lock: 0});
            render();
        }
        
        function delPoint(i) {
            points.splice(i, 1);
            render();
        }
        
        function showStatus(msg, type) {
            const el = document.getElementById('status');
            el.textContent = msg;
            el.className = 'status ' + type;
            setTimeout(() => el.style.display = 'none', 3000);
        }
        
        function loadConfig() {
            fetch('/api/getCustomMode')
                .then(r => r.json())
                .then(data => {
                    points = data.lockpoints || [];
                    render();
                    showStatus('Configuration loaded successfully!', 'success');
                })
                .catch(() => showStatus('Load failed', 'error'));
        }
        
        function saveConfig() {
            if (points.length === 0) {
                showStatus('Please add at least one lock point', 'error');
                return;
            }
            fetch('/api/setCustomMode', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ lockpoint_count: points.length, lockpoints: points })
            })
            .then(() => showStatus('Configuration saved successfully!', 'success'))
            .catch(() => showStatus('Save failed', 'error'));
        }
        
        initChart();
        loadConfig();
    </script>
</body>
</html>
)rawliteral";

const char canlog_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>CAN Frame History</title>
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: #111;
      color: #eee;
      padding: 20px;
    }
    h1 {
      text-align: center;
      color: #4facfe;
      margin-bottom: 20px;
    }
    .controls {
      text-align: center;
      margin-bottom: 15px;
    }
    .toggle-btn {
      background: #4facfe;
      color: white;
      border: none;
      border-radius: 8px;
      padding: 10px 20px;
      font-weight: bold;
      cursor: pointer;
    }
    .section {
      margin-bottom: 30px;
      max-height: 250px;
      overflow-y: auto;
      border: 1px solid #333;
      border-radius: 8px;
      padding: 5px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      background: #222;
    }
    th, td {
      padding: 6px 8px;
      border-bottom: 1px solid #333;
      text-align: left;
      font-family: monospace;
    }
    th {
      background: #333;
      color: #00f2fe;
      position: sticky;
      top: 0;
    }
    tr:hover {
      background: #2a2a2a;
    }
    tr.new {
      background: #006600 !important;
      color: #fff;
    }
    .back {
      display: inline-block;
      margin-bottom: 15px;
      background: #4facfe;
      color: white;
      padding: 10px 20px;
      border-radius: 8px;
      text-decoration: none;
      font-weight: bold;
    }
  </style>
</head>
<body>
  <a href="/" class="back">← Back</a>
  <h1>📡 CAN Frame History</h1>
  <div class="controls">
    <button id="toggleBtn" class="toggle-btn">Pause Updates</button>
  </div>
  <div id="content"></div>

  <script>
    let autoUpdate = true;
    let intervalId = null;

    async function loadData() {
      try {
        // Fetch both endpoints in parallel
        const [haldexRes, bodyRes] = await Promise.all([
          fetch('/api/haldexHistory'),
          fetch('/api/bodyHistory')
        ]);
        const haldexData = await haldexRes.json();
        const bodyData = await bodyRes.json();

        const sections = [
          ["Haldex Inbox", haldexData.haldex_inbox],
          ["Haldex Outbox", haldexData.haldex_outbox],
          ["Body Inbox", bodyData.body_inbox],
          ["Body Outbox", bodyData.body_outbox],
        ];

        const html = sections.map(([title, frames]) => {
          if (!frames) return '';
          return `
          <div class="section">
            <h2>${title}</h2>
            <table>
              <tr><th>Timestamp</th><th>ID</th><th>Data</th></tr>
              ${frames.map((f, i) => `
                <tr class="${i === frames.length - 1 ? 'new' : ''}">
                  <td>${f.timestamp ?? '—'}</td>
                  <td>0x${Number(f.id).toString(16).toUpperCase().padStart(3, '0')}</td>
                  <td>${f.data}</td>
                </tr>
              `).join('')}
            </table>
          </div>`;
        }).join('');

        document.getElementById('content').innerHTML = html;

      } catch (err) {
        console.error("Error loading CAN data:", err);
        document.getElementById('content').innerHTML = "<p style='color:red;text-align:center;'>Error loading CAN data</p>";
      }
    }

    function startUpdates() {
      if (!intervalId) {
        intervalId = setInterval(loadData, 500);
      }
      autoUpdate = true;
      document.getElementById('toggleBtn').textContent = "Pause Updates";
    }

    function stopUpdates() {
      if (intervalId) {
        clearInterval(intervalId);
        intervalId = null;
      }
      autoUpdate = false;
      document.getElementById('toggleBtn').textContent = "Resume Updates";
    }

    document.getElementById('toggleBtn').addEventListener('click', () => {
      if (autoUpdate) stopUpdates();
      else startUpdates();
    });

    // Start auto-updating by default
    startUpdates();
    loadData();
  </script>
</body>
</html>
)rawliteral";