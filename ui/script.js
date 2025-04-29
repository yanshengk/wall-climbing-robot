// const ws = new WebSocket("ws://192.168.50.137:81/");
const ws = new WebSocket("ws://localhost:81/");
// const ws_binary = new WebSocket("ws://192.168.50.137:81/");

ws.binaryType = "arraybuffer";

// Video feed configuration
const videoFeed = document.getElementById("videoFeed");
const videoUrl = "http://your-ip-camera-url/stream"; // Replace with actual IP camera URL

// LED control
const ledFront = document.getElementById("ledFront");
let frontLed = false;

// Power control
const powerControl = document.getElementById("powerControl");
const powerValue = document.getElementById("powerValue");

// Status elements
const connectionStatus = document.getElementById("connectionStatus");
const frontLedStatus = document.getElementById("frontLedStatus");
const motionStatus = document.getElementById("motionStatus");
const EdfStatus = document.getElementById("EdfStatus");

const robotStatus = {
    connection: "NOT CONNECTED",
    frontLed: "-",
    motion: "-",
    edf: "-"
};

ws.onopen = function () {
    ws.send("UI");
    console.log("Connected to the server");
    // Enable buttons once connection is established
    ledFront.disabled = false;
};

ws.onmessage = function (event) {
    let msg;
    try {
        msg = JSON.parse(event.data);
    } catch (err) {
        console.warn("Malformed JSON:", event.data);
        return;
    }

    // console.log(msg instanceof Object);

    switch (msg.type) {
        case "connection":
            // { "type": "connection", "content": "connected" }
            robotStatus.connection = msg.content.toUpperCase();
            break;

        case "led":
            robotStatus.frontLed = msg.content.toUpperCase();
            break;

        case "motion":
            robotStatus.motion = msg.content.toUpperCase();
            break;

        case "edf":
            robotStatus.edf = msg.content.toUpperCase();
            break;

        // console.log("Message from Arduino: " + event.data);
    };
}

ws.onerror = function (error) {
    console.error("WebSocket error: ", error);
};

ws.onclose = function () {
    console.warn("WebSocket connection closed");
}

// Initialize video feed
async function initializeVideo() {
    try {
        // Replace this with actual video stream implementation
        // videoFeed.src = videoUrl;
        console.log("Video feed initialized");
    } catch (error) {
        console.error("Error initializing video feed:", error);
    }
}

// LED control handler
ledFront.addEventListener("click", () => {
    frontLed = !frontLed;
    ledFront.classList.toggle("active");
    if (frontLed) {
        ledFront.innerHTML = "Front LED ON";
    } else {
        ledFront.innerHTML = "Front LED OFF";
    }
    sendMessage("led", `${frontLed}`);
});

// Power control hanfler
powerControl.addEventListener("input", () => {
    const v = powerControl.value;
    powerValue.textContent = `${v}%`;

    const value = parseInt(powerControl.value, 10);

    sendValue(value);
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
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(`{"type":"${type}", "content":"${content}"}`);
        console.log("Sent command: " + type + ", " + content);
    } else {
        console.warn("WebSocket not connected. Command not sent: " + content);
    }
}

function sendValue(value) {
    const buf = new Uint8Array([value]);
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(buf.buffer);
        console.log("Sent value: " + value);
    } else {
        console.warn("WebSocket not connected. Value not sent: " + value);
    }
}

// Status update function
function updateRobotStatus() {
    connectionStatus.textContent = robotStatus.connection;
    frontLedStatus.textContent = robotStatus.frontLed;
    motionStatus.textContent = robotStatus.motion;
    EdfStatus.textContent = robotStatus.edf;
}

// Initialize everything
initializeVideo();
setInterval(updateRobotStatus, 50); // Update status every 0.05 second

// document.onload();
