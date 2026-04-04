import asyncio
from bleak import BleakScanner, BleakClient

DEVICE_NAME = "NU40-LIGHT"
NUS_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

def handle_rx(_, data: bytearray):
    try:
        print(data.decode().strip())
    except Exception:
        print(data)

async def main():
    print("BLE 장치 검색 중...")
    device = await BleakScanner.find_device_by_name(DEVICE_NAME, timeout=15.0)

    if device is None:
        print("NU40-LIGHT를 찾지 못했습니다.")
        return

    async with BleakClient(device) as client:
        print("연결됨. 센서값 수신 중...")
        await client.start_notify(NUS_TX_UUID, handle_rx)

        while True:
            await asyncio.sleep(1)

asyncio.run(main())