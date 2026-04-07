import sys
import csv
import serial.tools.list_ports
import serial
import time
import os
import shutil
import tempfile
import struct
from pathlib import Path
import numpy as np
import esptool
from PyQt6.QtGui import QIcon, QAction, QPixmap
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import (
    QWidget, QApplication, QVBoxLayout, QPushButton, QFileDialog, QLabel, QTabWidget, QLineEdit, QSystemTrayIcon, QMenu, QMenuBar
)
from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg, NavigationToolbar2QT
from matplotlib.figure import Figure


def resource_path(*parts):
    # PyInstaller extracts bundled data under _MEIPASS at runtime.
    if hasattr(sys, "_MEIPASS"):
        base_path = Path(sys._MEIPASS)
    else:
        base_path = Path(__file__).resolve().parents[2]
    return base_path.joinpath(*parts)


def app_bundle_path(*parts):
    # In a PyInstaller onedir build, keep editable files next to the exe.
    if getattr(sys, "frozen", False):
        base_path = Path(sys.executable).resolve().parent
    else:
        base_path = Path(__file__).resolve().parents[2]
    return base_path.joinpath(*parts)


def editable_config_path(filename):
    external_path = app_bundle_path("config", filename)
    if external_path.exists():
        return external_path

    # In onefile builds, packaged defaults are extracted under _MEIPASS.
    bundled_path = resource_path("config", filename)
    if bundled_path.exists():
        return bundled_path

    raise FileNotFoundError(
        f"Missing required config file: {external_path} or {bundled_path}. "
        "Rebuild the app or restore the packaged config files."
    )


def ensure_external_config_files():
    config_dir = app_bundle_path("config")
    config_dir.mkdir(parents=True, exist_ok=True)

    for filename in ("commands.txt", "params.txt"):
        external_path = config_dir / filename
        if external_path.exists():
            continue

        bundled_path = resource_path("config", filename)
        if bundled_path.exists():
            shutil.copy2(bundled_path, external_path)

class GraphingTab(QWidget):
    def __init__(self, tray):
        super().__init__()

        self.setWindowTitle("Dynamic CSV Plot Viewer")

        layout = QVBoxLayout()
        self.setLayout(layout)

        # Figure
        self.fig = Figure(figsize=(6, 8))
        self.canvas = FigureCanvasQTAgg(self.fig)
        layout.addWidget(self.canvas)

        self.toolbar = NavigationToolbar2QT(self.canvas, self)
        layout.addWidget(self.toolbar)

        # Buttons
        self.load_button = QPushButton("Load CSV")
        self.load_button.clicked.connect(self.load_csv)
        layout.addWidget(self.load_button)

        self.prev_button = QPushButton("Previous Group")
        self.prev_button.clicked.connect(self.prev_group)
        layout.addWidget(self.prev_button)

        self.next_button = QPushButton("Next Group")
        self.next_button.clicked.connect(self.next_group)
        layout.addWidget(self.next_button)

        # Data storage
        self.data = None
        self.headers = []
        self.groups = []
        self.current_group = 0

        self.tray = tray

    def load_csv(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Open CSV File",
            "",
            "CSV Files (*.csv)"
        )

        if not file_path:
            return

        data = []

        with open(file_path, newline='', encoding='utf-8') as csvfile:
            reader = csv.reader(csvfile)

            # Extract headers
            self.headers = next(reader)

            for row in reader:
                numeric_row = [float(val) for val in row if val.strip() != ""]
                data.append(numeric_row)

        self.data = np.array(data)

        if self.data.shape[1] < 2:
            return

        self.create_groups()
        self.current_group = 0
        self.update_plots()
		
    def create_groups(self):
        self.groups = []

        # Skip first column (X-axis)
        num_cols = self.data.shape[1]

        for i in range(1, num_cols, 3):
            group = list(range(i, min(i + 3, num_cols)))
            self.groups.append(group)
			
    def next_group(self):
        if not self.groups:
            return

        self.current_group = (self.current_group + 1) % len(self.groups)
        self.update_plots()

    def prev_group(self):
        if not self.groups:
            return

        self.current_group = (self.current_group - 1) % len(self.groups)
        self.update_plots()

    def update_plots(self):
        if self.data is None:
            return

        self.fig.clear()

        time = self.data[:, 0]
        group = self.groups[self.current_group]

        num_plots = len(group)

        axes = []
        for i in range(num_plots):
            ax = self.fig.add_subplot(num_plots, 1, i + 1)
            axes.append(ax)

        for i, col_index in enumerate(group):
            ax = axes[i]
            y = self.data[:, col_index]

            ax.plot(time, y)
            ax.set_title(f"{self.headers[col_index]} vs {self.headers[0]}")
            ax.set_xlabel(self.headers[0])
            ax.set_ylabel(self.headers[col_index])

        self.fig.tight_layout()
        self.canvas.draw()


# Where the user will flash the drone and change parameters
class ProgramTab(QWidget):
    def __init__(self, tray):
        super().__init__()
        self.tray = tray
        layout = QVBoxLayout(self)

        self.p_box = QLineEdit()
        self.p_box.setFixedWidth(40)
        layout.addWidget(QLabel("P"))
        layout.addWidget(self.p_box, alignment=Qt.AlignmentFlag.AlignVCenter)
        
        self.i_box = QLineEdit()
        self.i_box.setFixedWidth(40)
        layout.addWidget(QLabel("I"))
        layout.addWidget(self.i_box, alignment=Qt.AlignmentFlag.AlignVCenter)

        self.d_box = QLineEdit()
        self.d_box.setFixedWidth(40)
        layout.addWidget(QLabel("D"))
        layout.addWidget(self.d_box, alignment=Qt.AlignmentFlag.AlignVCenter)


        self.flash_button = QPushButton("Flash Drone")
        self.flash_button.clicked.connect(self.flash_drone)
        layout.addWidget(self.flash_button)

        self.force_full_flash_button = QPushButton("Force Full Flash")
        self.force_full_flash_button.clicked.connect(lambda: self.flash_drone(full_flash=True))
        self.force_full_flash_button.setVisible(False)
        layout.addWidget(self.force_full_flash_button)

        self.port_name = None
        self.baud_rate_flash = "460800"
        self.baud_rate_config = 115200
        ## see partitions.csv and build/flash_args:
        self.bootloader_offset = "0x1000"
        self.bootloader_path = resource_path("firmware", "bootloader.bin")

        self.factory_offset = "0x10000"
        self.factory_path = resource_path("firmware", "drone-firmware.bin")

        self.partition_table_offset = "0x8000"
        self.partition_table_size = "0x1000"
        self.partition_table_path = resource_path("firmware", "partition-table.bin")

    # def flight_path(self):
    
    # def wall_follow(self):

    # def hover_mode(self):

    # def flight_options(self):

    def detect_port(self):
        for port in serial.tools.list_ports.comports():
            if "CH340" in port.description:
                #self.tray.showMessage("WingFlasher", f"Found device on port {port.device}")
                return port.device
        return None

    def flash_drone(self, full_flash = False):
        self.port_name = self.detect_port()
        if not self.port_name:
            self.tray.showMessage("WingFlasher", "No CH340 device detected")
            return

        if not full_flash:
            dump_path = None
            expected = Path(self.partition_table_path).read_bytes()

            with tempfile.NamedTemporaryFile(delete=False) as tmp:
                dump_path = tmp.name

            #check if drone is already flashed
            read_cmd = [
            "--chip", "esp32",
            "-b", self.baud_rate_flash,
            "--port", self.port_name,
            "--before", "default_reset",
            "--after", "hard_reset",
            "read_flash", self.partition_table_offset, self.partition_table_size, dump_path
            ]

            try:
                esptool.main(read_cmd)
                current = Path(dump_path).read_bytes()
                if current[:len(expected)] == expected: #if the partition table is already onboard, don't re-flash
                    full_flash = False
                else:
                    full_flash = True
            except Exception as exc:
                self.tray.showMessage("WingFlasher", f"Flash check failed. Performing full flash")
                full_flash = True
            finally:
                if dump_path and os.path.exists(dump_path):
                    os.remove(dump_path)

        if full_flash:
            write_cmd = [ #flash partition table last for verification purposes
                "--chip", "esp32",
                "-b", self.baud_rate_flash,
                "--port", self.port_name,
                "--before", "default_reset",
                "--after", "hard_reset",
                "write_flash", 
                "--flash_mode", "dio",
                "--flash_freq", "40m",
                "--flash_size", "8MB",
                self.bootloader_offset, str(self.bootloader_path),
                self.factory_offset, str(self.factory_path),
                self.partition_table_offset, str(self.partition_table_path),
            ]

            try:
                self.tray.showMessage("WingFlasher", f"Flashing started on {self.port_name}")
                esptool.main(write_cmd)
            except Exception as exc:
                self.tray.showMessage("WingFlasher", f"Flash failed: {exc}")
                raise
        
        #time.sleep(20)
        try:
            self.tray.showMessage("WingFlasher", f"Serial config transfer started on {self.port_name}")
            self.serial_send()
            self.tray.showMessage("WingFlasher", "Flash complete")
        except Exception as exc:
            self.tray.showMessage("WingFlasher", f"Serial config transfer failed: {exc}")
            raise
        

            # time.sleep(1)

            # with serial.Serial(self.port_name, 460800, timeout=1) as ser:
            #     ser.write(b'OK')        # send 1 byte
            #     #ser.write(b'\x42')     # send raw hex if you prefer
            #     ser.flush()

            #     print("Sent test bytes.")

    def serial_send(self):
        script_file_path = editable_config_path("commands.txt")
        config_file_path = editable_config_path("params.txt")
        chunk_size = 32

        def wait_for_ack(ser):
            while True:
                line = ser.read_until(b'\n')
                if not line:
                    raise RuntimeError("Timeout waiting for response")

                stripped = line.strip()

                if not stripped:
                    continue  # ignore blank lines

                if stripped == b'ACK':
                    break

                raise RuntimeError(f"Unexpected response: {line}")
            
            return "ACK"

        def wait_for_ready(ser):
            while True:
                line = ser.read_until(b"\n")
                if not line:
                    raise RuntimeError("Timeout waiting for READY")

                print(f"ESP: {line!r}")
                if b"READY" in line:
                    return "READY"

        def send_file(filename):
            with open(filename, "rb") as f:
                file_data = f.read()

            file_size = len(file_data)
            print(f"Sending file size: {file_size} bytes")

            ser.write(struct.pack("<I", file_size))
            ser.flush()

            print("Sending file in 32-byte chunks with ACK...")

            for i in range(0, file_size, chunk_size):
                chunk = file_data[i:i+chunk_size]
                ser.write(chunk)
                ser.flush()

                result = wait_for_ack(ser)

                if result == "ACK":
                    print(f"Chunk {i // chunk_size + 1} acknowledged")

            print("File transfer complete.")

        with serial.Serial(self.port_name, self.baud_rate_config, timeout=5) as ser:

            print("Opening serial port...")
            time.sleep(2)

            print("Sending SCRIPT...")
            ser.write(b"SCRIPT\n")
            ser.flush()

            print("Waiting for READY...")
            wait_for_ready(ser)

            print("Sending START...")
            ser.write(b"START\n")
            ser.flush()

            send_file(script_file_path)
            #TODO sleep??
            print("Sending PARAM...")
            ser.write(b"PARAM\n")
            ser.flush()

            print("Waiting for READY...")
            wait_for_ready(ser)

            print("Sending START...")
            ser.write(b"START\n")
            ser.flush()
            
            send_file(config_file_path)

            # print("Waiting for file dump from ESP (1024 bytes)...")

            # received = b""
            # expected = 1024

            # while len(received) < expected:
            #     chunk = ser.read(expected - len(received))
            #     if not chunk:
            #         raise RuntimeError("Timeout while receiving file dump")
            #     received += chunk

# Live data output?
# class LiveTab(QWidget):

class PlanningTab(QWidget):
    def __init__(self, tray):
        super().__init__()
        self.tray = tray
        layout = QVBoxLayout(self)

        self.fly_button = QPushButton("Save Flight Path")
        # self.fly_button.clicked.connect(self.flight_path)
        layout.addWidget(self.fly_button)

        self.wall_button = QPushButton("Wall Follow Mode")
        # self.wall_button.clicked.connect(self.wall_follow)
        layout.addWidget(self.wall_button)

        self.hover_button = QPushButton("Hover Mode")
        # self.hover_button.clicked.connect(self.hover_mode)
        layout.addWidget(self.hover_button)

        self.foptions_button = QPushButton("Flight Options")
        # self.foptions_button.clicked.connect(self.flight_options)
        layout.addWidget(self.foptions_button)


class PlotWindow(QWidget):
    def __init__(self, tray):
        super().__init__()
        self.setWindowTitle("Feedback Controls - WingFlasher Drone Software Suite")
        self.advanced_mode_enabled = False

        layout = QVBoxLayout(self)
        menu_bar = QMenuBar(self)
        file_menu = menu_bar.addMenu("File")
        layout.setMenuBar(menu_bar)

        self.advanced_mode_action = QAction("Advanced Mode", self)
        self.advanced_mode_action.setCheckable(True)
        self.advanced_mode_action.setChecked(False)
        self.advanced_mode_action.toggled.connect(self.toggle_advanced_mode)
        file_menu.addAction(self.advanced_mode_action)

        quit_action = QAction("Quit", self)
        quit_action.triggered.connect(QApplication.instance().quit)
        file_menu.addAction(quit_action)

        self.tabs = QTabWidget()
        layout.addWidget(self.tabs)

        self.logo_label = QLabel(self)
        self.logo_label.setStyleSheet("background: transparent;")
        self.logo_label.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        logo_path = resource_path("assets", "wingfeather_final_small_for_white_bg.png")
        if logo_path.exists():
            self.logo_pixmap = QPixmap(str(logo_path))
            self.update_logo_pixmap()
            self.logo_label.show()
        else:
            self.logo_pixmap = QPixmap()
            self.logo_label.hide()

        self.tray = tray
        self.graphing_tab = GraphingTab(tray)
        self.program_tab = ProgramTab(tray)
        self.planning_tab = PlanningTab(tray)
        # self.live_tab = LiveTab()

        self.tabs.addTab(self.graphing_tab, "Graphing")
        self.tabs.addTab(self.planning_tab, "Planning")
        self.tabs.addTab(self.program_tab, "Programming")
        # self.tabs.addTab(self.live_tab, "Live")

    def toggle_advanced_mode(self, enabled):
        self.advanced_mode_enabled = enabled
        self.program_tab.force_full_flash_button.setVisible(enabled)
        self.tray.showMessage("WingFlasher", f"Advanced mode {'enabled' if enabled else 'disabled'}")

    def update_logo_pixmap(self):
        if self.logo_pixmap.isNull():
            return
        scaled = self.logo_pixmap.scaledToHeight(
            60,
            Qt.TransformationMode.SmoothTransformation,
        )
        self.logo_label.setPixmap(scaled)
        self.logo_label.resize(scaled.size())

    def resizeEvent(self, event):
        super().resizeEvent(event)
        if self.logo_label.isHidden():
            return
        self.update_logo_pixmap()
        x = self.width() - self.logo_label.width() - 12
        y = 0
        self.logo_label.move(x, y)
        self.logo_label.raise_()

def main():
    ensure_external_config_files()
    app = QApplication(sys.argv)
    icon_path = resource_path("assets", "WingFlasher.ico")
    tray = QSystemTrayIcon(QIcon(str(icon_path))) #check if path exists??
    tray.setToolTip("WingFlasher")
    tray.show()
    window = PlotWindow(tray)
    if icon_path.exists():
        app.setWindowIcon(QIcon(str(icon_path)))
        window.setWindowIcon(QIcon(str(icon_path)))
    window.show()
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
