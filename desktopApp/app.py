import tkinter as tk
from tkinter.filedialog import asksaveasfile
from tkinter.messagebox import askyesno
import dbx
import create_csv
import show_mpl

class App:
    def __init__(self):
        self.window = tk.Tk()
        self.window.minsize(width=200, height=100)
        self.window.wm_title("Labino_v2")

        self.frame = tk.Frame(self.window)
        self.frame.pack(padx=10, pady=8)

        self.string_synch = tk.StringVar(value='DBX Synch')
        self.button_synch = tk.Button(self.frame, textvariable=self.string_synch, command=self.synch)
        self.button_synch.pack(pady=2)

        self.button_csv = tk.Button(self.frame, text='Create CSV', command=self.create_csv)
        self.button_csv.pack(pady=2)

        self.button_excel = tk.Button(self.frame, text='Create Excel', state=tk.DISABLED)
        self.button_excel.pack(pady=2)

        self.string_plot = tk.StringVar(value='Show plot')
        self.button_plot = tk.Button(self.frame, textvariable=self.string_plot, command=self.show_plot)
        self.button_plot.pack(pady=2)

        self.string_delete_dbx = tk.StringVar(value='Delete DBX logs')
        self.button_delete_dbx = tk.Button(self.frame, textvariable=self.string_delete_dbx, fg='red', command=self.delete_dbx)
        self.button_delete_dbx.pack(pady=2)

    def synch(self):
        self.button_synch['state'] = tk.DISABLED
        self.string_synch.set('Synchyng...')
        self.window.update()
        dbx.download_folder()
        self.button_synch['state'] = tk.NORMAL
        self.string_synch.set('Synched')

    def create_csv(self):
        self.button_csv['state'] = tk.DISABLED
        self.window.update()
        logs_folder = dbx.logs_folder
        files = [('csv File', '*.csv'), ('All Files', '*.*')]
        save_file = asksaveasfile(filetypes=files, defaultextension=files)
        if save_file is not None:
            dest_file = save_file.name
            create_csv.create_csv(logs_folder, dest_file)
            print(f'created \'{dest_file}\'')
        self.button_csv['state'] = tk.NORMAL

    def delete_dbx(self):
        self.button_delete_dbx['state'] = tk.DISABLED
        self.string_delete_dbx.set('Deleting...')
        self.window.update()
        answer = askyesno(title='Dropbox logs deletion', message='Are you sure that you want to delete all logs stored in Dropbox?\nTis action cannot be undone.')
        if answer:
            dbx.delete_folder()
        self.button_delete_dbx['state'] = tk.NORMAL
        self.string_delete_dbx.set('Delete DBX logs')

    def show_plot(self):
        self.button_plot['state'] = tk.DISABLED
        self.string_plot.set('Showing plot...')
        self.window.update()
        folder = dbx.logs_folder[1:] if dbx.logs_folder[0] == '/' else dbx.logs_folder
        show_mpl.show_plot(folder)
        self.string_plot.set('Show plot')
        self.button_plot['state'] = tk.NORMAL

    def loop(self):
        self.window.mainloop()

app = App()
app.loop()