import asyncio
import os
import csv
from bleak import BleakScanner, BleakClient

# Important address from Bluefruit chip
target_mac_address = "F4:7D:A4:2C:1E:EE" 
SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
TX_CHARACTERISTIC_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
RX_CHARACTERISTIC_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

# Store samples to store in a file later
voltage_samples = []

#scans for the device
async def scan_for_device():
    print("Scanning for nearby BLE devices...")
    devices = await BleakScanner.discover()
    for device in devices:
        if device.address == target_mac_address:
            print("Found target device:", device.name)
            return device
    print("Target device not found.")
    return None

#connects to device
async def connect_to_device(device):
    if device is None:
        return None
    print("Connecting to target device...")
    client = BleakClient(device)
    try:
        await client.connect()
        print("Connected to target device successfully!")
        return client
    except Exception as e:
        print("Failed to connect to target device:", e)
        return None

#read sensor value when updated in callback
def notification_callback(sender, data):
    if(len(data) % 3 == 0):
        for i in range(0, len(data), 3):
            print(f"Received notification: {data[i:i+3]}")
            if(len(voltage_samples) <= 2000):
                voltage_samples.append(data[i:i+3])

async def main():
    current_file_path = os.path.abspath(os.getcwd())
    target_device = await scan_for_device()
    if target_device:
        client = await connect_to_device(target_device)
        #THIS IS THE CODE THAT WILL RUN ONCE CONNECTED!
        if client:
            # # DEBUGGING
            for service_obj in client.services:
                print("-----")
                print("SERVICE: ")
                print(service_obj)

            for char_obj in client.services.characteristics.values():
                    print("-----")
                    print("DESCRIPTION: " + char_obj.description)
                    print("UUID: " + char_obj.uuid)
                    print(char_obj.properties)
            await client.start_notify(RX_CHARACTERISTIC_UUID, notification_callback)
            while True:
                # check if CSV is able to be made every second
                await asyncio.sleep(1)

                if(len(voltage_samples) >= 1000):
                    # Appending samples onto a dynamic array
                    # Once array reaches >= 1000 samples, convert into a file
                    # We will do post-processing on MATLab for debugging and implement DSP on Python later

                    for i in range(len(voltage_samples)):
                        voltage_samples[i] = int(voltage_samples[i].decode('utf-8'), 16) # converting samples into integers

                    print("Writing samples to file..") # debug message
                    with open(current_file_path + r'\voltageTest.csv', 'w', newline='') as file:
                        writer = csv.writer(file)
                        
                        # Write the header
                        writer.writerow(['Voltage (0-1023)'])
                        
                        # Write each key-value pair as a row
                        for v in voltage_samples:
                            writer.writerow([v])
                    await client.disconnect()
                    break

        else:
            print("Failed to connect to target device.")
    else:
        print("Exiting.")

if __name__ == "__main__":
    asyncio.run(main())
