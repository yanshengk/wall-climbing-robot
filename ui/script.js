// WebSocket connection
const webSocket = new WebSocket("ws://192.168.1.102:88/");
webSocket.binaryType = "arraybuffer";

// Camera feed handle
const cameraIpAddress = "192.168.1.100";
const cameraFeed = document.getElementById("cameraFeed");
const cameraUrl = `http://${cameraIpAddress}:81/stream`;
// const cameraUrl = `http://192.168.1.102:8080/stream`;
const captureBtn = document.getElementById("captureBtn");
const captureUrl = `http://${cameraIpAddress}/capture`;

// Front LED control handle
const frontLedBtn = document.getElementById("frontLedBtn");
let frontLedLogic = false;

// EDF Power control handle
const edfPowerControl = document.getElementById("edfPowerControl");
const edfPowerValue = document.getElementById("edfPowerValue");

// Robot Status handle
const connectionStatus = document.getElementById("connectionStatus");
const frontLedStatus = document.getElementById("frontLedStatus");
const edfStatus = document.getElementById("edfStatus");
const motionStatus = document.getElementById("motionStatus");

const robotStatus = {
    connection: "DISCONNECTED",
    frontLed: "-",
    edfPower: "-",
    motion: "-"
};
let retrieveStatus;

webSocket.onopen = function () {
    webSocket.send("UI");
    console.log("[WebSocket] Connected to the server");
    sendMessage("retrieve", "status");
    retrieveStatus = true;
};

webSocket.onmessage = function (event) {
    let msg;
    try {
        msg = JSON.parse(event.data);
    } catch (error) {
        console.error("[WebSocket] Malformed JSON:", event.data);
        return;
    }

    // { "type": "connection", "content": "connected" }
    switch (msg.type) {
        case "connection":
            robotStatus.connection = msg.content.toUpperCase();
            break;

        case "frontLed":
            robotStatus.frontLed = msg.content.toUpperCase();
            break;

        case "edfPower":
            robotStatus.edfPower = msg.content.toUpperCase();
            break;

        case "motion":
            robotStatus.motion = msg.content.toUpperCase();
            break;
    };
    console.log(`[WebSocket] Received message: ${msg.type}, ${msg.content}`);

    updateRobotStatus();
}

webSocket.onerror = function (error) {
    console.error("[WebSocket] WebSocket error: ", error);
};

webSocket.onclose = function () {
    console.warn("[WebSocket] WebSocket connection closed");
}

async function initialiseCamera() {
    try {
        cameraFeed.src = cameraUrl;
        console.log("Camera feed initialized");
    } catch (error) {
        console.error("Error initialising camera feed:", error);
    }
}

async function captureAndSave() {
    const res = await fetch(captureUrl);
    const blob = await res.blob();
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `capture-${Date.now()}.jpg`;
    a.click();
    URL.revokeObjectURL(url);
}

captureBtn.addEventListener("click", captureAndSave);

// Front LED control handler
frontLedBtn.addEventListener("click", () => {
    frontLedLogic = !frontLedLogic;
    sendMessage("frontLed", `${frontLedLogic}`);
});

// EDF Power control handler
edfPowerControl.addEventListener("input", () => {
    const value = parseInt(edfPowerControl.value, 10);
    sendPowerValue(value);
});

// Keyboard control handlers
document.addEventListener("keydown", (event) => {
    if (event.repeat) return; // Prevent repeat events while key is held

    switch (event.key) {
        case "ArrowUp":
            sendMessage("motion", "forward");
            break;

        case "ArrowDown":
            sendMessage("motion", "backward");
            break;

        case "ArrowLeft":
            sendMessage("motion", "left");
            break;

        case "ArrowRight":
            sendMessage("motion", "right");
            break;
    }
});

document.addEventListener("keyup", (event) => {
    switch (event.key) {
        case "ArrowUp":
        case "ArrowDown":
        case "ArrowLeft":
        case "ArrowRight":
            sendMessage("motion", "stop");
            break;
    }
});

function sendMessage(type, content) {
    if (webSocket.readyState === WebSocket.OPEN) {
        webSocket.send(`{"type":"${type}", "content":"${content}"}`);
        console.log("[WebSocket] Sent message: " + type + ", " + content);
    } else {
        console.warn("[WebSocket] WebSocket not connected. Message not sent: " + content);
    }
}

function sendPowerValue(value) {
    const buf = new Uint8Array([value]);
    if (webSocket.readyState === WebSocket.OPEN) {
        webSocket.send(buf.buffer);
        console.log("[WebSocket] Sent power value: " + value);
    } else {
        console.warn("[WebSocket] WebSocket not connected. Power value not sent: " + value);
    }
}

// Status update function
function updateRobotStatus() {
    connectionStatus.textContent = robotStatus.connection;
    if (robotStatus.connection == "CONNECTED") {
        frontLedBtn.disabled = false;
        edfPowerControl.disabled = false;
    } else {
        frontLedBtn.disabled = true;
        edfPowerControl.disabled = true;
    }

    frontLedStatus.textContent = robotStatus.frontLed;
    if (robotStatus.frontLed == "ON") {
        frontLedLogic = true;
        frontLedBtn.classList.add("active");
    } else {
        frontLedLogic = false;
        frontLedBtn.classList.remove("active");
    }

    if (parseInt(robotStatus.edfPower) > 5) {
        edfStatus.textContent = "RUNNING";
    }
    else if (parseInt(robotStatus.edfPower) <= 5) {
        edfStatus.textContent = "STOP";
    }
    else {
        edfStatus.textContent = robotStatus.edfPower;
    }

    motionStatus.textContent = robotStatus.motion;

    edfPowerValue.textContent = robotStatus.edfPower + " %";
    if (retrieveStatus && robotStatus.edfPower != "-") {
        edfPowerControl.value = robotStatus.edfPower;
        retrieveStatus = false;
    }
}

initialiseCamera();
updateRobotStatus();
