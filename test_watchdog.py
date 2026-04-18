import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import struct
import threading
import time


class WatchdogGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Gowin Watchdog Control Panel - FPGA Contest 2026")
        self.root.geometry("680x640")
        self.ser = None
        self.running = False
        self.lock = threading.Lock()   # Bảo vệ truy cập serial từ nhiều thread

        style = ttk.Style()
        style.configure("Status.TLabel", font=('Arial', 10))

        # --- Kết nối UART ---
        conn_frame = ttk.LabelFrame(root, text=" Cấu hình kết nối UART ")
        conn_frame.pack(fill="x", padx=10, pady=5)

        ttk.Label(conn_frame, text="Cổng COM:").grid(row=0, column=0, padx=5, pady=5)
        self.port_cb = ttk.Combobox(conn_frame, values=self.get_ports(), width=15)
        self.port_cb.grid(row=0, column=1, padx=5, pady=5)
        if self.port_cb['values']:
            self.port_cb.current(0)

        ttk.Button(conn_frame, text="Làm mới", width=8,
                   command=self.refresh_ports).grid(row=0, column=2, padx=2)

        ttk.Label(conn_frame, text="Baudrate:").grid(row=0, column=3, padx=5, pady=5)
        self.baud_cb = ttk.Combobox(conn_frame,
                                    values=[9600, 19200, 38400, 57600, 115200],
                                    width=10)
        self.baud_cb.grid(row=0, column=4, padx=5, pady=5)
        self.baud_cb.set(9600)          # FPGA uart_tx/rx đều dùng BAUD_RATE=9600

        self.btn_connect = tk.Button(conn_frame, text="KẾT NỐI",
                                     bg="#4CAF50", fg="white",
                                     font=('Arial', 9, 'bold'),
                                     command=self.toggle_connect)
        self.btn_connect.grid(row=0, column=5, padx=10, pady=5)

        # --- Cài đặt tham số ---
        param_frame = ttk.LabelFrame(root, text=" Cấu hình tham số Watchdog (Registers) ")
        param_frame.pack(fill="x", padx=10, pady=5)

        ttk.Label(param_frame, text="tWD (ms):").grid(row=0, column=0, padx=5, pady=10)
        self.ent_twd = ttk.Entry(param_frame, width=10)
        self.ent_twd.insert(0, "1600")
        self.ent_twd.grid(row=0, column=1, padx=5)
        ttk.Button(param_frame, text="Ghi", width=6,
                   command=lambda: self.write_reg(0x04, int(self.ent_twd.get()))
                   ).grid(row=0, column=2, padx=5)

        ttk.Label(param_frame, text="tRST (ms):").grid(row=0, column=3, padx=5, pady=10)
        self.ent_trst = ttk.Entry(param_frame, width=10)
        self.ent_trst.insert(0, "200")
        self.ent_trst.grid(row=0, column=4, padx=5)
        ttk.Button(param_frame, text="Ghi", width=6,
                   command=lambda: self.write_reg(0x08, int(self.ent_trst.get()))
                   ).grid(row=0, column=5, padx=5)

        ttk.Label(param_frame, text="Arm Delay (us):").grid(row=1, column=0, padx=5, pady=10)
        self.ent_arm = ttk.Entry(param_frame, width=10)
        self.ent_arm.insert(0, "150")
        self.ent_arm.grid(row=1, column=1, padx=5)
        ttk.Button(param_frame, text="Ghi", width=6,
                   command=lambda: self.write_reg(0x0C, int(self.ent_arm.get()), is_16bit=True)
                   ).grid(row=1, column=2, padx=5)

        # --- Trạng thái hệ thống ---
        status_frame = ttk.LabelFrame(root, text=" Trạng thái hệ thống thời gian thực ")
        status_frame.pack(fill="both", expand=True, padx=10, pady=5)

        self.lbl_en    = self._make_light(status_frame, "EN_EFFECTIVE (Đang chạy):", 0, 0)
        self.lbl_fault = self._make_light(status_frame, "FAULT_ACTIVE (Cảnh báo lỗi):", 1, 0)
        self.lbl_enout = self._make_light(status_frame, "ENOUT (LED D4 - Đã arm):", 0, 2)
        self.lbl_wdo   = self._make_light(status_frame, "WDO (LED D3 - Trạng thái):", 1, 2)

        self.lbl_src = ttk.Label(status_frame,
                                 text="Nguồn Kick cuối: ---",
                                 font=('Arial', 10, 'italic'))
        self.lbl_src.grid(row=2, column=0, columnspan=4, pady=15)

        # --- Nút chức năng ---
        btn_frame = ttk.Frame(root)
        btn_frame.pack(fill="x", padx=10, pady=5)

        tk.Button(btn_frame, text="KICK (Software)", bg="#2196F3", fg="white",
                  height=2, font=('Arial', 10, 'bold'),
                  command=self.send_kick
                  ).pack(side="left", padx=5, expand=True, fill="x")

        tk.Button(btn_frame, text="BẬT WATCHDOG", bg="#FF9800", fg="white",
                  height=2, font=('Arial', 10, 'bold'),
                  command=lambda: self.write_reg(0x00, 0x01)
                  ).pack(side="left", padx=5, expand=True, fill="x")

        tk.Button(btn_frame, text="TẮT WATCHDOG", bg="#607D8B", fg="white",
                  height=2, font=('Arial', 10, 'bold'),
                  command=lambda: self.write_reg(0x00, 0x00)
                  ).pack(side="left", padx=5, expand=True, fill="x")

        tk.Button(btn_frame, text="XÓA LỖI (CLR)", bg="#9C27B0", fg="white",
                  height=2, font=('Arial', 10, 'bold'),
                  command=lambda: self.write_reg(0x00, 0x04)
                  ).pack(side="left", padx=5, expand=True, fill="x")

        # --- Console Log ---
        log_frame = ttk.LabelFrame(root, text=" Nhật ký UART ")
        log_frame.pack(fill="x", padx=10, pady=5)
        self.log = tk.Text(log_frame, height=7, state='disabled',
                           bg="#1e1e1e", fg="#00ff00", font=('Consolas', 9))
        self.log.pack(fill="x", padx=5, pady=5)

    # =========================================================================
    # Tiện ích giao diện
    # =========================================================================

    def _make_light(self, parent, text, row, col):
        ttk.Label(parent, text=text, style="Status.TLabel"
                  ).grid(row=row, column=col, sticky="w", padx=10, pady=10)
        lbl = tk.Label(parent, text="---", bg="grey", width=14,
                       fg="white", font=('Arial', 9, 'bold'))
        lbl.grid(row=row, column=col + 1, padx=5, pady=10)
        return lbl

    def get_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

    def refresh_ports(self):
        self.port_cb['values'] = self.get_ports()
        if self.port_cb['values']:
            self.port_cb.current(0)

    def log_msg(self, msg, is_tx=True):
        """Thread-safe: gọi được từ bất kỳ thread nào."""
        prefix = ">> TX: " if is_tx else "<< RX: "
        line = f"{time.strftime('%H:%M:%S')} {prefix}{msg}\n"
        def _append():
            self.log.config(state='normal')
            self.log.insert('end', line)
            self.log.see('end')
            self.log.config(state='disabled')
        self.root.after(0, _append)

    # =========================================================================
    # Kết nối / Ngắt kết nối
    # =========================================================================

    def toggle_connect(self):
        if self.ser and self.ser.is_open:
            self.running = False
            time.sleep(0.3)
            self.ser.close()
            self.btn_connect.config(text="KẾT NỐI", bg="#4CAF50")
            self.log_msg("Đã ngắt kết nối UART.")
        else:
            try:
                self.ser = serial.Serial(
                    self.port_cb.get(),
                    int(self.baud_cb.get()),
                    timeout=0.3        # Đủ thời gian chờ response tại 9600 baud
                )
                self.ser.reset_input_buffer()   # Xóa byte cũ 1 lần duy nhất khi mở cổng
                self.btn_connect.config(text="NGẮT KẾT NỐI", bg="#f44336")
                self.log_msg(f"Đã mở {self.port_cb.get()} @ {self.baud_cb.get()} bps")
                self.running = True
                threading.Thread(target=self.update_loop, daemon=True).start()
            except Exception as e:
                messagebox.showerror("Lỗi UART", f"Không thể mở cổng:\n{e}")

    # =========================================================================
    # Protocol helpers
    # =========================================================================

    @staticmethod
    def _calc_chk(payload: bytes) -> int:
        res = 0
        for b in payload:
            res ^= b
        return res

    def _send_recv(self, payload: bytes, expect_bytes: int) -> bytes:
        """Gửi 1 frame UART và đọc response. Thread-safe qua lock."""
        frame = bytes([0x55]) + payload + bytes([self._calc_chk(payload)])
        with self.lock:
            # KHÔNG gọi reset_input_buffer() ở đây —
            # nếu gọi trong lock sẽ xóa mất ACK của lệnh trước đang về đến
            self.ser.write(frame)
            resp = self.ser.read(expect_bytes)
        return frame, resp

    def _check_resp(self, resp: bytes, expected_len: int, expected_cmd: int) -> bool:
        """Kiểm tra header, độ dài và checksum của response."""
        if len(resp) != expected_len:
            return False
        if resp[0] != 0xAA or resp[1] != expected_cmd:
            return False
        # Checksum = XOR các byte từ resp[1] đến resp[-2]
        if self._calc_chk(resp[1:-1]) != resp[-1]:
            return False
        return True

    # =========================================================================
    # Các lệnh UART
    # =========================================================================

    def write_reg(self, addr: int, val: int, is_16bit: bool = False):
        """Dispatch sang background thread — không bao giờ block UI thread."""
        threading.Thread(
            target=self._write_reg_bg,
            args=(addr, val, is_16bit),
            daemon=True
        ).start()

    def _write_reg_bg(self, addr: int, val: int, is_16bit: bool):
        """Thực thi WRITE (0x01) trên background thread, log kết quả qua root.after."""
        if not self.ser or not self.ser.is_open:
            return
        try:
            data = struct.pack('<H' if is_16bit else '<I', val)
            payload = bytes([0x01, addr, len(data)]) + data
            frame, resp = self._send_recv(payload, 4)
            self.log_msg(frame.hex(' ').upper())
            if self._check_resp(resp, 4, 0x01):
                self.log_msg(f"{resp.hex(' ').upper()}  [ACK OK]", is_tx=False)
            elif len(resp) >= 2 and resp[1] in (0xFE, 0xFF):
                self.log_msg(f"{resp.hex(' ').upper()}  [NACK]", is_tx=False)
            else:
                self.log_msg(
                    f"Timeout/lỗi: {resp.hex(' ').upper() if resp else 'rỗng'}",
                    is_tx=False)
        except Exception as e:
            self.log_msg(f"Lỗi write: {e}", is_tx=False)

    def send_kick(self):
        """Dispatch sang background thread."""
        threading.Thread(target=self._send_kick_bg, daemon=True).start()

    def _send_kick_bg(self):
        """Thực thi KICK (0x03) trên background thread."""
        if not self.ser or not self.ser.is_open:
            return
        try:
            payload = bytes([0x03, 0x00, 0x00])
            frame, resp = self._send_recv(payload, 4)
            self.log_msg(frame.hex(' ').upper() + "  [KICK]")
            if self._check_resp(resp, 4, 0x03):
                self.log_msg(f"{resp.hex(' ').upper()}  [ACK OK]", is_tx=False)
            else:
                self.log_msg(f"KICK ACK lỗi: {resp.hex(' ').upper()}", is_tx=False)
        except Exception as e:
            self.log_msg(f"Lỗi kick: {e}", is_tx=False)

    # =========================================================================
    # Polling trạng thái (background thread)
    # =========================================================================

    def update_loop(self):
        """Gửi lệnh STATUS (0x04) mỗi 0.5s, cập nhật UI."""
        payload = bytes([0x04, 0x00, 0x00])
        consecutive_errors = 0

        while self.running:
            if self.ser and self.ser.is_open:
                try:
                    _, resp = self._send_recv(payload, 7)
                    # Response format: [AA][04][00][D0][D1][D2][CHK]
                    if self._check_resp(resp, 7, 0x04):
                        status_byte = resp[3]   # reg_status[7:0] chứa tất cả các bit
                        self.root.after(0, self.update_ui_status, status_byte)
                        consecutive_errors = 0
                    elif len(resp) >= 2 and resp[1] in (0xFE, 0xFF):
                        self.root.after(0, self.log_msg,
                                        f"NACK từ FPGA: {resp.hex(' ').upper()}", False)
                    else:
                        consecutive_errors += 1
                        if consecutive_errors >= 5:
                            self.root.after(0, self.log_msg,
                                            f"Mất kết nối ({consecutive_errors} lần timeout liên tiếp)", False)
                            consecutive_errors = 0
                except Exception as e:
                    consecutive_errors += 1
            time.sleep(0.5)

    # =========================================================================
    # Cập nhật UI từ status byte
    # =========================================================================

    def update_ui_status(self, val: int):
        # reg_status = {27'd0, last_kick_src, wdo, enout, fault_active, en_effective}
        # bit0 = en_effective
        # bit1 = fault_active
        # bit2 = enout
        # bit3 = wdo  (0=lỗi, 1=bình thường)
        # bit4 = last_kick_src (0=button, 1=UART)
        en_eff = (val >> 0) & 0x01
        fault  = (val >> 1) & 0x01
        enout  = (val >> 2) & 0x01
        wdo    = (val >> 3) & 0x01
        src    = (val >> 4) & 0x01

        self.lbl_en.config(
            text="ĐANG CHẠY"  if en_eff else "ĐANG DỪNG",
            bg="#2ecc71"       if en_eff else "#7f8c8d")

        self.lbl_fault.config(
            text="PHÁT HIỆN LỖI" if fault else "BÌNH THƯỜNG",
            bg="#e74c3c"          if fault else "#27ae60")

        self.lbl_enout.config(
            text="SÁNG (ARMED)" if enout else "TẮT",
            bg="#f1c40f"         if enout else "#7f8c8d")

        self.lbl_wdo.config(
            text="LỖI (LOW)"  if not wdo else "OK (HIGH)",
            bg="#c0392b"       if not wdo else "#27ae60")

        src_txt = "Nút nhấn S1 (Hardware)" if src == 0 else "Lệnh UART (Software)"
        self.lbl_src.config(text=f"Nguồn Kick cuối: {src_txt}")


if __name__ == "__main__":
    root = tk.Tk()
    app = WatchdogGUI(root)
    root.mainloop()
