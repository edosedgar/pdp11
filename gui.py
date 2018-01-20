#!/usr/bin/python

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('PangoCairo', '1.0')
from gi.repository import Gtk, Gdk, Pango, GObject, cairo, PangoCairo

"""class SearchDialog(Gtk.Dialog):

    def __init__(self, parent):
        Gtk.Dialog.__init__(self, "Search", parent,
            Gtk.DialogFlags.MODAL, buttons=(
            Gtk.STOCK_FIND, Gtk.ResponseType.OK,
            Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL))

        box = self.get_content_area()

        label = Gtk.Label("Insert text you want to search for:")
        box.add(label)

        self.entry = Gtk.Entry()
        box.add(self.entry)

        self.show_all()"""

class Emulator(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="PDP11 Emulator")

        self.set_default_size(1024, 768)

        self.grid = Gtk.Grid()
        self.add(self.grid)

        self.create_toolbar()
        self.create_sourceviewer()
        self.create_display()
        self.breakpoint_list = [];

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
                                 Gtk.PositionType.BOTTOM, 1, 1)

        self.textview = Gtk.TextView()
        self.textbuffer = self.textview.get_buffer()
        self.textbuffer.set_text("0x0000 MOV R1, R2 \n"
                                 "0x0002 HALT");
        self.textview.set_editable(False)
        self.textview.modify_font(Pango.FontDescription('Monospace 20'))# OK

        self.scrolledwindow.add(self.textview)

        self.tag_bold = self.textbuffer.create_tag("bold",
            weight=Pango.Weight.BOLD)
        self.tag_italic = self.textbuffer.create_tag("italic",
            style=Pango.Style.ITALIC)
        self.tag_underline = self.textbuffer.create_tag("underline",
            underline=Pango.Underline.SINGLE)
        self.tag_breakpoint = self.textbuffer.create_tag("breakpoint",
            background="red")

    def create_display(self):
        self.display = Gtk.DrawingArea();
        self.display.set_size_request(512, 512);
        self.grid.attach(self.display, 1, 1, 2, 1);

        self.display.connect('draw', self.display_render)
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

        #cr.select_font_face("Courier", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)

    def load_button_clicked(self, widget):
        fc = Gtk.FileChooserDialog("Open.." , self, Gtk.FileChooserAction.OPEN,
                                   (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                    Gtk.STOCK_OPEN, Gtk.ResponseType.OK))

        response = fc.run()
        if response == Gtk.ResponseType.OK:
            self.binary_full_name = fc.get_filename()
        fc.destroy()

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

    """def create_buttons(self):
        check_editable = Gtk.CheckButton("Editable")
        check_editable.set_active(True)
        check_editable.connect("toggled", self.on_editable_toggled)
        self.grid.attach(check_editable, 0, 2, 1, 1)

        check_cursor = Gtk.CheckButton("Cursor Visible")
        check_cursor.set_active(True)
        check_editable.connect("toggled", self.on_cursor_toggled)
        self.grid.attach_next_to(check_cursor, check_editable,
            Gtk.PositionType.RIGHT, 1, 1)

        radio_wrapnone = Gtk.RadioButton.new_with_label_from_widget(None,
            "No Wrapping")
        self.grid.attach(radio_wrapnone, 0, 3, 1, 1)

        radio_wrapchar = Gtk.RadioButton.new_with_label_from_widget(
            radio_wrapnone, "Character Wrapping")
        self.grid.attach_next_to(radio_wrapchar, radio_wrapnone,
            Gtk.PositionType.RIGHT, 1, 1)

        radio_wrapword = Gtk.RadioButton.new_with_label_from_widget(
            radio_wrapnone, "Word Wrapping")
        self.grid.attach_next_to(radio_wrapword, radio_wrapchar,
            Gtk.PositionType.RIGHT, 1, 1)

        radio_wrapnone.connect("toggled", self.on_wrap_toggled,
            Gtk.WrapMode.NONE)
        radio_wrapchar.connect("toggled", self.on_wrap_toggled,
            Gtk.WrapMode.CHAR)
        radio_wrapword.connect("toggled", self.on_wrap_toggled,
            Gtk.WrapMode.WORD)"""
"""
    def on_button_clicked(self, widget, tag):
        bounds = self.textbuffer.get_selection_bounds()
        if len(bounds) != 0:
            start, end = bounds
            self.textbuffer.apply_tag(tag, start, end)

    def on_clear_clicked(self, widget):
        start = self.textbuffer.get_start_iter()
        end = self.textbuffer.get_end_iter()
        self.textbuffer.remove_all_tags(start, end)

    def on_editable_toggled(self, widget):
        self.textview.set_editable(widget.get_active())

    def on_cursor_toggled(self, widget):
        self.textview.set_cursor_visible(widget.get_active())

    def on_wrap_toggled(self, widget, mode):
        self.textview.set_wrap_mode(mode)

    def on_justify_toggled(self, widget, justification):
        self.textview.set_justification(justification)

    def on_search_clicked(self, widget):
        dialog = SearchDialog(self)
        response = dialog.run()
        if response == Gtk.ResponseType.OK:
            cursor_mark = self.textbuffer.get_insert()
            start = self.textbuffer.get_iter_at_mark(cursor_mark)
            if start.get_offset() == self.textbuffer.get_char_count():
                start = self.textbuffer.get_start_iter()

            self.search_and_mark(dialog.entry.get_text(), start)

        dialog.destroy()

    def search_and_mark(self, text, start):
        end = self.textbuffer.get_end_iter()
        match = start.forward_search(text, 0, end)

        if match != None:
            match_start, match_end = match
            self.textbuffer.apply_tag(self.tag_found, match_start, match_end)
            self.search_and_mark(text, match_end)
"""
# Main interaction
pdp_emul = Emulator()
pdp_emul.connect("delete-event", Gtk.main_quit)
pdp_emul.show_all()
Gtk.main()
