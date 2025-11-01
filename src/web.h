#include <pgmspace.h>
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-cn">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Camera Control</title>
    <style>
        /* ======= 全局重置与基础 ======= */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', 'Arial', sans-serif;
            background: #f6fafd;
            min-height: 100vh;
        }

        /* ======= 顶部导航 ======= */
        header {
            width: 100%;
            padding: 24px 0 16px 0;
            background: linear-gradient(90deg, #3ce8ff 0%, #21d86d 100%);
            box-shadow: 0 2px 8px rgba(60, 232, 255, 0.08);
            text-align: center;
        }

        header .logo {
            font-size: 2em;
            font-weight: 700;
            color: #222;
            letter-spacing: 2px;
        }

        /* ======= 表单容器 ======= */
        .form-container {
            max-width: 380px;
            margin: 48px auto 0 auto;
            background: #fff;
            border-radius: 18px;
            box-shadow: 0 4px 32px rgba(60, 232, 255, 0.10), 0 1.5px 8px rgba(0, 0, 0, 0.06);
            padding: 32px 28px 24px 28px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .form-container form {
            width: 100%;
            display: flex;
            flex-direction: column;
            gap: 20px;
        }

        .form-container label {
            font-size: 1.08em;
            color: #222;
            margin-bottom: 4px;
            display: flex;
            flex-direction: column;
            gap: 4px;
        }

        /* ======= 输入框与下拉 ======= */
        .form-container input[type="number"],
        .styled-select {
            padding: 10px 12px;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            font-size: 1em;
            background: #f8fafd;
            outline: none;
            transition: border-color 0.2s;
        }

        .form-container input[type="number"]:focus,
        .styled-select:focus {
            border-color: #21d86d;
            background: #fff;
        }

        .styled-select {
            width: 100%;
        }

        /* ======= 按钮 ======= */
        .button {
            padding: 12px 0;
            font-size: 1.1em;
            border-radius: 8px;
            background: linear-gradient(90deg, #3ce8ff 0%, #21d86d 100%);
            color: #fff;
            border: none;
            box-shadow: 0 2px 8px rgba(60, 232, 255, 0.10);
            cursor: pointer;
            font-weight: 600;
            letter-spacing: 1px;
            transition: background 0.2s, box-shadow 0.2s;
        }

        .button:hover {
            background: linear-gradient(90deg, #21d86d 0%, #3ce8ff 100%);
            box-shadow: 0 4px 16px rgba(60, 232, 255, 0.18);
        }

        /* ======= Switch开关 ======= */
        .switch {
            position: relative;
            display: inline-block;
            width: 44px;
            height: 24px;
            vertical-align: middle;
        }

        .switch input {
            display: none;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #e0e0e0;
            transition: .4s;
            border-radius: 24px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 18px;
            width: 18px;
            left: 3px;
            bottom: 3px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }

        input:checked+.slider {
            background-color: #21d86d;
        }

        input:checked+.slider:before {
            transform: translateX(20px);
        }

        /* ======= 响应式 ======= */
        @media screen and (max-width: 600px) {
            .form-container {
                margin: 24px 8px 0 8px;
                padding: 18px 8px;
            }

            header .logo {
                font-size: 1.3em;
            }
        }

        /* ======= 预计时间显示 ======= */
        #totalTime {
            margin: 10px 0;
            color: #21d86d;
            font-weight: bold;
            font-size: 1.08em;
            text-align: left;
            line-height: 1.6;
        }

        .button-row {
            display: flex;
            gap: 18px;
            margin-top: 22px;
            width: 100%;
        }

        .start-btn {
            flex: 1;
            background: linear-gradient(90deg, #21d86d 0%, #3ce8ff 100%);
            color: #fff;
            border: none;
            border-radius: 8px;
            font-size: 1.1em;
            font-weight: 600;
            transition: background 0.2s, box-shadow 0.2s;
        }

        .start-btn:hover {
            background: linear-gradient(90deg, #3ce8ff 0%, #21d86d 100%);
            box-shadow: 0 4px 16px rgba(33, 216, 109, 0.18);
        }

        .stop-btn {
            flex: 1;
            background: linear-gradient(90deg, #ff4d4f 0%, #ff7b7c 100%);
            color: #fff;
            border: none;
            border-radius: 8px;
            font-size: 1.1em;
            font-weight: 600;
            transition: background 0.2s, box-shadow 0.2s;
        }

        .stop-btn:hover {
            background: linear-gradient(90deg, #ff7b7c 0%, #ff4d4f 100%);
            box-shadow: 0 4px 16px rgba(255, 77, 79, 0.18);
        }
    </style>
</head>

<body>
    <!-- 顶部导航栏 -->
    <header>
        <span class="logo">ESP32 Camera Control</span>
    </header>
    <!-- 表单区域 -->
    <div class="form-container">
        <form id="shootForm" onsubmit="saveParams(event)">
            <!-- Bule模式开关 -->
            <label style="align-items:center;flex-direction:row;">
                <span style="margin-right:12px;">Bule模式（手动快门速度）</span>
                <label class="switch">
                    <input type="checkbox" id="buleMode" name="buleMode" onchange="toggleBuleMode()">
                    <span class="slider"></span>
                </label>
            </label>
            <!-- 拍摄参数 -->
            <label>拍摄间隔（秒）：
                <input type="number" id="interval" name="interval" min="1" required>
            </label>
            <label>拍摄张数：
                <input type="number" id="count" name="count" min="1" required>
            </label>
            <!-- 快门速度选择 -->
            <label id="shutterLabel">
                快门速度：
                <select id="shutterSelect" name="shutterSelect" onchange="shutterSelectChange()" class="styled-select"
                    style="display:inline;">
                    <option value="0.000125">1/8000</option>
                    <option value="0.00025">1/4000</option>
                    <option value="0.0005">1/2000</option>
                    <option value="0.001">1/1000</option>
                    <option value="0.002">1/500</option>
                    <option value="0.004">1/250</option>
                    <option value="0.008">1/125</option>
                    <option value="0.016">1/60</option>
                    <option value="0.033">1/30</option>
                    <option value="1">1</option>
                    <option value="2">2</option>
                    <option value="5">5</option>
                    <option value="10">10</option>
                    <option value="20">20</option>
                    <option value="30">30</option>
                </select>
                <input type="number" id="shutterInput" name="shutterInput" min="1" max="600" style="display:none;"
                    placeholder="秒" class="styled-input">
            </label>
            <!-- 预计拍摄时间显示 -->
            <div id="totalTime"></div>
            <button type="submit" class="button">保存参数</button>
        </form>
        <div class="button-row">
            <button id="startBtn" class="button start-btn">开始</button>
            <button id="stopBtn" class="button stop-btn">停止</button>
        </div>
    </div>
    <script>
    // ======= 全局变量 =======
    let savedInterval = null;
    let savedCount = null;
    let savedShutter = null;
    let buleMode = false;

    // ======= Bule模式切换逻辑 =======
    function toggleBuleMode() {
        buleMode = document.getElementById('buleMode').checked;
        document.getElementById('shutterSelect').disabled = buleMode;
        document.getElementById('shutterInput').disabled = !buleMode;
        if (buleMode) {
            document.getElementById('shutterSelect').style.display = 'none';
            document.getElementById('shutterInput').style.display = 'inline';
        } else {
            document.getElementById('shutterSelect').style.display = 'inline';
            document.getElementById('shutterInput').style.display = 'none';
        }
        calcTotalTime();
    }

    // ======= 快门速度选择逻辑 =======
    function shutterSelectChange() {
        const val = document.getElementById('shutterSelect').value;
        if (parseFloat(val) > 30) {
            buleMode = true;
            document.getElementById('buleMode').checked = true;
            toggleBuleMode();
            document.getElementById('shutterInput').value = val;
        } else {
            buleMode = false;
            document.getElementById('buleMode').checked = false;
            document.getElementById('shutterSelect').disabled = false;
            document.getElementById('shutterInput').disabled = true;
            document.getElementById('shutterInput').style.display = 'none';
            document.getElementById('shutterSelect').style.display = 'inline';
        }
        calcTotalTime();
    }

    // ======= 获取快门速度值 =======
    function getShutterValue() {
        if (buleMode) {
            return Number(document.getElementById('shutterInput').value) || 0;
        } else {
            return Number(document.getElementById('shutterSelect').value);
        }
    }

    // ======= 预计拍摄时间计算与显示 =======
    function calcTotalTime() {
        const interval = Number(document.getElementById('interval').value) || 0;
        const count = Number(document.getElementById('count').value) || 0;
        const shutter = getShutterValue();
        let total = 0;
        if (count > 0) {
            if (shutter > 0) {
                total = (count - 1) * interval + count * shutter;
            } else {
                total = (count - 1) * interval;
            }
        }
        // 转换为天、小时、分钟、秒
        let seconds = Math.floor(total);
        let days = Math.floor(seconds / 86400);
        seconds %= 86400;
        let hours = Math.floor(seconds / 3600);
        seconds %= 3600;
        let minutes = Math.floor(seconds / 60);
        seconds %= 60;
        let result = '';
        if (days > 0) result += `${days}天`;
        if (hours > 0) result += `${hours}小时`;
        if (minutes > 0) result += `${minutes}分`;
        result += `${seconds}秒`;

        // 获取当前时间和预计结束时间
        const now = new Date();
        const end = new Date(now.getTime() + total * 1000);
        const pad = n => n < 10 ? '0' + n : n;
        const nowStr = `${pad(now.getHours())}:${pad(now.getMinutes())}:${pad(now.getSeconds())}`;
        const endStr = `${pad(end.getHours())}:${pad(end.getMinutes())}:${pad(end.getSeconds())}`;

        document.getElementById('totalTime').innerHTML =
            `预计拍摄总时间：${result}<br>当前时间：${nowStr}<br>预计结束时间：${endStr}`;
    }

    // ======= 事件绑定 =======
    document.getElementById('interval').oninput = calcTotalTime;
    document.getElementById('count').oninput = calcTotalTime;
    document.getElementById('shutterSelect').onchange = shutterSelectChange;
    document.getElementById('shutterInput').oninput = calcTotalTime;

    // ======= 保存参数 =======
    function saveParams(e) {
        e.preventDefault();
        savedInterval = document.getElementById('interval').value;
        savedCount = document.getElementById('count').value;
        savedShutter = getShutterValue();
        alert('参数已保存，请点击“开始延迟摄影”按钮');
    }

    // ======= 启动延迟摄影 =======
    document.getElementById('startBtn').onclick = function() {
        if (!savedInterval || !savedCount || (buleMode && !savedShutter)) {
            alert('请先填写并保存参数');
            return;
        }
        let url = `/timelapse?interval=${savedInterval}&count=${savedCount}`;
        if (buleMode && savedShutter) url += `&shutter=${savedShutter}&bule=1`;
        fetch(url)
            .then(r => r.text())
            .then(alert)
            .catch(err => alert('请求失败: ' + err));
    }

    // ======= 停止拍摄 =======
    document.getElementById('stopBtn').onclick = function() {
        fetch('/timelapse_stop')
            .then(r => r.text())
            .then(alert)
            .catch(err => alert('请求失败: ' + err));
    }
    </script>
</body>

</html>
)rawliteral";