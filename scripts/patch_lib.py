from pathlib import Path
Import("env")

print("[patch] Looking for DFRobot_EnvironmentalSensor")

for lib in env.GetLibBuilders():
    # Find the environmental sensor library
    lib_path = Path(lib.path)
    if lib_path.name != "DFRobot_EnvironmentalSensor":
        continue

    header = lib_path / "src" / "DFRobot_EnvironmentalSensor.h"
    if not header.exists():
        print(f"[patch] Missing header: {header}")
        continue

    # Read the header file and remove the String.h include
    text = header.read_text(
        encoding="utf-8",
        errors="ignore"
    )

    old = '#include "String.h"'
    if old not in text:
        print("[patch] Already patched")
        continue
    text = text.replace(old, "")

    header.write_text(
        text,
        encoding="utf-8"
    )
    print(f"[patch] Removed String.h include from {header}")
    break
