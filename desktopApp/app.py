import tkinter as tk

class GUI:
    def __init__(self):
        self.window = tk.Tk()
        self.window.minsize(width=500, height=500)

        tk.Label(self.window, text='Labino_v2').grid(row=0, column=0)

        self.plot_frame = tk.Frame(self.window, relief=tk.GROOVE, bd=2)
        self.plot_frame.grid(row=1, column=0)
        tk.Label(self.plot_frame, text='holo').pack()

        self.controls_frame = tk.Frame(self.window)
        self.controls_frame.grid(row=1, column=1)
        self.creation_frame = tk.LabelFrame(self.controls_frame, text='Create file')
        self.creation_frame.grid(row=0, column=0)
        self.button_csv = tk.Button(self.creation_frame, text='Create CSV')
        self.button_csv.grid(row=0, column=0)
        self.button_excel = tk.Button(self.creation_frame, text='Create Excel')
        self.button_excel.grid(row=1, column=0)


    def mainloop(self):
        self.window.mainloop()

gui = GUI()
gui.mainloop()