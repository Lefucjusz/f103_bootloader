from updater import Updater

PORT_PATH = '/dev/ttyUSB0'
FW_FILE_PATH = '../../../build/signed.bin'
DEVICE_ID = b'\x69'


def main() -> None:
    updater = Updater()
    updater.run(PORT_PATH, FW_FILE_PATH, DEVICE_ID) # TODO take these from command line


if __name__ == "__main__":
    main()
