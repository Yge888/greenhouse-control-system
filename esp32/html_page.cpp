const char* htmlPage = R"=====(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Web Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        h1 {
            text-align: center;
            color: #333;
        }
        .status-card {
            background-color: white;
            border-radius: 8px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .sensor-data {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }
        .sensor-item {
            background-color: #e8f4f8;
            padding: 15px;
            border-radius: 6px;
            text-align: center;
        }
        .control-panel {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
        }
        .control-item {
            background-color: #f0f0f0;
            padding: 20px;
            border-radius: 6px;
            text-align: center;
        }
        .btn {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            margin: 5px;
        }
        .btn:hover {
            background-color: #45a049;
        }
        .btn.off {
            background-color: #f44336;
        }
        .btn.off:hover {
            background-color: #da190b;
        }
        input[type="range"] {
            width: 100%;
            margin: 10px 0;
        }
        #status-message {
            text-align: center;
            color: #2196F3;
            font-weight: bold;
            margin: 10px 0;
        }
    </style>
</head>
<body>
    <h1>智能设备控制面板</h1>
    
    <div class="status-card">
        <h2>传感器数据</h2>
        <div class="sensor-data">
            <div class="sensor-item">
                <h3>温度</h3>
                <p id="temp">--°C</p>
            </div>
            <div class="sensor-item">
                <h3>湿度</h3>
                <p id="humi">--%</p>
            </div>
            <div class="sensor-item">
                <h3>光照</h3>
                <p id="light">--%</p>
            </div>
            <div class="sensor-item">
                <h3>CO2</h3>
                <p id="co2">--ppm</p>
            </div>
        </div>
    </div>
    
    <div class="status-card">
        <h2>设备控制</h2>
        <div id="status-message"></div>
        <div class="control-panel">
            <div class="control-item">
                <h3>风扇</h3>
                <button class="btn" onclick="toggleDevice('FAN')" id="fan-btn">开启</button>
            </div>
            <div class="control-item">
                <h3>水泵</h3>
                <button class="btn" onclick="toggleDevice('PUMP')" id="pump-btn">开启</button>
            </div>
            <div class="control-item">
                <h3>步进电机</h3>
                <input type="range" id="stepper-slider" min="0" max="180" value="0" onchange="setStepper(this.value)">
                <p>角度: <span id="stepper-value">0</span></p>
            </div>
        </div>
    </div>

    <script>
        // 更新设备状态显示
        function updateDeviceStatus(status) {
            document.getElementById('temp').textContent = status.temp + '°C';
            document.getElementById('humi').textContent = status.humi + '%';
            document.getElementById('light').textContent = status.light + '%';
            document.getElementById('co2').textContent = status.co2 + 'ppm';
            
            // 更新风扇状态
            const fanBtn = document.getElementById('fan-btn');
            if (status.fan) {
                fanBtn.textContent = '关闭';
                fanBtn.className = 'btn off';
            } else {
                fanBtn.textContent = '开启';
                fanBtn.className = 'btn';
            }
            
            // 更新水泵状态
            const pumpBtn = document.getElementById('pump-btn');
            if (status.pump) {
                pumpBtn.textContent = '关闭';
                pumpBtn.className = 'btn off';
            } else {
                pumpBtn.textContent = '开启';
                pumpBtn.className = 'btn';
            }
            
            // 更新步进电机状态
            document.getElementById('stepper-slider').value = status.stepper;
            document.getElementById('stepper-value').textContent = status.stepper;
        }
        
        // 切换设备状态
        function toggleDevice(device) {
            fetch('/control?device=' + device + '&action=toggle')
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        updateDeviceStatus(data.status);
                        document.getElementById('status-message').textContent = '操作成功！';
                    } else {
                        document.getElementById('status-message').textContent = '操作失败！';
                    }
                    setTimeout(() => {
                        document.getElementById('status-message').textContent = '';
                    }, 2000);
                });
        }
        
        // 设置步进电机角度
        function setStepper(angle) {
            document.getElementById('stepper-value').textContent = angle;
            fetch('/control?device=STEPPER&action=' + angle)
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        updateDeviceStatus(data.status);
                    }
                });
        }
        
        // 定期获取设备状态
        setInterval(() => {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    updateDeviceStatus(data);
                });
        }, 2000);
        
        // 页面加载时获取初始状态
        window.onload = function() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    updateDeviceStatus(data);
                });
        };
    </script>
</body>
</html>)=====";
