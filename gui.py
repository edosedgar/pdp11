#!/usr/bin/python

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('PangoCairo', '1.0')
from gi.repository import Gtk, Gdk, Pango, GObject, cairo, PangoCairo
import socket, sys

class SourceCode():

    def __init__(self, textview, server):
        self.code = dict(((i, ["HALT", 2]) for i in range (0, 0xE000)))
        self.textview = textview
        self.server = server

    def fetch_code(self, start, end):
        count = start
        while (count < end):
            command, size = self.server.em_recv_command(count)
            self.code[count] = command, size
            count += size

    def show_rom(self):
        count = 0
        string = ""
        while (count < 0xE000):
            command, size = self.code[count]
            string += "{0:#0{1}x}".format(count, 6)
            string += "        "
            string += command.replace("\n", "") + "\n"
            count += size
        textbuffer = self.textview.get_buffer()
        textbuffer.set_text(string)

    def show_cur_line(self, address):

class Server():
    def __init__(self):
        self.TCP_IP = '127.0.0.1'
        self.TCP_PORT = 6700
        BUFFER_SIZE = 1024

        self.channel = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.channel.connect((self.TCP_IP, self.TCP_PORT))
    def em_recv_reply(self):
        reply = self.channel.recv(256)
        if (reply[0] == '0'):
            return "ok"
        else:
            return "err"

    def em_send_init_gui(self):
        self.channel.send("em_init_gui")
        return self.em_recv_reply()

    def em_send_load_file(self, file_path):
        self.channel.send("em_load_file")
        reply = self.em_recv_reply()
        if reply == "ok":
            self.channel.send(file_path)
        else:
            return "err"
        return self.em_recv_reply()

    def em_recv_command(self, address):
        self.channel.send("em_get_command")
        self.channel.send(str(address))
        command = self.channel.recv(256)
        size = int(self.channel.recv(256))
        self.channel.send("ok")
        return command, size

    def receive(self):
        data = channel.recv(message)

    def close(self):
        self.channel.close()

class Emulator(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="PDP11 Emulator")

        self.set_default_size(1024, 768)
        self.set_resizable(False)

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
        self.curr_address = 0x0000

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
        self.scrolledwindow.set_shadow_type(Gtk.ShadowType.IN)
        self.grid.attach_next_to(self.scrolledwindow, self.toolbar,
                                 Gtk.PositionType.BOTTOM, 1, 2)

        self.textview = Gtk.TextView()
        self.textbuffer = self.textview.get_buffer()
        self.textview.set_editable(False)
        self.textview.modify_font(Pango.FontDescription('Monospace 19'))
        self.textview.set_cursor_visible(False)

        self.scrolledwindow.add(self.textview)

        self.tag_breakpoint = self.textbuffer.create_tag("breakpoint",
            background="red")

    def create_stateviewer(self):
        viewer = Gtk.TextView()
        self.grid.attach(viewer, 1, 1, 2, 1)

        self.stateview = Gtk.TextView()
        self.statebuffer2 = self.stateview.get_buffer()
        self.statebuffer2.set_text("\n  |> R0 = 0x0000 <|> R1 = 0x0000 <|\n"
                                   "  |> R2 = 0x0000 <|> R3 = 0x0000 <|\n"
                                   "  |> R4 = 0x0000 <|> R5 = 0x0000 <|\n"
                                   "  |> R6 = 0x0000 <|> R7 = 0x0000 <|\n"
                                   "  |>    C = 0 | V = 0 | Z = 0    <|\n"
                                   "  |>    N = 0 | T = 0 | I = 0    <|\n"
                                   "  |>  Time  = 0000000000000 mcs  <|\n"
                                   "  |>  Cycle = 0000000000000 cyc  <|\n")
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
        cr.set_source_rgb(0.0, 0.0, 0.0)
        cr.rectangle(0,0,512,512);
        cr.fill()

        layout = PangoCairo.create_layout (cr)
        layout.set_text("No signal", -1)
        desc = Pango.font_description_from_string ("Sans Bold 40")
        layout.set_font_description(desc)
        cr.set_source_rgb((self.invert % 255) / 256.0,(self.invert % 255)/256.0 ,(self.invert % 255.0)/256)
        cr.move_to(130, 230)
        PangoCairo.show_layout (cr, layout)

        self.invert = self.invert + 1

    def load_button_clicked(self, widget):
        fc = Gtk.FileChooserDialog("Open.." , self, Gtk.FileChooserAction.OPEN,
                                   (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                    Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

        response = fc.run()
        if response == Gtk.ResponseType.OK:
            self.binary_full_name = fc.get_filename()
        fc.destroy()

        self.server.em_send_load_file(self.binary_full_name)
        self.disasm = SourceCode(self.textview, self.server)
        self.disasm.fetch_code(0x0000, 0x0004)
        self.current_addr = 0
        self.disasm.show_rom()
        self.disasm.show_cur_line(self.current_addr)

    def button_next_clicked(self, widget):
        self.current_addr += 2
        self.disasm.show_cur_line(self.current_addr)

    def toggle_b_clicked(self, widget):
        try:
            start, end = self.textbuffer.get_selection_bounds()
        except:
            return

        if (start.get_line_offset() != 0 or end.get_line_offset() != 6):
            return

        for i in self.breakpoint_list:
            if i[0].get_line() == start.get_line():
                self.textbuffer.remove_tag(self.tag_breakpoint, i[0], i[1])
                self.breakpoint_list.remove([i[0], i[1]]);
                return

        self.breakpoint_list.append([start, end]);
        self.textbuffer.apply_tag(self.tag_breakpoint, start, end)

# Main interaction
pdp_emul = Emulator()
pdp_emul.connect("delete-event", Gtk.main_quit)
pdp_emul.show_all()
Gtk.main()
