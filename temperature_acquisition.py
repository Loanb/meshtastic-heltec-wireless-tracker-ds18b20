import csv
import re
import time
from datetime import datetime

import meshtastic.serial_interface
from pubsub import pub

# Serial port connected to the local Meshtastic node
PORT = "COM9"

# Time between two temperature requests (seconds)
INTERVAL_SECONDS = 300

# Output CSV file
CSV_FILE = "temperatures.csv"

# Regular expression used to extract temperature values from received text messages.
# Expected format:
#   1m : 12.4 °C
#   surface : 11.8 °C
TEMP_RE = re.compile(
    r"^(.+?)\s*:\s*(-?\d+(?:\.\d+)?)\s*°?C$",
    re.MULTILINE,
)


def on_receive(packet, interface):
    """
    Callback executed each time a Meshtastic packet is received.

    If the packet contains temperature data, extract the sensor name and value,
    then append them to the CSV file.
    """
    decoded = packet.get("decoded", {})
    text = decoded.get("text")

    # Ignore packets without text content
    if not text:
        return

    # Get sender ID and reception timestamp
    from_id = packet.get("fromId") or packet.get("from")
    timestamp = datetime.now().isoformat(timespec="seconds")

    # Extract all temperature measurements from the received text
    matches = TEMP_RE.findall(text)

    # Ignore messages that do not contain temperature data
    if not matches:
        return

    # Append all measurements to the CSV file
    with open(CSV_FILE, "a", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)

        for sensor_name, value in matches:
            writer.writerow([timestamp, from_id, sensor_name.strip(), value])

    print(f"[{timestamp}] Data saved from {from_id}: {text}")


def ensure_csv_header():
    """
    Create the CSV file with its header if it does not already exist.
    """
    try:
        with open(CSV_FILE, "x", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["timestamp", "node_id", "sensor", "value_c"])
    except FileExistsError:
        pass


def main():
    """
    Connect to the local Meshtastic node, periodically broadcast a /temp command,
    and log all received temperature responses.
    """
    ensure_csv_header()

    # Subscribe to incoming Meshtastic messages
    pub.subscribe(on_receive, "meshtastic.receive")

    # Open the serial connection to the local node
    interface = meshtastic.serial_interface.SerialInterface(PORT)

    while True:
        print("Sending broadcast /temp request...")

        # Broadcast the temperature request to all nodes
        interface.sendText("/temp")

        # Wait before sending the next request
        time.sleep(INTERVAL_SECONDS)


if __name__ == "__main__":
    main()