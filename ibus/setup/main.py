
import gtk
_ = lambda a : a

TYPE_NOTEBOOK, \
TYPE_FRAME = range(2)

general_page = [
]

notebook = [
	(_("General"), general_page),
]

root_ui = [
	(TYPE_NOTEBOOK, notebook)
]



class SetupDialog(gtk.Dialog):
	def __init__(self):
		super(SetupDialog, self).__init__()

		self.add_button(gtk.STOCK_CLOSE, gtk.RESPONSE_OK)
		for i in root_ui:
			widget = self.__create_ui(i)
			self.vbox.pack_start(widget)
		self.vbox.show_all()

	def __create_ui(self, ui):
		print ui
		type, data = ui
		if type == TYPE_NOTEBOOK:
			widget = self.__create_notebook(data)
		else:
			widget = None
		return widget

	def __create_notebook(self, notebook):
		widget = gtk.Notebook()
		for page_name, data in notebook:
			vbox = gtk.VBox()
			for i in data:
				widget = self.__create_ui(i)
				vbox.pack_start(widget)
			widget.append_page(vbox, gtk.Label(page_name))
		return widget

def main():
	dlg = SetupDialog()
	dlg.run()

if __name__ == "__main__":
	main()
