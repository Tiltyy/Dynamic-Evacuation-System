// Leaflet 地图实例
let map;
// 存储当前路径的图层
let currentPathLayer = null;
// 存储当前节点标记的图层组
let currentNodeMarkers = L.layerGroup();

// WebSocket 连接
const ws = new WebSocket("ws://" + window.location.hostname + ":8000/websocket");

// DOM 元素引用
const systemStatusElement = document.getElementById('system-status');
const tvocElement = document.getElementById('tvoc-value');
const eco2Element = document.getElementById('eco2-value');
const mq2VoltageElement = document.getElementById('mq2-voltage-value');
const mq2ConcentrationElement = document.getElementById('mq2-concentration-value');
const accelXElement = document.getElementById('accel-x-value');
const accelYElement = document.getElementById('accel-y-value');
const accelZElement = document.getElementById('accel-z-value');
const gyroXElement = document.getElementById('gyro-x-value');
const gyroYElement = document.getElementById('gyro-y-value');
const gyroZElement = document.getElementById('gyro-z-value');
const pitchElement = document.getElementById('pitch-value');
const rollElement = document.getElementById('roll-value');
const yawElement = document.getElementById('yaw-value');
const alertMessageElement = document.getElementById('alert-message');

// 初始化地图
function initMap() {
    // 默认中心点和缩放级别 (北京故宫，您可以根据您的实际地图范围调整)
    map = L.map('map').setView([39.9042, 116.4074], 16);

    // 添加 OpenStreetMap 瓦片图层
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
    } ).addTo(map);

    // 将节点标记图层组添加到地图
    currentNodeMarkers.addTo(map);
}

// 根据气体浓度获取路径颜色和粗细
function getPathStyle(gasConcentration) {
    // 您可以根据实际的气体浓度范围定义更精细的颜色和粗细
    if (gasConcentration > 200) { // 示例阈值
        return { className: 'leaflet-path-gas-high' }; // 红色，粗
    } else if (gasConcentration > 100) {
        return { className: 'leaflet-path-gas-medium' }; // 橙色，中
    } else {
        return { className: 'leaflet-path-gas-low' }; // 绿色，细
    }
}

// 根据气体浓度获取节点标记样式
function getNodeMarkerStyle(gasConcentration) {
    let className = 'gas-node-marker';
    if (gasConcentration > 200) {
        className += ' gas-node-marker-high';
    } else if (gasConcentration > 100) {
        className += ' gas-node-marker-medium';
    } else {
        className += ' gas-node-marker-low';
    }
    return L.divIcon({ className: className, iconSize: [10, 10] });
}

// 更新地图上的路径和气体浓度可视化
function updateMapPath(pathNodes) {
    // 移除旧的路径和标记
    if (currentPathLayer) {
        map.removeLayer(currentPathLayer);
    }
    currentNodeMarkers.clearLayers();

    if (!pathNodes || pathNodes.length < 2) {
        console.warn("No valid path data to display.");
        return;
    }

    const latlngs = [];
    const pathSegments = []; // 用于存储带气体浓度的路径段

    for (let i = 0; i < pathNodes.length; i++) {
        const node = pathNodes[i];
        const latlng = [node.lat, node.lng];
        latlngs.push(latlng);

        // 添加节点标记
        const marker = L.marker(latlng, {
            icon: getNodeMarkerStyle(node.gas_conc)
        }).bindPopup(`节点ID: ${node.node_id}<br>区域ID: ${node.area_id}<br>气体浓度: ${node.gas_conc.toFixed(2)} ppm`);
        currentNodeMarkers.addLayer(marker);

        // 准备路径段数据
        if (i > 0) {
            const prevNode = pathNodes[i - 1];
            // 假设路径段的气体浓度取两个节点中的最大值
            const segmentGasConc = Math.max(prevNode.gas_conc, node.gas_conc);
            pathSegments.push({
                coords: [
                    [prevNode.lat, prevNode.lng],
                    [node.lat, node.lng]
                ],
                gasConc: segmentGasConc
            });
        }
    }

    // 绘制分段路径，根据气体浓度应用不同样式
    pathSegments.forEach(segment => {
        L.polyline(segment.coords, getPathStyle(segment.gasConc)).addTo(map);
    });

    // 调整地图视图以适应路径
    map.fitBounds(latlngs);
}

// WebSocket 事件处理
ws.onopen = function(event) {
    console.log("WebSocket connected!");
};

ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log("Received data:", data);

    // 更新系统状态
    if (data.system_status) {
        systemStatusElement.textContent = data.system_status;
        systemStatusElement.className = 'system-status'; // 重置类名
        if (data.system_status === 'Normal') {
            systemStatusElement.classList.add('status-normal');
            alertMessageElement.style.display = 'none'; // 隐藏警报
        } else if (data.system_status === 'Warning') {
            systemStatusElement.classList.add('status-warning');
            alertMessageElement.style.display = 'block'; // 显示警报
            alertMessageElement.textContent = '警告：环境气体异常！';
        } else if (data.system_status === 'Evacuation') {
            systemStatusElement.classList.add('status-evacuation');
            alertMessageElement.style.display = 'block'; // 显示警报
            alertMessageElement.textContent = '紧急疏散！请跟随指引！';
        }
    }

    // 更新环境数据
    if (data.environmental) {
        tvocElement.textContent = data.environmental.tvoc !== undefined ? data.environmental.tvoc.toFixed(0) + ' ppb' : 'N/A';
        eco2Element.textContent = data.environmental.eco2 !== undefined ? data.environmental.eco2.toFixed(0) + ' ppm' : 'N/A';
        mq2VoltageElement.textContent = data.environmental.mq2_voltage !== undefined ? data.environmental.mq2_voltage.toFixed(3) + ' V' : 'N/A';
        mq2ConcentrationElement.textContent = data.environmental.mq2_concentration !== undefined ? data.environmental.mq2_concentration.toFixed(2) + ' ppm' : 'N/A';
    }

    // 更新运动数据
    if (data.motion) {
        accelXElement.textContent = data.motion.accel_x !== undefined ? data.motion.accel_x.toFixed(2) + ' g' : 'N/A';
        accelYElement.textContent = data.motion.accel_y !== undefined ? data.motion.accel_y.toFixed(2) + ' g' : 'N/A';
        accelZElement.textContent = data.motion.accel_z !== undefined ? data.motion.accel_z.toFixed(2) + ' g' : 'N/A';
        gyroXElement.textContent = data.motion.gyro_x !== undefined ? data.motion.gyro_x.toFixed(2) + ' dps' : 'N/A';
        gyroYElement.textContent = data.motion.gyro_y !== undefined ? data.motion.gyro_y.toFixed(2) + ' dps' : 'N/A';
        gyroZElement.textContent = data.motion.gyro_z !== undefined ? data.motion.gyro_z.toFixed(2) + ' dps' : 'N/A';
        pitchElement.textContent = data.motion.pitch !== undefined ? data.motion.pitch.toFixed(2) + ' °' : 'N/A';
        rollElement.textContent = data.motion.roll !== undefined ? data.motion.roll.toFixed(2) + ' °' : 'N/A';
        yawElement.textContent = data.motion.yaw !== undefined ? data.motion.yaw.toFixed(2) + ' °' : 'N/A';
    }

    // 更新地图路径
    if (data.path_nodes) {
        updateMapPath(data.path_nodes);
    }
};

ws.onerror = function(error) {
    console.error("WebSocket Error:", error);
};

ws.onclose = function(event) {
    console.log("WebSocket closed:", event);
    // 可以在这里尝试重连
};

// 页面加载完成后初始化地图
document.addEventListener('DOMContentLoaded', initMap);

// 示例：控制按钮点击事件 (发送消息到后端，后端需要有相应的处理逻辑)
document.getElementById('btn-normal-mode').addEventListener('click', () => {
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ command: 'set_mode', mode: 'normal' }));
        console.log("Sent command: set_mode normal");
    }
});

document.getElementById('btn-evac-mode').addEventListener('click', () => {
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ command: 'set_mode', mode: 'evacuation' }));
        console.log("Sent command: set_mode evacuation");
    }
});

