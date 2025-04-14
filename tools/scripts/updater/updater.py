from update import Update
import argparse

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('port_path', help='path to device serial port', type=str)
    parser.add_argument('firmware_path', help='path to signed binary for update', type=str)
    parser.add_argument('device_id', help='ID of the device to update', type=str)  # TODO this should be read from the firmware file
    args = parser.parse_args()

    if args.device_id.startswith('0x'):
        device_id = int(args.device_id, 16)
    else:
        device_id = int(args.device_id)

    updater = Update()
    updater.run(args.port_path, args.firmware_path, device_id.to_bytes(1, 'little'))


if __name__ == "__main__":
    main()
