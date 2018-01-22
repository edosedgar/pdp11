#!/usr/bin/python

import cairo
import gi
import numpy
gi.require_version('Gtk', '3.0')
gi.require_version('PangoCairo', '1.0')
from gi.repository import Gtk, Gdk, Pango, GObject, PangoCairo
import socket, sys
import os

class MachineState():

    def __init__(self, stateview, server):
        self.stateview = stateview
        self.server = server
        self.r0 = 0
        self.r1 = 0
        self.r2 = 0
        self.r3 = 0
        self.r4 = 0
        self.r5 = 0
        self.r6 = 0
        self.r7 = 0
        self.c = 0
        self.v = 0
        self.z = 0
        self.n = 0
        self.t = 0
        self.i = 0
        self.cycle = 0 #us
        self.time = 0

    def show_state(self):
        self.fetch_state()
        statebuffer2 = self.stateview.get_buffer()
        string = "\n R0:" + "{0:#0{1}x}".format(self.r0, 6) + "    "
        string += "R1:" + "{0:#0{1}x}".format(self.r1, 6) + "   "
        string += "R2:" + "{0:#0{1}x}".format(self.r2, 6) + "\n"
        string += " R3:" + "{0:#0{1}x}".format(self.r3, 6) + "    "
        string += "R4:" + "{0:#0{1}x}".format(self.r4, 6) + "   "
        string += "R5:" + "{0:#0{1}x}".format(self.r5, 6) + "\n"
        string += "    R6(SP):" + "{0:#0{1}x}".format(self.r6, 6) + "  "
        string += "R7(PC):" + "{0:#0{1}x}".format(self.r7, 6) + "   \n"
        string += "     C:" + str(self.c) + " V:" + str(self.v)
        string += " Z:" + str(self.z)
        string += " N:" + str(self.n) + " T:" + str(self.t) + " I:"
        string += "{0:#0{1}b}".format(self.i, 5) + "\n\n"
        string += "      Time  = " + "{0:#0{1}d}".format(self.time, 13)
        string += " us\n"
        string += "      Cycle = " + "{0:#0{1}d}".format(self.cycle, 13)
        string += " cyc"

        statebuffer2.set_text(string)

    def fetch_state(self):
        regs = self.server.em_recv_state();
        regs_list = regs.split();
        self.r0 = int(regs_list[0][3:])
        self.r1 = int(regs_list[1][3:])
        self.r2 = int(regs_list[2][3:])
        self.r3 = int(regs_list[3][3:])
        self.r4 = int(regs_list[4][3:])
        self.r5 = int(regs_list[5][3:])
        self.r6 = int(regs_list[6][3:])
        self.r7 = int(regs_list[7][3:])
        psw = int(regs_list[8][4:])
        self.c = psw & 0x01
        self.v = (psw & 0x02) >> 1
        self.z = (psw & 0x04) >> 2
        self.n = (psw & 0x08) >> 3
        self.t = (psw & 0x10) >> 4
        self.i = (psw & 0xE0) >> 5

    def add_emul_time(self, new_cycle):
        self.cycle += new_cycle
        self.time = self.cycle * 1

class SourceCode():

    def __init__(self, textview, server):
        self.code = dict(((i, ["HALT", 2, 0]) for i in range(0x0000, 0xFFFF)))
        self.textview = textview
        self.server = server

    def fetch_code(self, start, end):
        count = start
        while (count <= end + 1):
            comm, s, b = self.code[count]
            command, size = self.server.em_recv_command(count)
            self.code[count] = command, size, b
            count += size

    def show_rom(self, start):
        line = 0
        self.frame_start = start
        count = start
        end = start + 0x80 #Maximum screen line
        string = ""
        self.fetch_code(start, end) #Read ROM
        while (count <= end and line != 25):
            line += 1
            self.frame_end = count
            try:
                command, size, b = self.code[count]
            except:
                command = "Out of range"
                size = 2
            if (b == 1):
                string += u"\u25CF" + " "
            else:
                string += "  "
            string += "{0:#0{1}x}".format(count, 6)
            string += "        "
            string += command.replace("\n", "") + "\n"
            count += size
        textbuffer = self.textview.get_buffer()
        textbuffer.set_text(string)

    def show_cur_line(self, address):
        if (address < self.frame_start or address > self.frame_end):
            self.show_rom(address)

        count = self.frame_start
        string = ""
        while (count <= self.frame_end):
            try:
                command, size, b = self.code[count]
            except:
                command = "Out of range"
                size = 2
            if (b == 1):
                string += u"\u25CF" + " "
            else:
                string += "  "
            string += "{0:#0{1}x}".format(count, 6)
            if (count == address):
                string += " >>>>> "
            else:
                string += "       "
            string += command.replace("\n", "") + "\n"
            count += size
        textbuffer = self.textview.get_buffer()
        textbuffer.set_text(string)

    def toggle_break(self, address):
        try:
            command, size, b = self.code[address]
        except:
            return
        if (b == 0):
            b = 1
        else:
            b = 0
        self.code[address] = command, size, b

class Server():
    def __init__(self):
        self.TCP_IP = '127.0.0.1'
        self.TCP_PORT = 6700

        self.channel = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.channel.connect((self.TCP_IP, self.TCP_PORT))
    def em_recv_reply(self):
        reply = self.channel.recv(256)
        if (reply[0] == '0'):
            return "ok"
        else:
            return "err"

    def em_send_init_gui(self):
        self.channel.send("em_init_gui\0")
        return self.em_recv_reply()

    def em_send_load_file(self, file_path):
        f = open(file_path,'r')
        size = os.path.getsize(file_path);
        self.channel.send("em_load_file " + str(size))
        reply = self.em_recv_reply()
        self.channel.send(f.read(size))
        return self.em_recv_reply()

    def em_recv_command(self, address):
        self.channel.send("em_get_command " + str(address))
        command = self.channel.recv(256)
        size = int(command[0])
        command = command[2:]
        return command, size

    def em_recv_state(self):
        self.channel.send("em_get_state")
        regs = self.channel.recv(256)
        return regs

    def em_send_step(self):
        self.channel.send("em_make_step")
        data = self.channel.recv(256)
        l_data = data.split();
        return int(l_data[0]), int(l_data[1])

    def em_send_reset(self):
        self.channel.send("em_reset_state")
        reply = self.em_recv_reply()
        return

    def em_recv_disp(self):
        self.channel.send("em_get_vram")
        disp = self.channel.recv(8192)
        return disp

    def close(self):
        self.channel.close()

class Emulator(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="PDP11 Emulator")

        self.set_default_size(1100, 768)
        self.set_resizable(False)

        self.logo = 1
        self.disp_lock = 0
        self.grid = Gtk.Grid()
        self.add(self.grid)

        self.create_toolbar()
        self.create_sourceviewer()
        self.create_display()
        self.create_stateviewer()
        self.breakpoint_list = [];

        self.server = Server()
        ret = self.server.em_send_init_gui()
        if (ret == "err"):
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR,
                                       Gtk.ButtonsType.OK,
                                       "Client refused connection")
            dialog.run()
            dialog.destroy()
            sys.exit(1)
        self.current_addr = 0x0000
        self.pixdata = [chr(0) for i in range(0, 8192)]

    def create_toolbar(self):
        toolbar = Gtk.Toolbar()
        self.grid.attach(toolbar, 0, 0, 3, 1)

        button_load = Gtk.ToolButton()
        button_load.set_icon_name("document-open")
        button_load.set_label("Load binary");
        button_load.set_is_important(True)
        button_load.connect("clicked", self.load_button_clicked)
        toolbar.insert(button_load, 0)

        toolbar.insert(Gtk.SeparatorToolItem(), 1)

        button_start = Gtk.ToolButton()
        button_start.set_icon_name("media-playback-start")
        button_start.set_label("Run");
        button_start.set_is_important(True)
        toolbar.insert(button_start, 2)

        button_pause = Gtk.ToolButton()
        button_pause.set_icon_name("media-playback-pause")
        button_pause.set_label("Pause");
        button_pause.set_is_important(True)
        toolbar.insert(button_pause, 3)

        button_reset = Gtk.ToolButton()
        button_reset.set_icon_name("system-shutdown")
        button_reset.set_label("Reset");
        button_reset.set_is_important(True)
        button_reset.connect("clicked", self.button_reset_clicked)
        toolbar.insert(button_reset, 4)

        button_next = Gtk.ToolButton()
        button_next.set_icon_name("go-next")
        button_next.set_label("Step");
        button_next.set_is_important(True)
        toolbar.insert(button_next, 5)
        button_next.connect("clicked", self.button_next_clicked)

        button_toggle_b = Gtk.ToolButton()
        button_toggle_b.set_icon_name("media-record")
        button_toggle_b.set_label("Toggle breakpoint");
        button_toggle_b.set_is_important(True)
        button_toggle_b.connect("clicked", self.toggle_b_clicked)
        toolbar.insert(button_toggle_b, 6)

        toolbar.insert(Gtk.SeparatorToolItem(), 7)
        self.toolbar = toolbar

    def create_sourceviewer(self):
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.set_policy(Gtk.PolicyType.NEVER,
                                       Gtk.PolicyType.NEVER)
        self.scrolledwindow.set_shadow_type(Gtk.ShadowType.IN)
        self.grid.attach_next_to(self.scrolledwindow, self.toolbar,
                                 Gtk.PositionType.BOTTOM, 1, 2)

        self.textview = Gtk.TextView()
        self.textbuffer = self.textview.get_buffer()
        self.textview.set_editable(False)
        self.textview.modify_font(Pango.FontDescription('Monospace 19'))
        self.textview.set_cursor_visible(False)

        self.scrolledwindow.add(self.textview)

    def create_stateviewer(self):
        viewer = Gtk.TextView()
        self.grid.attach(viewer, 1, 1, 2, 1)

        self.stateview = Gtk.TextView()
        self.stateview.set_editable(True)
        self.stateview.modify_font(Pango.FontDescription('Monospace 18'))# OK
        self.stateview.set_cursor_visible(False)

        self.grid.attach_next_to(self.stateview, viewer, Gtk.PositionType.BOTTOM, 2, 1)

    def create_display(self):
        self.display = Gtk.DrawingArea();
        self.display.set_size_request(512, 512);
        self.grid.attach(self.display, 1, 1, 2, 1);
        self.display.connect("draw", self.display_render);

        self.invert = 0
        GObject.timeout_add(16, self.on_timer);

    def on_timer(self):
        self.display.queue_draw()

        return True

    def display_render(self, widget, cr):
        if (self.disp_lock):
            return
        cr.set_source_rgb(0.0, 0.0, 0.0)
        cr.rectangle(0,0,512,512);
        cr.fill()

        if (self.logo):
            layout = PangoCairo.create_layout (cr)
            layout.set_text("No signal", -1)
            desc = Pango.font_description_from_string ("Sans Bold 40")
            layout.set_font_description(desc)
            cr.set_source_rgb((self.invert % 255) / 256.0,(self.invert % 255)/256.0 ,(self.invert % 255.0)/256)
            cr.move_to(130, 230)
            PangoCairo.show_layout (cr, layout)

            self.invert = self.invert + 1
            return

        cr.scale(2, 2)
        cr.set_source_rgb(1.0, 1.0, 1.0)

        for i in range(0, 8192):
            if (ord(self.pixdata[i]) & 0x01):
                cr.rectangle((i % 32) * 8 + 0, i / 32, 1, 1);
            if (ord(self.pixdata[i]) & 0x02):
                cr.rectangle((i % 32) * 8 + 1, i / 32, 1, 1);
            if (ord(self.pixdata[i]) & 0x04):
                cr.rectangle((i % 32) * 8 + 2, i / 32, 1, 1);
            if (ord(self.pixdata[i]) & 0x08):
                cr.rectangle((i % 32) * 8 + 3, i / 32, 1, 1);
            if (ord(self.pixdata[i]) & 0x10):
                cr.rectangle((i % 32) * 8 + 4, i / 32, 1, 1);
            if (ord(self.pixdata[i]) & 0x20):
                cr.rectangle((i % 32) * 8 + 5, i / 32, 1, 1);
            if (ord(self.pixdata[i]) & 0x40):
                cr.rectangle((i % 32) * 8 + 6, i / 32, 1, 1);
            if (ord(self.pixdata[i]) & 0x80):
                cr.rectangle((i % 32) * 8 + 7, i / 32, 1, 1);

        cr.fill()

    def load_button_clicked(self, widget):
        fc = Gtk.FileChooserDialog("Open.." , self, Gtk.FileChooserAction.OPEN,
                                   (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                    Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

        response = fc.run()
        if response == Gtk.ResponseType.OK:
            self.binary_full_name = fc.get_filename()
        else:
            fc.destroy()
            return
        fc.destroy()

        self.server.em_send_load_file(self.binary_full_name)
        self.disasm = SourceCode(self.textview, self.server)
        # ROM initial address
        self.current_addr = 0x8000
        self.halted = 0;
        self.set_title("PDP11 Emulator")
        self.disasm.show_rom(self.current_addr)
        self.disasm.show_cur_line(self.current_addr)

        self.mstate = MachineState(self.stateview, self.server)
        self.mstate.show_state()
        self.logo = 0

    def button_next_clicked(self, widget):
        if (self.halted == 1):
            return
        self.current_addr, cycle = self.server.em_send_step()
        if (cycle == -1):
            self.set_title("PDP11 Emulator (HALTED)")
            self.halted = 1
            return
        comm, size, b = self.disasm.code[self.current_addr]
        self.mstate.add_emul_time(cycle)
        self.disasm.show_cur_line(self.current_addr)
        self.mstate.show_state()
        self.disp_lock = 1
        self.pixdata = self.server.em_recv_disp()
        self.disp_lock = 0

    def toggle_b_clicked(self, widget):
        try:
            start, end = self.textbuffer.get_selection_bounds()
        except:
            return

        # Check that address was selected
        if (start.get_line_offset() != 2 or end.get_line_offset() != 8):
            return

        address = int(start.get_text(end), 16)

        self.disasm.toggle_break(address)
        self.disasm.show_cur_line(self.current_addr)

    def button_reset_clicked(self, widget):
        self.server.em_send_reset()
        self.disasm = SourceCode(self.textview, self.server)
        # ROM initial address
        self.current_addr = 0x8000
        self.halted = 0;
        self.set_title("PDP11 Emulator")
        self.disasm.show_rom(self.current_addr)
        self.disasm.show_cur_line(self.current_addr)

        self.mstate = MachineState(self.stateview, self.server)
        self.mstate.show_state()

# Main interaction
pdp_emul = Emulator()
pdp_emul.connect("delete-event", Gtk.main_quit)
pdp_emul.show_all()
Gtk.main()
