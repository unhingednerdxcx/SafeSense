from Shield import collect_data
import re
from random import randint
while True:
    arr = collect_data()
    temp = arr[0]  # Temperature
    currentDb = arr[1]  # Volume
    dist = arr[2]  # Distance
    gas = arr[3]  # PPM
    lux = randint(0, 40000)  # Lux value, set to 0 as placeholder
    # Sensor values

    # Read lines from HTML
    with open("PUT FULL LOCATION OF INDEX.HTML HERE", "r") as file:
        lines = file.readlines()

    # Edit lines
    for i, line in enumerate(lines):
        if "let temp" in line:
            lines[i] = re.sub(r"let temp\s*=\s*.*?;", f"let temp = {temp};", line)
        if "let currentDb" in line:
            lines[i] = re.sub(r"let currentDb\s*=\s*.*?;", f"let currentDb = {currentDb};", line)
        if "let gas" in line:
            lines[i] = re.sub(r"let gas\s*=\s*.*?;", f"let gas = {gas};", line)
        if "let dist" in line:
            lines[i] = re.sub(r"let dist\s*=\s*.*?;", f"let dist = {dist};", line)
        if "let lux" in line:
            lines[i] = re.sub(r"let lux\s*=\s*.*?;", f"let lux = {lux};", line)

    # Save changes
    with open("PUT FULL LOCATION OF INDEX.HTML HERE", "w") as file:
        file.writelines(lines)
