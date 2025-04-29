import asyncio
import websockets

UI_CLIENTS = set()
ARDUINO = None

async def handler(ws, path=None):
    global ARDUINO

    # First message must be an identity packet
    identity = await ws.recv()
    if identity == "ARDUINO":
        ARDUINO = ws
        print("Arduino connected")
    else:
        UI_CLIENTS.add(ws)
        print("UI client connected")

    try:
        async for msg in ws:
            # Forward Arduino → UI
            if ws == ARDUINO:
                for ui in UI_CLIENTS:
                    await ui.send(msg)
            # Forward UI → Arduino
            else:
                if ARDUINO:
                    await ARDUINO.send(msg)
    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        if ws == ARDUINO:
            ARDUINO = None
            print("Arduino disconnected")
        else:
            UI_CLIENTS.remove(ws)
            print("UI client disconnected")

async def main():
    async with websockets.serve(handler, "0.0.0.0", 81):
        print("WebSocket broker listening on port 81")
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    asyncio.run(main())
